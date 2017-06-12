#include "buffered_sender.h"
#include "gril_stream_parcer.h"
#include "assert.h"

#ifndef LINUX
	#include "user_interface.h"
	#include "osapi.h"
#else 
	#define LOCAL static
	#define ICACHE_FLASH_ATTR
	#define os_strcmp strcmp
	#define os_strncmp strncmp
	#define os_strlen strlen
	#define os_sprintf sprintf
	#define os_memcpy memcpy
	#include <stdio.h>
	#include <string.h>
#endif

void buffered_sender_init(
		struct BufferedSender_t * sender,
		struct GrilStreamParcerResult_t * parcer_res,
		char first) {
	assert(sender);
	sender->count = first ? 0 : sender->count + 1;

	if (parcer_res->error != GrilStreamParcerNoError) {
		sender->pos = sender->buffer + os_sprintf(
			sender->buffer,
			"ER%03X%%%s%%",
			(unsigned int)(os_strlen(parcer_res->prefix) + 2),
			parcer_res->prefix);
	} else {
		sender->pos = sender->buffer + os_sprintf(
			sender->buffer,
			"RE%03X%%%s%%",
			(unsigned int)(os_strlen(parcer_res->prefix) + 2),
			parcer_res->prefix);
	}
	sender->begin = sender->pos;
	sender->end = &sender->buffer[sizeof(sender->buffer)] - 2;
	sender->parcer_res = parcer_res;
}

void buffered_sender_flush(
		struct BufferedSender_t * sender) {
	assert(sender);
	int len = (sender->pos - sender->buffer) - 5;
	char len_buffer[8];
	os_memcpy(
		&sender->buffer[2],
		len_buffer,
		os_sprintf(len_buffer, "%03X", len));

	//send data
	fprintf(stderr, "%s count:%d begin:%p pos:%p\n",
		__FUNCTION__,
		sender->count,
		sender->begin,
		sender->pos);

	if (sender->count == 0 || sender->begin != sender->pos) {
		*(sender->pos++) = '\n';
		sender->parcer_res->sender->fun_send(
			sender->parcer_res->sender->sender,
			sender->buffer,
			sender->pos - sender->buffer);
		buffered_sender_init(sender, sender->parcer_res, 0);
	}
}

void buffered_sender_send(
		void * sender_,
		const char * res_str,
		int res_str_len) {
	assert(sender_);
	if (!res_str_len)
		return;

	struct BufferedSender_t * sender = sender_;
	const char * end = &res_str[res_str_len];
	int len;

	while (res_str < end) {
		len = end - res_str;
		//copy to buffer
		if ((sender->end - sender->pos) >= len) {
			fprintf(stderr, "%s 1. \n", __FUNCTION__);
			os_memcpy(sender->pos, res_str, len);
			res_str = end;
			sender->pos += len;
		//flush to file
		} else {
			fprintf(stderr, "%s 2. \n", __FUNCTION__);
			len = sender->end - sender->pos;
			os_memcpy(sender->pos, res_str, len);
			res_str += len;
			sender->pos += len;
			buffered_sender_flush(sender);
		}
	}
}



#ifdef TEST_BUFFERED_SENDER

void fun_send(void * sender, const char * res_str, int str_len) {
	fprintf(stderr, "fun_send str_len:%d res_str:%s", str_len, res_str);
}

int main() {
	struct BufferedSender_t bf_sender;
	struct GrilStreamParcerResultSender_t sender;
	struct GrilStreamParcerResult_t res;

	sender.sender = NULL;
	sender.fun_send = fun_send;

	res.error = GrilStreamParcerNoError;
	res.cmd = "print";
	res.prefix = "";
	res.param = "XXX";
	res.value = "V1";
	res.sender = &sender;

	buffered_sender_init(&bf_sender, &res, 1);
	char test_str[70] = "";
	memset(test_str, 'a', sizeof(test_str));
	test_str[sizeof(test_str) - 1] = 0;
	buffered_sender_send(&bf_sender, test_str, os_strlen(test_str));
	buffered_sender_flush(&bf_sender);
	buffered_sender_flush(&bf_sender);
	buffered_sender_flush(&bf_sender);
	return 0;
}
#endif

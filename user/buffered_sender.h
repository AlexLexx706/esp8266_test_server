#ifndef BUFFERED_SENDER_H
#define BUFFERED_SENDER_H

//Buffered sender
typedef struct BufferedSender_t {
	char buffer[64];
	char * begin;
	char * pos;
	char * end;
	int  count;
	struct GrilStreamParcerResult_t * parcer_res;
} BufferedSender;

void buffered_sender_init(
		struct BufferedSender_t * sender,
		struct GrilStreamParcerResult_t * parcer_res,
		char first);

void buffered_sender_flush(
		struct BufferedSender_t * sender);

void buffered_sender_send(
		void * sender,
		const char * res_str,
		int res_str_len);

#endif //BUFFERED_SENDER
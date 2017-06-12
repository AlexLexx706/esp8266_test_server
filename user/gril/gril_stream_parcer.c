#include "gril_stream_parcer.h"
#include <assert.h>

//extern int os_strncmp(const char * str1, const char * str2, int len);
#ifndef LINUX
	#include <osapi.h>
	#include <c_types.h>
#else
	#include <string.h>
	#include <stdio.h>

	#define os_strncmp(str1, str2, len)\
		strncmp(str1, str2, len)

	#define os_strlen(str)\
		strlen(str)

	#ifndef LOCAL
		#define LOCAL static
	#endif

	#define ICACHE_FLASH_ATTR
#endif

enum GrilStreamParcerState {
	BeginParceState = 0,
	ReadPrefixState,
	ReadCmdState,
	ReadParamSplitter,
	ReadParamState,
	ReadDataState,
};


LOCAL void ICACHE_FLASH_ATTR
gril_stream_cmd_parcer_reinit(GrilStreamParcer * parcer) {
	parcer->state = BeginParceState;

	parcer->pos_prefix = parcer->prefix;
	*parcer->pos_prefix = 0;

	parcer->pos_cmd = parcer->cmd;
	*parcer->pos_cmd = 0;

	parcer->pos_param = parcer->param;
	*parcer->pos_param = 0;

	parcer->pos_value = parcer->value;
	*parcer->pos_value = 0;
}


void ICACHE_FLASH_ATTR
gril_stream_parcer_init(
		GrilStreamParcer * parcer,
		GrilStreamParcerHandler handler,
		GrilStreamParcerResultSender * sender, 
		GrilCommandNameDesc * commands_list,
		unsigned char commands_list_len) {
	assert(parcer);
	assert(handler);
	assert(sender);
	assert(commands_list);
	assert(commands_list_len);
	gril_stream_cmd_parcer_reinit(parcer);	
	parcer->handler = handler;
	parcer->sender = sender;
	parcer->commands_list = commands_list;
	parcer->commands_list_len = commands_list_len;

}


LOCAL void ICACHE_FLASH_ATTR
complete_parce(
		GrilStreamParcer * parcer,
		enum GrilStreamParcerError error) {
	GrilStreamParcerResult result;
	*parcer->pos_prefix = 0;
	*parcer->pos_cmd = 0;
	*parcer->pos_param = 0;
	*parcer->pos_value = 0;

	result.error = error;
	result.prefix = parcer->prefix;
	result.cmd = parcer->cmd;
	result.param = parcer->param;
	result.value = parcer->value;
	result.sender = parcer->sender;

	parcer->handler(&result);
	gril_stream_cmd_parcer_reinit(parcer);
}

void ICACHE_FLASH_ATTR
gril_stream_parcer_parce(
		GrilStreamParcer * parcer,
		const char * data,
		int data_size) {
	assert(parcer);
	assert(data);
	int i;

	const char * data_end = &data[data_size]; 

	//[%prefix%][name[,parameter][,value][:option]]<eoc>
	while (data < data_end) {
		//end
		if (*data == GRIL_CMD_COMPLETE_CHAR) {
			switch (parcer->state) {
				case ReadParamSplitter:
				case ReadParamState:
				case ReadDataState:
					complete_parce(parcer, GrilStreamParcerNoError);
					break;
				case BeginParceState:
					complete_parce(
						parcer,
						parcer->pos_prefix == parcer->prefix ?
							GrilStreamParcerErrorNotComplete :
							GrilStreamParcerNoError);
					break;
				default:
					complete_parce(parcer, GrilStreamParcerErrorNotComplete);
			}
		}

		switch (parcer->state) {
			case BeginParceState:
				switch (*data) {
					case '%':
						parcer->state = ReadPrefixState;
						break;
					default:
						for (i = 0; i < parcer->commands_list_len; i++) {
							if (parcer->commands_list[i].name[0] == *data) {
								parcer->state = ReadCmdState;
								*(parcer->pos_cmd++) = *data;
								break;
							}
						}
				}
				break;
			case ReadPrefixState:
				switch (*data) {
					case '%':
						parcer->state = BeginParceState;
						break;
					default:
						if (parcer->pos_prefix == &parcer->prefix[sizeof(parcer->prefix)]) {
							//complete_parce(parcer, handler, GrilStreamParcerErrorPrefixTooLong);
							gril_stream_cmd_parcer_reinit(parcer);
						} else
							*(parcer->pos_prefix++) = *data;
						break;
				}
				break;
			case ReadCmdState:
				if (parcer->pos_cmd == &parcer->cmd[sizeof(parcer->cmd)]){
					//complete_parce(parcer, handler, GrilStreamParcerErrorCmdTooLong);
					gril_stream_cmd_parcer_reinit(parcer);
				} else {
					*(parcer->pos_cmd++) = *data;
					int has_cmd = 0;
					for (i = 0; i < parcer->commands_list_len; i++) {
						if (!os_strncmp(parcer->cmd, parcer->commands_list[i].name, parcer->pos_cmd - parcer->cmd)) {
							if (parcer->pos_cmd - parcer->cmd == parcer->commands_list[i].name_len) {
								parcer->state = ReadParamSplitter;
							}
							has_cmd = 1;
							break;
						}
					}
					if (!has_cmd)
						//complete_parce(parcer, handler, GrilStreamParcerErrorUnknownCmd);
						gril_stream_cmd_parcer_reinit(parcer);
				}
				break;
			case ReadParamSplitter:
				if (*data != ','){
					//complete_parce(parcer, handler, GrilStreamParcerErrorWnongParamSplitter);
					gril_stream_cmd_parcer_reinit(parcer);
				} else
					parcer->state = ReadParamState;
				break;
			case ReadParamState:
				if (parcer->pos_param == &parcer->param[sizeof(parcer->param)]) {
					//complete_parce(parcer, handler, GrilStreamParcerErrorParamTooLong);
					gril_stream_cmd_parcer_reinit(parcer);
				} else {
					if (*data == ',')
						parcer->state = ReadDataState;
					else
						*(parcer->pos_param++) = *data;
				}
				break;
			case ReadDataState:
				if (parcer->pos_value == &parcer->value[sizeof(parcer->value)]) {
					//complete_parce(parcer, handler, GrilStreamParcerErrorDataTooLong);
					gril_stream_cmd_parcer_reinit(parcer);
				} else
					*(parcer->pos_value++) = *data;
				break;
		}
		data++;
	}
}


#ifdef GRIL_STREAM_PARCER_TEST
int main() {
	GrilStreamParcer parcer;
	char * test_str[] = {"123123\n", "%12%\n", "%sdsd\n", "%%en\n", "%set\n", "print,\n", "set,xx/sd,23\n"};
	int i;
	static const char * errr_descs[] = {
		"GrilStreamParcerNoError",
		"GrilStreamParcerErrorPrefixNotComplete",
		"GrilStreamParcerErrorPrefixTooLong",
		"GrilStreamParcerErrorNotComplete",
		"GrilStreamParcerErrorCmdTooLong",
		"GrilStreamParcerErrorUnknownCmd",
		"GrilStreamParcerErrorWnongParamSplitter",
		"GrilStreamParcerErrorParamTooLong",
		"GrilStreamParcerErrorDataTooLong"
	};

	void parcer_handler(GrilStreamParcerResult * result) {
		fprintf(stderr, "parce_handler prefix:%s, cmd:%s param:%s, value:%s error:%d errr_descs: %s\n",
			result->prefix,
			result->cmd,
			result->param,
			result->value,
			result->error,
			errr_descs[result->error]);
		result->sender->fun_send(result->sender->sender, "res", 3);
	}

	void send_fun(void * sender, const char * res_str, int str_len) {
		fprintf(stderr, "send_fun res_str:%s str_len:%d\n", res_str, str_len);
	}

	GrilCommandNameDesc cmd_names[] = {{"print", 5}, {"set", 3}};
	GrilStreamParcerResultSender result_sender = {0, send_fun};
	gril_stream_parcer_init(&parcer, parcer_handler, &result_sender, cmd_names, 2);

	for (i=0; i < sizeof(test_str)/sizeof(char *); i++) {
		char tmp[50];
		strcpy(tmp, test_str[i]);
		tmp[strlen(tmp) - 1] = 0;
		fprintf(stderr, "test str:%s\n", tmp);
		gril_stream_parcer_parce(&parcer, test_str[i], strlen(test_str[i]), NULL);	
	}
	return 0;
}
#endif
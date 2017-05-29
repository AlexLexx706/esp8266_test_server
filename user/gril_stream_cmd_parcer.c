#include "gril_stream_cmd_parcer.h"
#include <assert.h>

//extern int os_strncmp(const char * str1, const char * str2, int len);
#ifndef TEST
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

enum GrilStreamCmdParcerState {
	BeginParceState = 0,
	ReadPrefixState,
	ReadCmdState,
	ReadParamSplitter,
	ReadParamState,
	ReadDataState,
};


LOCAL void ICACHE_FLASH_ATTR
gril_stream_cmd_parcer_reinit(GrilStreamCmdParcer * parcer) {
	parcer->state = BeginParceState;
	parcer->pos_prefix = parcer->prefix;
	*parcer->pos_prefix = 0;

	parcer->pos_cmd = parcer->cmd;
	*parcer->cmd = 0;

	parcer->pos_param = parcer->param;
	*parcer->param = 0;

	parcer->pos_value = parcer->value;
	*parcer->value = 0;
}


void ICACHE_FLASH_ATTR
gril_stream_cmd_parcer_init(GrilStreamCmdParcer * parcer, parce_res_handler handler,
		GrilCommandNameDesc * commands_list, unsigned char commands_list_len) {
	assert(parcer);
	assert(handler);
	assert(commands_list);
	assert(commands_list_len);
	gril_stream_cmd_parcer_reinit(parcer);	
	parcer->handler = handler;
	parcer->commands_list = commands_list;
	parcer->commands_list_len = commands_list_len;
}


LOCAL void ICACHE_FLASH_ATTR
complete_parce(GrilStreamCmdParcer * parcer, enum GrilStreamCmdParcerError error, void * user_data) {
	*parcer->pos_prefix = 0;
	*parcer->pos_cmd = 0;
	*parcer->pos_param = 0;
	*parcer->pos_value = 0;
	parcer->handler(error, parcer->prefix, parcer->cmd, parcer->param, parcer->value, user_data);
	gril_stream_cmd_parcer_reinit(parcer);
}

void ICACHE_FLASH_ATTR
gril_stream_cmd_parcer_parce(GrilStreamCmdParcer * parcer, const char * data, int data_size, void * user_data) {
	//#ifdef ASSERT
	assert(parcer);
	assert(data);
	//#endif
	int i;

	#ifdef TEST_PRINT
	fprintf(stderr, "gril_stream_cmd_parcer_parce data:%s data_size:%d\n", data, data_size);
	#endif
	const char * data_end = &data[data_size]; 

	//[%prefix%][name[,parameter][,value][:option]]<eoc>
	while (data < data_end) {
		#ifdef TEST_PRINT
		fprintf(stderr, "gril_stream_cmd_parcer_parce char:%c\n", *data);
		#endif

		//end
		if (*data == GRIL_CMD_COMPLETE_CHAR) {
			#ifdef TEST_PRINT
			fprintf(stderr, "gril_stream_cmd_parcer_parce 1\n");
			#endif
			switch (parcer->state) {
				case ReadParamSplitter:
				case ReadParamState:
				case ReadDataState:
					#ifdef TEST_PRINT
					fprintf(stderr, "gril_stream_cmd_parcer_parce 2\n");
					#endif
					complete_parce(parcer, GrilStreamCmdParcerNoError, user_data);
					break;
				case BeginParceState:
					#ifdef TEST_PRINT
					fprintf(stderr, "gril_stream_cmd_parcer_parce 3\n");
					#endif

					complete_parce(
						parcer,
						parcer->pos_prefix == parcer->prefix ? GrilStreamCmdParcerErrorNotComplete : GrilStreamCmdParcerNoError,
						user_data);
					break;
				default:
					#ifdef TEST_PRINT
					fprintf(stderr, "gril_stream_cmd_parcer_parce 4\n");
					#endif

					complete_parce(parcer, GrilStreamCmdParcerErrorNotComplete, user_data);
			}
		}

		switch (parcer->state) {
			case BeginParceState:
				#ifdef TEST_PRINT
				fprintf(stderr, "gril_stream_cmd_parcer_parce BeginParceState\n");
				#endif

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
				#ifdef TEST_PRINT
				fprintf(stderr, "gril_stream_cmd_parcer_parce ReadPrefixState\n");
				#endif
				switch (*data) {
					case '%':
						parcer->state = BeginParceState;
						break;
					default:
						if (parcer->pos_prefix == &parcer->prefix[sizeof(parcer->prefix)]) {
							//complete_parce(parcer, handler, GrilStreamCmdParcerErrorPrefixTooLong, user_data);
							gril_stream_cmd_parcer_reinit(parcer);
						} else
							*(parcer->pos_prefix++) = *data;
						break;
				}
				break;
			case ReadCmdState:
				#ifdef TEST_PRINT
				fprintf(stderr, "gril_stream_cmd_parcer_parce ReadCmdState\n");
				#endif
				if (parcer->pos_cmd == &parcer->cmd[sizeof(parcer->cmd)]){
					//complete_parce(parcer, handler, GrilStreamCmdParcerErrorCmdTooLong, user_data);
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
						//complete_parce(parcer, handler, GrilStreamCmdParcerErrorUnknownCmd, user_data);
						gril_stream_cmd_parcer_reinit(parcer);
				}
				break;
			case ReadParamSplitter:
				#ifdef TEST_PRINT
				fprintf(stderr, "gril_stream_cmd_parcer_parce ReadParamSplitter\n");
				#endif
				if (*data != ','){
					//complete_parce(parcer, handler, GrilStreamCmdParcerErrorWnongParamSplitter, user_data);
					gril_stream_cmd_parcer_reinit(parcer);
				} else
					parcer->state = ReadParamState;
				break;
			case ReadParamState:
				#ifdef TEST_PRINT
				fprintf(stderr, "gril_stream_cmd_parcer_parce ReadParamState\n");
				#endif

				if (parcer->pos_param == &parcer->param[sizeof(parcer->param)]) {
					//complete_parce(parcer, handler, GrilStreamCmdParcerErrorParamTooLong, user_data);
					gril_stream_cmd_parcer_reinit(parcer);
				} else {
					if (*data == ',')
						parcer->state = ReadDataState;
					else
						*(parcer->pos_param++) = *data;
				}
				break;
			case ReadDataState:
				#ifdef TEST_PRINT
				fprintf(stderr, "gril_stream_cmd_parcer_parce ReadDataState\n");
				#endif

				if (parcer->pos_value == &parcer->value[sizeof(parcer->value)]) {
					//complete_parce(parcer, handler, GrilStreamCmdParcerErrorDataTooLong, user_data);
					gril_stream_cmd_parcer_reinit(parcer);
				} else
					*(parcer->pos_value++) = *data;
				break;
		}
		data++;
	}
}


#ifdef TEST
int main() {
	GrilStreamCmdParcer parcer;
	char * test_str[] = {"123123\n", "%12%\n", "%sdsd\n", "%%en\n", "%set\n", "print,\n", "set,xx/sd,23\n"};
	//char * test_str[] = {"%abc%set,xx/sd,23\r"};
	int i;
	static const char * errr_descs[] = {
		"GrilStreamCmdParcerNoError",
		"GrilStreamCmdParcerErrorPrefixNotComplete",
		"GrilStreamCmdParcerErrorPrefixTooLong",
		"GrilStreamCmdParcerErrorNotComplete",
		"GrilStreamCmdParcerErrorCmdTooLong",
		"GrilStreamCmdParcerErrorUnknownCmd",
		"GrilStreamCmdParcerErrorWnongParamSplitter",
		"GrilStreamCmdParcerErrorParamTooLong",
		"GrilStreamCmdParcerErrorDataTooLong"
	};

	void parce_handler(enum GrilStreamCmdParcerError error, const char * prefix, const char * cmd, const char * param, const char * value, void * user_data) {
		fprintf(stderr, "parce_handler prefix:%s, cmd:%s param:%s, value:%s error:%d errr_descs: %s\n",
			prefix,
			cmd,
			param,
			value,
			error,
			errr_descs[error]);
	}

	GrilCommandNameDesc cmd_names[] = {{"print", 5}, {"set", 3}};
	gril_stream_cmd_parcer_init(&parcer, parce_handler, cmd_names, 2);


	for (i=0; i < sizeof(test_str)/sizeof(char *); i++) {
		char tmp[50];
		strcpy(tmp, test_str[i]);
		tmp[strlen(tmp) - 1] = 0;
		fprintf(stderr, "test str:%s\n", tmp);
		gril_stream_cmd_parcer_parce(&parcer, test_str[i], strlen(test_str[i]), NULL);	
	}
	return 0;
}
#endif
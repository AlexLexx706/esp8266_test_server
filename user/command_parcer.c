#include "command_parcer.h"
#include <assert.h>

//extern int os_strncmp(const char * str1, const char * str2, int len);
#ifdef TEST
	#include <string.h>
	#include <stdio.h>
	#define os_strncmp(str1, str2, len)\
		strncmp(str1, str2, len)
#endif

void command_parcer_init(CommandParcer * parcer) {
	assert(parcer);
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

static void complete_parce(CommandParcer * parcer, parce_res_handler handler, int res) {
	*parcer->pos_prefix = 0;
    *parcer->pos_cmd = 0;
    *parcer->pos_param = 0;
    *parcer->pos_value = 0;
	handler(parcer, res);
	command_parcer_init(parcer);
}

static const char * commands_names[] = {"print", "set", "en", "de"};   

void command_parcer_parce(CommandParcer * parcer, const char * data, int data_size, parce_res_handler handler) {
	assert(parcer);
	assert(data);
	assert(handler);
	int i;

	#ifdef TEST_PRINT
	fprintf(stderr, "command_parcer_parce data:%s data_size:%d\n", data, data_size);
	#endif
	const char * data_end = &data[data_size]; 

	//[%prefix%][name[,parameter][,value][:option]]<eoc>
	while (data < data_end) {
		#ifdef TEST_PRINT
		fprintf(stderr, "command_parcer_parce char:%c\n", *data);
		#endif

		//end
		if (*data == '\r') {
			#ifdef TEST_PRINT
			fprintf(stderr, "command_parcer_parce 1\n");
			#endif
			switch (parcer->state) {
				case ReadParamSplitter:
				case ReadParamState:
				case ReadDataState:
					#ifdef TEST_PRINT
					fprintf(stderr, "command_parcer_parce 2\n");
					#endif

					complete_parce(parcer, handler, 1);
					break;
				case BeginParceState:
					#ifdef TEST_PRINT
					fprintf(stderr, "command_parcer_parce 3\n");
					#endif

					complete_parce(parcer, handler, parcer->pos_prefix != parcer->prefix);
					break;
				default:
					#ifdef TEST_PRINT
					fprintf(stderr, "command_parcer_parce 4\n");
					#endif

					complete_parce(parcer, handler, 0);
			}
		}

		switch (parcer->state) {
			case BeginParceState:
				#ifdef TEST_PRINT
				fprintf(stderr, "command_parcer_parce BeginParceState\n");
				#endif

				switch (*data) {
					case '%':
						parcer->state = ReadPrefixState;
						break;
					default:
						for (i = 0; i < sizeof(commands_names)/sizeof(commands_names[0]); i++) {
							if (commands_names[i][0] == *data) {
								parcer->state = ReadCmdState;
								*(parcer->pos_cmd++) = *data;
								break;
							}
						}
				}
				break;
			case ReadPrefixState:
				#ifdef TEST_PRINT
				fprintf(stderr, "command_parcer_parce ReadPrefixState\n");
				#endif
				switch (*data) {
					case '%':
						parcer->state = BeginParceState;
						break;
					default:
						if (parcer->pos_prefix == &parcer->prefix[sizeof(parcer->prefix)]) {
							complete_parce(parcer, handler, 0);
						} else
							*(parcer->pos_prefix++) = *data;
						break;
				}
				break;
			case ReadCmdState:
				#ifdef TEST_PRINT
				fprintf(stderr, "command_parcer_parce ReadCmdState\n");
				#endif
				if (parcer->pos_cmd == &parcer->cmd[sizeof(parcer->cmd)]){
					complete_parce(parcer, handler, 0);
				} else {
					*(parcer->pos_cmd++) = *data;
					int has_cmd = 0;
					for (i = 0; i < sizeof(commands_names)/sizeof(commands_names[0]); i++) {
						if (!os_strncmp(parcer->cmd, commands_names[i], parcer->pos_cmd - parcer->cmd)) {
							if (parcer->pos_cmd - parcer->cmd == strlen(commands_names[i])) {
								parcer->state = ReadParamSplitter;
							}
							has_cmd = 1;
							break;
						}
					}
					if (!has_cmd)
						complete_parce(parcer, handler, 0);
				}
				break;
			case ReadParamSplitter:
				#ifdef TEST_PRINT
				fprintf(stderr, "command_parcer_parce ReadParamSplitter\n");
				#endif
				if (*data != ','){
					complete_parce(parcer, handler, 0);
				} else
					parcer->state = ReadParamState;
				break;
			case ReadParamState:
				#ifdef TEST_PRINT
				fprintf(stderr, "command_parcer_parce ReadParamState\n");
				#endif

				if (parcer->pos_param == &parcer->param[sizeof(parcer->param)]) {
					complete_parce(parcer, handler, 0);
				} else {
					if (*data == ',')
						parcer->state = ReadDataState;
					else
						*(parcer->pos_param++) = *data;
				}
				break;
			case ReadDataState:
				#ifdef TEST_PRINT
				fprintf(stderr, "command_parcer_parce ReadDataState\n");
				#endif

				if (parcer->pos_value == &parcer->value[sizeof(parcer->value)]) {
					complete_parce(parcer, handler, 0);
				} else
					*(parcer->pos_value++) = *data;
				break;
		}
		data++;
	}
}


#ifdef TEST
int main() {
	CommandParcer parcer;
	command_parcer_init(&parcer);
	char * test_str[] = {"123123\r", "%12%\r", "%sdsd\r", "%%en\r", "%set\r", "print,\r", "set,xx/sd,23\r"};
	//char * test_str[] = {"%abc%set,xx/sd,23\r"};
	int i;

	void parce_handler(CommandParcer * parcer, int is_ok) {
		fprintf(stderr, "parce_handler prefix:%s, cmd:%s param:%s, value:%s res:%d \n",
			parcer->prefix,
			parcer->cmd,
			parcer->param,
			parcer->value,
			is_ok);
	}

	for (i=0; i < sizeof(test_str)/sizeof(char *); i++) {
		char tmp[50];
		strcpy(tmp, test_str[i]);
		tmp[strlen(tmp) - 1] = 0;
		fprintf(stderr, "test str:%s\n", tmp);
		command_parcer_parce(&parcer, test_str[i], strlen(test_str[i]), parce_handler);	
	}
	return 0;
}
#endif
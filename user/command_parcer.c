#include "command_parcer.h"
#include <assert.h>

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

void command_parcer_parce(CommandParcer * parcer, const char * data, int data_size, parce_res_handler handler) {
	assert(parcer);
	assert(data);
	assert(handler);
	
	//[%prefix%][name[,parameter][,value][:option]]<eoc>
	while (data != &data[data_size]) {
		//end
		if (*data == '\r') {
			switch (parcer->state) {
				case ReadParamSplitter:
				case ReadParamState:
				case ReadDataState:
					complete_parce(parcer, handler, 1);
					break;
				case BeginParceState:
					complete_parce(parcer, handler, parcer->pos_prefix != parcer->prefix);
					break;
				default:
					complete_parce(parcer, handler, 0);
			}
		}

		switch (parcer->state) {
			BeginParceState:
				switch (*data) {
					case '%':
						parcer->state = ReadPrefixState;
						break;
					case 's':
					case 'p':
					case 'e':
					case 'd':
						parcer->state = ReadCmdState;
						*(parcer->pos_cmd++) = *data;
						break;
				}
				break;
			ReadPrefixState:
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
			ReadCmdState:
				if (parcer->pos_cmd == &parcer->cmd[sizeof(parcer->cmd)]){
					complete_parce(parcer, handler, 0);
				} else {
					*(parcer->pos_cmd++) = *data;
					
					if (!os_strncmp(parcer->cmd, "print", parcer->pos_cmd - parcer->cmd) || 
						!os_strncmp(parcer->cmd, "set", parcer->pos_cmd - parcer->cmd) ||
						!os_strncmp(parcer->cmd, "en", parcer->pos_cmd - parcer->cmd) ||
						!os_strncmp(parcer->cmd, "de", parcer->pos_cmd - parcer->cmd)) {
						parcer->state = ReadParamSplitter;
					} else {
						complete_parce(parcer, handler, 0);
					}
				}
				break;
			ReadParamSplitter:
				if (*data != ','){
					complete_parce(parcer, handler, 0);
				} else
					parcer->state = ReadParamState;
					parcer->pos_param = parcer->param;
				break;
			ReadParamState:
				if (parcer->pos_param == &parcer->param[sizeof(parcer->param)]) {
					complete_parce(parcer, handler, 0);
				} else {
					*(parcer->pos_param++) = *data;
					
					if (*data == ',')
						parcer->state = ReadDataState;
				}
				break;
			ReadDataState:
				if (parcer->pos_value == &parcer->value[sizeof(parcer->value)]) {
					complete_parce(parcer, handler, 0);
				} else
					*(parcer->pos_param++) = *data;
				break;
		}
		data++;
	}
}

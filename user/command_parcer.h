#ifndef _COMMAND_PARCER_H_
#define _COMMAND_PARCER_H_

enum CommandParcerState {
    BeginParceState,
    ReadPrefixState,
    ReadCmdState,
    ReadParamSplitter,
    ReadParamState,
    ReadDataState,
};

typedef struct CommandParcer {
	enum CommandParcerState state;
    char prefix[10];
    char cmd[10];
    char param[20];
    char value[20];
    int is_ok;

    char * pos_prefix;
    char * pos_cmd;
    char * pos_param;
    char * pos_value;
} CommandParcer;

typedef void (*parce_res_handler)(CommandParcer * parcer, int is_ok);

void command_parcer_init(CommandParcer * parcer);
void command_parcer_parce(CommandParcer * parcer, const char * data, int data_size, parce_res_handler handler);

#endif //_COMMAND_PARCER_H_

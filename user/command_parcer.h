#ifndef _COMMAND_PARCER_H_
#define _COMMAND_PARCER_H_

enum CommandParcerState {
    BeginParceState = 0,
    ReadPrefixState,
    ReadCmdState,
    ReadParamSplitter,
    ReadParamState,
    ReadDataState,
};

enum CommandParcerError {
    CommandParcerNoError = 0,
    CommandParcerErrorPrefixNotComplete,
    CommandParcerErrorPrefixTooLong,
    CommandParcerErrorNotComplete,
    CommandParcerErrorCmdTooLong,
    CommandParcerErrorUnknownCmd,
    CommandParcerErrorWnongParamSplitter,
    CommandParcerErrorParamTooLong,
    CommandParcerErrorDataTooLong
};

typedef struct CommandParcer {
	enum CommandParcerState state;
    char prefix[10];
    char cmd[10];
    char param[20];
    char value[20];

    char * pos_prefix;
    char * pos_cmd;
    char * pos_param;
    char * pos_value;
} CommandParcer;

typedef void (*parce_res_handler)(CommandParcer * parcer, enum CommandParcerError error, void * user_data);

void command_parcer_init(CommandParcer * parcer);

//parse byte stream, search this template: [%prefix%][name[,parameter][,value][:option]]<eoc>
void command_parcer_parce(CommandParcer * parcer, const char * data, int data_size, parce_res_handler handler, void * user_data);

#endif //_COMMAND_PARCER_H_

/**alexlexx 2017*/
#ifndef _gril_stream_cmd_parcer_H_
#define _gril_stream_cmd_parcer_H_

#define GRIL_CMD_COMPLETE_CHAR '\n'
#define GRIL_PARCER_PREFIX_LEN 10
#define GRIL_PARCER_CMD_LEN 10
#define GRIL_PARCER_PARAMS_LEN 20
#define GRIL_PARCER_VALUE_LEN 20

//parcer errors enum
enum GrilStreamParcerError {
	GrilStreamParcerNoError = 0,
	GrilStreamParcerErrorPrefixNotComplete,
	GrilStreamParcerErrorPrefixTooLong,
	GrilStreamParcerErrorNotComplete,
	GrilStreamParcerErrorCmdTooLong,
	GrilStreamParcerErrorUnknownCmd,
	GrilStreamParcerErrorWnongParamSplitter,
	GrilStreamParcerErrorParamTooLong,
	GrilStreamParcerErrorDataTooLong,
	GrilStreamParcerErrorWrongParam,
};

//Strict definition use for describe commands, example: {{"print", 5},{"set", 3}}
typedef struct GrilCommandNameDesc_t {
	const char * name;
	unsigned char name_len;
} GrilCommandNameDesc;


/**
	Struct used for send gril result back
	sender - user data
	send_fun -function for send result back
*/
typedef struct GrilStreamParcerResultSender_t{
	void * sender;
	void (* fun_send)(void * sender, const char * res_str, int str_len);	
} GrilStreamParcerResultSender;

/**
	Struct use as parametr for function GrilStreamParcerHandler
	error - parcer error
	prefix - prefix string, can be empty string
	cmd - command
	param - pamert string 
	value - value string, can be empty string
	sender - use for send back result

*/
typedef struct GrilStreamParcerResult_t {
	enum GrilStreamParcerError error;
	const char * prefix;
	const char * cmd;
	const char * param;
	const char * value;
	GrilStreamParcerResultSender * sender;
} GrilStreamParcerResult;

/**
	Function for process parcer result 
*/
typedef void (*GrilStreamParcerHandler)(GrilStreamParcerResult * res);

/**
	Gril parcer struct
*/
typedef struct {
	int state;
	char prefix[GRIL_PARCER_PREFIX_LEN];
	char cmd[GRIL_PARCER_CMD_LEN];
	char param[GRIL_PARCER_PARAMS_LEN];
	char value[GRIL_PARCER_VALUE_LEN];
	char * pos_prefix;
	char * pos_cmd;
	char * pos_param;
	char * pos_value;
	GrilStreamParcerHandler handler;
	GrilStreamParcerResultSender * sender;
	GrilCommandNameDesc * commands_list;
	unsigned char commands_list_len;
} GrilStreamParcer;

/*
	Init gril parcer struct.
	params:
		parcer - pointer on parcer struct
		handler - cmd handler function
		sender - use for send gril result
		commands_list - pointer on commands list
		commands_list_len - commands list len
*/
void gril_stream_parcer_init(
	GrilStreamParcer * parcer,
	GrilStreamParcerHandler handler,
	GrilStreamParcerResultSender * sender, 
	GrilCommandNameDesc * commands_list,
	unsigned char commands_list_len);

/*
	Parse byte stream, search this template: [%prefix%][name[,parameter][,value][:option]]<eoc>
	params:
		parcer - pointer to parcer
		data - pointer to array of char to parce
		data_size - size of array
**/
void gril_stream_parcer_parce(
	GrilStreamParcer * parcer,
	const char * data,
	int data_size);

#endif //_gril_stream_cmd_parcer_H_

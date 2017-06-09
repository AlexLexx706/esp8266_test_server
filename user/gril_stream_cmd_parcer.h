/**alexlexx 2017*/
#ifndef _gril_stream_cmd_parcer_H_
#define _gril_stream_cmd_parcer_H_

#define GRIL_CMD_COMPLETE_CHAR '\n'
#define GRIL_PARCER_PREFIX_LEN 10
#define GRIL_PARCER_CMD_LEN 10
#define GRIL_PARCER_PARAMS_LEN 20
#define GRIL_PARCER_VALUE_LEN 20

//parcer errors enum
enum GrilStreamCmdParcerError {
	GrilStreamCmdParcerNoError = 0,
	GrilStreamCmdParcerErrorPrefixNotComplete,
	GrilStreamCmdParcerErrorPrefixTooLong,
	GrilStreamCmdParcerErrorNotComplete,
	GrilStreamCmdParcerErrorCmdTooLong,
	GrilStreamCmdParcerErrorUnknownCmd,
	GrilStreamCmdParcerErrorWnongParamSplitter,
	GrilStreamCmdParcerErrorParamTooLong,
	GrilStreamCmdParcerErrorDataTooLong,
	GrilStreamCmdParcerErrorWrongParam,
};

//Strict definition use for describe commands, example: {{"print", 5},{"set", 3}}
typedef struct GrilCommandNameDesc_t {
	const char * name;
	unsigned char name_len;
} GrilCommandNameDesc;


//Command handler function definition, call when parcer complate search cmd
typedef void (*parce_res_handler)(
	enum GrilStreamCmdParcerError error,
	const char * prefix,
	const char * cmd,
	const char * param,
	const char * value,
	void * user_data);

//define gril parcer struct
typedef struct GrilStreamCmdParcer_t {
	int state;
	char prefix[GRIL_PARCER_PREFIX_LEN];
	char cmd[GRIL_PARCER_CMD_LEN];
	char param[GRIL_PARCER_PARAMS_LEN];
	char value[GRIL_PARCER_VALUE_LEN];

	char * pos_prefix;
	char * pos_cmd;
	char * pos_param;
	char * pos_value;
	parce_res_handler handler;
	GrilCommandNameDesc * commands_list;
	unsigned char commands_list_len;
} GrilStreamCmdParcer;

/*
	Init gril parcer struct.
	params:
		parcer - pointer on parcer struct
		handler - cmd handler function
		commands_list - pointer on commands list
		commands_list_len - commands list len
*/
void gril_stream_cmd_parcer_init(
	GrilStreamCmdParcer * parcer,
	parce_res_handler handler, 
	GrilCommandNameDesc * commands_list,
	unsigned char commands_list_len);

/*
	Parse byte stream, search this template: [%prefix%][name[,parameter][,value][:option]]<eoc>
	params:
		parcer - pointer to parcer
		data - pointer to array of char to parce
		data_size - size of array
		user_data - pointer to user data wich sent to cmd handler
**/
void gril_stream_cmd_parcer_parce(
	GrilStreamCmdParcer * parcer,
	const char * data,
	int data_size,
	void * user_data);

#endif //_gril_stream_cmd_parcer_H_

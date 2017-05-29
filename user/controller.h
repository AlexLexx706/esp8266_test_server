#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "gril_stream_cmd_parcer.h"

typedef struct Userdata {
    void * socket;
    void (*send_data)(void *socket, char *string);

} Userdata;

void process_commands(
    enum GrilStreamCmdParcerError error,
    const char * prefix,
    const char * cmd,
    const char * param,
    const char * value,
    void * user_data);
#endif
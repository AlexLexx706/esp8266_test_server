#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "command_parcer.h"

typedef struct Userdata {
    void * socket;
    void (*send_data)(void *socket, char *string);

} Userdata;

void process_commands(CommandParcer * parcer, enum CommandParcerError error, Userdata * user_data);
#endif
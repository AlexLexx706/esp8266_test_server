#include "controller.h"
#include "user_interface.h"
#include "osapi.h"

void ICACHE_FLASH_ATTR
process_commands(CommandParcer * parcer, enum CommandParcerError error, Userdata * user_data) {
    char buffer[128];
    //Userdata * user_data = (Userdata *)_user_data;

    if (error == CommandParcerNoError) {
        int res = 0;
        if (!os_strcmp(parcer->cmd, "set")) {
            if (!os_strcmp(parcer->param, "light")) {
                if (!os_strcmp(parcer->value, "on")) {
                    res = 1;
                    gpio_output_set(0, BIT2, BIT2, 0);
                } else if (!os_strcmp(parcer->value, "off")) {
                    res = 1;
                    //Set GPIO2 to LOW
                    gpio_output_set(BIT2, 0, BIT2, 0);
                }
            }
        } else if (!os_strcmp(parcer->cmd, "print")) {
            if (!os_strcmp(parcer->param, "light")) {
                res = 1;
            }
        }
        if (res)
            os_sprintf(buffer, "no_error\n");
        else
            os_sprintf(buffer, "wrong_params\n");
    } else {
        os_sprintf(buffer, "errno:%d\n",error);
    }
    user_data->send_data(user_data->socket, buffer);
}

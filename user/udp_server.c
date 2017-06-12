#include "udp_server.h"
#include "user_interface.h"
#include "espconn.h"
#include "osapi.h"
#include "controller.h"

uint32 receive_bytes_count = 0;
GrilStreamParcer command_parcer;
/******************************************************************************
 * FunctionName : data_send
 * Description  : processing the data as http format and send to the client or server
 * Parameters   : arg -- argument to set for client or server
 *                responseOK -- true or false
 *                psend -- The send data
 * Returns      :
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
data_send(void *arg, char *psend)
{
    if (!psend)
        return;

    struct espconn *pesp_conn = arg;
    int length = os_strlen(psend);
    espconn_sent(pesp_conn, psend, length);
}


/******************************************************************************
 * FunctionName : webserver_recv
 * Description  : Processing the received data from the server
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pusrdata -- The received data (or NULL when the connection has been closed!)
 *                length -- The length of received data
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
webserver_recv(void *arg, char *pusrdata, unsigned short length)
{
    receive_bytes_count += length;
    Userdata data = {arg, data_send};
    gril_stream_parcer_parce(&command_parcer, pusrdata, length,  &data);
}




/******************************************************************************
 * FunctionName : user_webserver_init
 * Description  : parameter initialize as a server
 * Parameters   : port -- server port
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_webserver_init(uint32 port)
{
    LOCAL struct espconn esp_conn;
    //LOCAL esp_tcp esptcp;
    LOCAL esp_udp espudp;
    LOCAL GrilCommandNameDesc cmd_names[] = {{"print", 5}, {"set", 3}};

    gril_stream_parcer_init(
        &command_parcer,
        (GrilStreamParcerHandler)controller_process_commands,
        cmd_names,
        2);

    //esp_conn.type = ESPCONN_TCP;
    esp_conn.type = ESPCONN_UDP;
    esp_conn.state = ESPCONN_NONE;
    esp_conn.proto.udp = &espudp;
    esp_conn.proto.udp->local_port = port;
    espconn_create(&esp_conn);
    espconn_regist_recvcb(&esp_conn, webserver_recv);
}
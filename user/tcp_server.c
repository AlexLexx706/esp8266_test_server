#include "tcp_server.h"
#include "user_interface.h"
#include "espconn.h"
#include "osapi.h"
#include "command_parcer.h"

uint32 receive_bytes_count = 0;
CommandParcer command_parcer;
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


LOCAL void ICACHE_FLASH_ATTR
parce_handler(CommandParcer * parcer, enum CommandParcerError error, void * user_data) {
    char buffer[128];
    os_sprintf(buffer,
        "prefix:%s cmd:%s param:%s value:%s error:%d\n",
            parcer->prefix,
            parcer->cmd,
            parcer->param,
            parcer->value,
            error);
    data_send(user_data, buffer);
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
    int i;
    for (i = 0; i < length; i++) {
        os_printf("%c", pusrdata[i]);
    }
    os_printf("\n", pusrdata[i]);

    struct espconn *pesp_conn = arg;
    // os_printf("webserver's %d.%d.%d.%d:%d receive data_len: %u\n",
    //  pesp_conn->proto.tcp->remote_ip[0],
    //  pesp_conn->proto.tcp->remote_ip[1],
    //  pesp_conn->proto.tcp->remote_ip[2],
    //  pesp_conn->proto.tcp->remote_ip[3]
    //  ,pesp_conn->proto.tcp->remote_port,
    //  length);
    receive_bytes_count += length;
    command_parcer_parce(&command_parcer, pusrdata, length, parce_handler, arg);
}

/******************************************************************************
 * FunctionName : webserver_recon
 * Description  : the connection has been err, reconnection
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL ICACHE_FLASH_ATTR
void webserver_recon(void *arg, sint8 err)
{
    struct espconn *pesp_conn = arg;

    os_printf("webserver's %d.%d.%d.%d:%d err %d reconnect\n", pesp_conn->proto.tcp->remote_ip[0],
        pesp_conn->proto.tcp->remote_ip[1],pesp_conn->proto.tcp->remote_ip[2],
        pesp_conn->proto.tcp->remote_ip[3],pesp_conn->proto.tcp->remote_port, err);
}

/******************************************************************************
 * FunctionName : webserver_recon
 * Description  : the connection has been err, reconnection
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL ICACHE_FLASH_ATTR
void webserver_discon(void *arg)
{
    struct espconn *pesp_conn = arg;

    os_printf("webserver's %d.%d.%d.%d:%d disconnect\n", pesp_conn->proto.tcp->remote_ip[0],
            pesp_conn->proto.tcp->remote_ip[1],pesp_conn->proto.tcp->remote_ip[2],
            pesp_conn->proto.tcp->remote_ip[3],pesp_conn->proto.tcp->remote_port);
}

/******************************************************************************
 * FunctionName : user_accept_listen
 * Description  : server listened a connection successfully
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
webserver_listen(void *arg)
{
    struct espconn *pesp_conn = arg;

    espconn_regist_recvcb(pesp_conn, webserver_recv);
    espconn_regist_reconcb(pesp_conn, webserver_recon);
    espconn_regist_disconcb(pesp_conn, webserver_discon);
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
    LOCAL esp_tcp esptcp;
    sint8 res;
    command_parcer_init(&command_parcer);

    esp_conn.type = ESPCONN_TCP;
    //esp_conn.type = ESPCONN_UDP;
    esp_conn.state = ESPCONN_NONE;
    esp_conn.proto.tcp = &esptcp;
    esp_conn.proto.tcp->local_port = port;
    res = espconn_regist_connectcb(&esp_conn, webserver_listen);
    os_printf("espconn_regist_connectcb res:%d\n", res);

    #ifdef SERVER_SSL_ENABLE
        res = espconn_secure_accept(&esp_conn);
         os_printf("espconn_secure_accept port:%u res:%d\n", port, res);
    #else
        res = espconn_accept(&esp_conn);
        os_printf("espconn_accept port:%u res:%d\n", port, res);
    #endif
    espconn_regist_time(&esp_conn, 60*60, 0);
 
}
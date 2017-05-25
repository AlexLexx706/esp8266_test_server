#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "espconn.h"
#include "utils.h"
#include "math.h"

static volatile os_timer_t some_timer;
extern uint32 receive_bytes_count;

static void loop(os_event_t *events);
void user_webserver_init(uint32 port);

void some_timerfunc(void *arg)
{
	LOCAL uint32 last_bytes_count = 0;
	char tmp_buffer[24];
	double speed = (receive_bytes_count - last_bytes_count) / 1024.;
	last_bytes_count = receive_bytes_count;
	ftoa(speed, tmp_buffer, 4);
	os_printf("receive_bytes_count:%u speed:%s kB/s\n", receive_bytes_count, tmp_buffer);
}



//Init function 
void ICACHE_FLASH_ATTR
user_init()
{
	char ssid[32] = SSID;
	char password[64] = SSID_PASSWORD;
	struct station_config stationConf;

	char ip_buffer[24];
	char mask_buffer[24];
	char gw_buffer[24];
	struct ip_info info;

	//setup uart
	uart_div_modify( 0, UART_CLK_FREQ / (115200) );
	os_printf( "%s\n", __FUNCTION__ );

	//Set station mode
	wifi_set_opmode(0x1);

	//Set ap settings
	os_memcpy(&stationConf.ssid, ssid, 32);
	os_memcpy(&stationConf.password, password, 64);
	wifi_station_set_config(&stationConf);
	wifi_station_dhcpc_start();

	if (wifi_get_ip_info(0x0, &info)) {
		os_printf("ip:%s mask:%s gw:%s\n",
			ipaddr_ntoa_r(&info.ip, ip_buffer, sizeof(ip_buffer)),
			ipaddr_ntoa_r(&info.netmask, mask_buffer, sizeof(mask_buffer)),
			ipaddr_ntoa_r(&info.gw, gw_buffer, sizeof(gw_buffer)));
	}

	gpio_init();

    //Set GPIO2 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

    //Set GPIO2 low
    gpio_output_set(0, BIT2, BIT2, 0);


	//setup timer
	//Disarm timer
	os_timer_disarm(&some_timer);

	//Setup timer
	os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);

	//Arm the timer
	//&some_timer is the pointer
	//1000 is the fire time in ms
	//0 for once and 1 for repeating
	os_timer_arm(&some_timer, 1000, 1);

	user_webserver_init(80);
}
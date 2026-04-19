#include "httpd.c"
#include "wifi-src.c"

#include "nvs_flash.h"

void app_main(void){
	ESP_ERROR_CHECK(nvs_flash_init());
	wifi_init_sta(); 
	start_webserver();
}

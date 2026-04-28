#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "mdns.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define WIFI_CONNECTED_BIT BIT1
#define WIFI_FAIL BIT0

static EventGroupHandle_t s_wifi_event_group; 
static const char *TAG = "Wifi Notify"; 
//static int s_retry_num = 0; 

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
		esp_wifi_connect(); 
	}	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED){
			esp_wifi_connect(); 
			ESP_LOGI(TAG, "Retry connecting"); 
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL); 
		}
		else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
			ip_event_got_ip_t *event = (ip_event_got_ip_t*)event_data; 
			ESP_LOGI(TAG, "got IP:" IPSTR, IP2STR(&event->ip_info.ip));
			xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT); 
		}
}


void wifi_init_sta(void){

	s_wifi_event_group = xEventGroupCreate(); 
	ESP_ERROR_CHECK(esp_netif_init()); 
	ESP_ERROR_CHECK(esp_event_loop_create_default()); 
	esp_netif_create_default_wifi_sta(); 

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg)); 

	esp_event_handler_instance_t instance_got_any_id; 
	esp_event_handler_instance_t instance_got_ip; 
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_got_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

	wifi_config_t wifi_config = {
		.sta = {
		 .ssid = "HUAWEI Y9s",
		.password = "taktau2lah",

		},
	};

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
       	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));	
	ESP_ERROR_CHECK(esp_wifi_start());	


	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL , pdFALSE, pdFALSE, portMAX_DELAY); 

	if (bits & WIFI_CONNECTED_BIT){
		ESP_LOGI(TAG, "Connected!");
	}
	else if (bits & WIFI_FAIL){
		ESP_LOGI(TAG, "Failed to connect.. Retrying.. "); 
	}
	else 
		ESP_LOGI(TAG, "Unexpected problem..");

	// Mdns
	esp_err_t err = mdns_init(); 
	if (err) {
		printf("MDNS Init failed: %d\n", err);
	}
	mdns_hostname_set("esp32");
	mdns_instance_name_set("Alif's Esp32");
	mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
}

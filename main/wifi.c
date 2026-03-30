#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_eap_client.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"

static EventGroupHandle_t wifi_event_group; 
static esp_netif_t *sta_netif = NULL;
static char *TAG = "Example";


static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {

	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		esp_wifi_connect();
	}
}

static void init_wifi(void){
	esp_eap_method_t eap_methods = ESP_EAP_TYPE_PEAP; 
	ESP_ERROR_CHECK(esp_netif_init()); 
	wifi_event_group = xEventGroupCreate(); 
	sta_netif = esp_netif_create_default_wifi_sta(); 
	assert(sta_netif); 


	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); 
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL)); 
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL)); 
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
	wifi_config_t wifi_config = {
		.sta.ssid = "UniMAP-WiFi"
	};

	ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_MODE_STA, &wifi_config));

	ESP_ERROR_CHECK(esp_eap_client_set_identity((uint8_t *)"241021382", strlen("241021381")));
	ESP_ERROR_CHECK(esp_eap_client_set_new_password((uint8_t *)"241021382", strlen("241021381")));
	
	ESP_ERROR_CHECK(esp_eap_client_set_eap_methods(eap_methods));
	ESP_ERROR_CHECK(esp_wifi_sta_enterprise_enable());
	ESP_ERROR_CHECK(esp_wifi_start());
	
}

static void wifi_enterprise_task_print_status(void *pvParameter)
{
	esp_netif_ip_info_t ip; 
	memset(&ip, 0, sizeof(esp_netif_ip_info_t)); 
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	while (1){
		vTaskDelay(2000 / portTICK_PERIOD_MS);

        if (esp_netif_get_ip_info(sta_netif, &ip) == 0) {
            ESP_LOGI(TAG, "~~~~~~~~~~~");
            ESP_LOGI(TAG, "IP:"IPSTR, IP2STR(&ip.ip));
            ESP_LOGI(TAG, "MASK:"IPSTR, IP2STR(&ip.netmask));
            ESP_LOGI(TAG, "GW:"IPSTR, IP2STR(&ip.gw));
            ESP_LOGI(TAG, "~~~~~~~~~~~");
        	}
	}
}

void app_main(void)
{
	ESP_ERROR_CHECK(nvs_flash_init());
	init_wifi();
	xTaskCreate(&wifi_enterprise_task_print_status, "wifi just print status", 4096, NULL, 5, NULL);
}

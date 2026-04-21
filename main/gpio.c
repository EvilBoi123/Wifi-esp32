#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_pm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define GPIO_NUM 4 
#define BIT_DURATION_MS 1 
#define NUM_BITS 2 

static QueueHandle_t gpio_evt_queue = NULL; 
static uint8_t received_flag = 0;

static void cfg_gpio(void){
	gpio_config_t cfg = {
		.pin_bit_mask = (1ULL << GPIO_NUM), 
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.intr_type = GPIO_INTR_NEGEDGE,
	};

	gpio_config(&cfg);
	}

static void IRAM_ATTR gpio_isr_handler(void *arg){
	uint32_t pin = (uint32_t) arg;
	xQueueSendFromISR(gpio_evt_queue, &pin, NULL); 
}

static void data_task(void *arg){
	uint32_t pin; 
	while (1){
		if (xQueueReceive(gpio_evt_queue, &pin, portMAX_DELAY)){
			vTaskDelay(pdMS_TO_TICKS(BIT_DURATION_MS + BIT_DURATION_MS / 2));

			uint8_t value; 

			for (int i=0; i < NUM_BITS; i++){
				int bit = gpio_get_level(GPIO_NUM); 
				value |= (bit << i);
				vTaskDelay(pdMS_TO_TICKS(BIT_DURATION_MS));
			}

			received_flag = value; 

			ESP_LOGI("DATA", "Received flag value: %d", received_flag); 
			vTaskDelay(pdMS_TO_TICKS(BIT_DURATION_MS));
		}
	}	
}

static void setup_gpio(void){
	cfg_gpio();
	gpio_evt_queue = xQueueCreate(5, sizeof(uint32_t));

	xTaskCreate(data_task, "data_task", 2048, NULL, 10, NULL);
	gpio_install_isr_service(0);
	gpio_isr_handler_add(GPIO_NUM, gpio_isr_handler, (void *)GPIO_NUM);
}

#include "esp_http_server.h"

extern const char index_html_start[] asm("_binary_index_html_start");
extern const char index_html_end[] asm("_binary_index_html_end");

esp_err_t root_handler(httpd_req_t *req){
	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, index_html_start, index_html_end - index_html_start); 
	return ESP_OK; 
};

void start_webserver(void){
	httpd_handle_t server = NULL; 
	httpd_config_t config = HTTPD_DEFAULT_CONFIG(); 

	httpd_start(&server, &config); 

	httpd_uri_t root = {
		.uri = "/",
		.method = HTTP_GET,
		.handler = root_handler,
	};

	httpd_register_uri_handler(server, &root); 
};

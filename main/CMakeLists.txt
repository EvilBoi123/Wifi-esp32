idf_component_register(SRCS "main.c"
		   EMBED_TXTFILES "index.html" 
                    INCLUDE_DIRS "."
		    REQUIRES mdns
		    	esp_http_server
			esp_system
			esp_wifi
			nvs_flash
			esp_driver_ledc
			esp_netif
			esp_pm
	    )

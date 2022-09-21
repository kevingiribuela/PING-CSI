idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES WiFi nvs_flash esp_http_server WebServer)

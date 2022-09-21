#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include <esp_http_server.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_wifi.h"
#include "ctype.h"

extern bool parametters;
extern nvs_handle_t my_handle;


esp_err_t homepage_get_handler(httpd_req_t *req);

esp_err_t data_get_handler(httpd_req_t *req);

httpd_handle_t start_webserver(void);

void stop_webserver(httpd_handle_t server);

void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

char *urlDecode(const char *str);
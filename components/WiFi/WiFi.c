#include <stdio.h>
#include "WiFi.h"

/* FreeRTOS event group to signal when we are connected*/
extern EventGroupHandle_t s_wifi_event_group;
extern bool wifi_ok;
extern nvs_handle_t my_handle;

#define WIFI_SSID       "WIMUMO"
#define WIFI_PASS       "defiunlp2022"
#define WIFI_CHANNEL    1
#define MAX_STA_CONN    1


#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


esp_netif_t * wifi_init_softap(void){
    // It is recommended firstable initialize the Wi-Fi module with the default configuration
    esp_netif_t *wifi_object= esp_netif_create_default_wifi_ap();   // Creates default WIFI AP
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();            // Load default WiFi structure
    esp_wifi_init(&cfg);                                            // Initialize WiFi

    wifi_config_t wifi_config = {                   // WiFi structure configuration
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = WIFI_CHANNEL,
            .password = WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    esp_wifi_set_mode(WIFI_MODE_AP);               // Set WiFi mode to Acces Point  
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config); // Set WiFi structure configuration
    esp_wifi_start();                              // Start WiFi

    printf( "Inicialización de acces point exitosa!\n");
    printf("SSID: %s\n", WIFI_SSID);
    printf("PSWD: %s\n", WIFI_PASS);
    printf("CANAL: %d\n", WIFI_CHANNEL);
    return wifi_object;
}

esp_netif_t* wifi_init_sta(void){

    uint8_t protocol_bitmap;
    int canal;

    // It is recommended firstable initialize the Wi-Fi module with the default configuration
    esp_netif_t *wifi_object = esp_netif_create_default_wifi_sta(); // Creates default WiFi STA
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();            // Load default WiFi structure
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));                           // Initialize WiFi
    
    nvs_open("wifi",NVS_READWRITE, &my_handle);         // Open the nvs in read/write mode
    size_t required_size;
    
    nvs_get_str(my_handle, "SSID", NULL, &required_size);  // Get the required size, and value of the SSID from NVS
    char *wifi_ssid = malloc(required_size);
    nvs_get_str(my_handle, "SSID", wifi_ssid, &required_size);
    
    nvs_get_str(my_handle, "PSWD", NULL, &required_size);  // Get the required size, and value of the PSWD from NVS
    char *wifi_pswd = malloc(required_size);
    nvs_get_str(my_handle, "PSWD", wifi_pswd, &required_size);

    wifi_config_t wifi_config = {                           // WiFi structure configuration
        .sta = 
        {            
            .pmf_cfg =
            {
                .capable = true,
                .required = false
            },
        },
    };
    if(*wifi_pswd =='\0'){
        wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
        strcpy((char *)wifi_config.sta.password, "");
    }
    else{
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        strcpy((char *)wifi_config.sta.password, wifi_pswd);
    }
    strcpy((char *)wifi_config.sta.ssid, wifi_ssid);
    
    nvs_get_str(my_handle, "CANAL", NULL, &required_size);  // Get the required size, and value of the SSID from NVS
    char *esp32_channel = malloc(required_size);
    nvs_get_str(my_handle, "CANAL", esp32_channel, &required_size);

    canal = atoi(esp32_channel);
    switch (canal)
    {
        case 1:
             wifi_config.sta.channel = canal;
            break;
        case 2:
             wifi_config.sta.channel = canal;
            break;
        case 3:
             wifi_config.sta.channel = canal;
            break;
        case 4:
             wifi_config.sta.channel = canal;
            break;
        case 5:
             wifi_config.sta.channel = canal;
            break;
        case 6:
             wifi_config.sta.channel = canal;
            break;
        case 7:
             wifi_config.sta.channel = canal;
            break;
        case 8:
             wifi_config.sta.channel = canal;
            break;
        case 9:
             wifi_config.sta.channel = canal;
            break;
        case 10:
             wifi_config.sta.channel = canal;
            break;
        case 11:
             wifi_config.sta.channel = canal;
            break;
        case 12:
             wifi_config.sta.channel = canal;
            break;
        case 13:
             wifi_config.sta.channel = canal;
            break;
        default:
            break;
    }

    nvs_get_str(my_handle, "PROTOCOLO", NULL, &required_size);  // Get the required size, and value of the SSID from NVS
    char *protocolo = malloc(required_size);
    nvs_get_str(my_handle, "PROTOCOLO", protocolo, &required_size);
    if(strcmp(protocolo,"b")==0){
        protocol_bitmap = WIFI_PROTOCOL_11B;
    }
    else if(strcmp(protocolo, "bg")==0){
        protocol_bitmap = WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G;
    }
    else{
        protocol_bitmap = WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N;
    }
    
    esp_wifi_set_mode(WIFI_MODE_STA);                   // Set WiFi mode to Station  
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);     // Set WiFi structure configuration 
    esp_wifi_set_protocol(WIFI_IF_STA, protocol_bitmap); 
    esp_wifi_start();                                   // Start WiFi
    
    /* Waiting until the connection is established (WIFI_CONNECTED_BIT) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdTRUE, pdFALSE, pdMS_TO_TICKS(10000));
    if(bits & WIFI_CONNECTED_BIT){
        wifi_ok = true;
    }
    else if(bits & WIFI_FAIL_BIT){
        wifi_ok = false;
    }
    else{
        wifi_ok = false;
    }

    nvs_close(my_handle);
    free(wifi_pswd);
    free(wifi_ssid);
    return wifi_object;
}


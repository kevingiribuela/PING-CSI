/*************************************************************************
**************************************************************************
*****************************   BIBLIOTECAS   ****************************
**************************************************************************
**************************************************************************/
 /* Bibliotecas FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

/* Bibliotecas para avisos de ESP32 */
#include "esp_system.h"
#include "esp_log.h"
#include "esp_event.h"

/* Bibliotecas para el manejo de memoria no-volatil */
#include "nvs_flash.h"
#include "nvs.h"

#include <sys/param.h>
/* Bibliotecas para el WebServer */
#include <esp_http_server.h>
#include "WebServer.h"

/* Bibliotecas de WiFi*/
#include "WiFi.h"

/* Bibliotecas para los puertos GPIO */ 
#include "driver/gpio.h"

/* Bibliotecas de protocolo de comunicaciones */ 
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/dns.h"

#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "ping/ping_sock.h"

/* Bibliotecas extras */
#include "stdbool.h"
#include "ctype.h"


/************************************************************************
*************************************************************************
******************************   DEFINES   ******************************
*************************************************************************
*************************************************************************/
// Bits de señalización para los eventos WiFi en RTOS
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// Parametros que definen el numero de reintentos y tiempo de espera 
#define MAX_RETRY     20    // Numero de reintentos cada 5 segundos
#define TIME_OUT_WIFI 120   // Segundos en espera antes de conectarse con los datos por default

// Parametros de frecuencia de ping y tiempo de ping
#define CONFIG_SEND_FREQUENCY      100
#define MUESTREO 10
// Comiteando
// Definicion de OFF y ON para el LED
#define OFF 0
#define ON 1

// Definicion plataforma que se utilice ----> Opciones: PROTOBOARD | WIMUMO | PLACA
#define PROTOBOARD 1

#ifdef PROTOBOARD
    #define PULSADOR GPIO_NUM_36
    #define LED      GPIO_NUM_2
#elif WIMUMO
    #define PULSADOR GPIO_NUM_2
    #define LED      GPIO_NUM_12
#elif PLACA
    #define PULSADOR GPIO_NUM_15
    #define LED      GPIO_NUM_13
#endif

/*************************************************************************
**************************************************************************
*************************   VARIABLES GLOBALES   *************************
**************************************************************************
**************************************************************************/

// A continuación, variables globales utilizadas por diferentes bibliotecas para comunicarse 
//con las funciones cuando suceden diferentes eventos
bool wifi_ok = false, acces_point = false;
bool parametters = false, loop = false;

bool guardar_datos=false, config_default=false;
int retry_conn = 0, i=0;
size_t required_size;

nvs_handle_t my_handle;
EventGroupHandle_t s_wifi_event_group;
esp_netif_t *sta_object, *ap_object;


static const char *TAG = "TESINA";

/*************************************************************************
**************************************************************************
******************* PROTOTIPO DE FUNCIONES/MANJEADORES *******************
**************************************************************************
**************************************************************************/

/* void wifi_event_handler
* Se trata del manejador de enventos WiFi, sus parámetros son el evento base, id del evento, y el dato que 
* aporta dicho evento. 
* WIFI_EVENT: Si se trata de eventos WiFi analiza si se ha iniciado en modo STA, si conectó como
*             STA o se desconectó del STA, o si simplemente la conexión falló. 
*               1 - WIFI_EVENT_STA_START: Inicia el protocolo de conexión.
*               2 - WIFI_EVENT_STA_CONNECTED: Indica que se pudo conectar al AP.
*               3 - WIFI_EVENT_STA_DISCONNECTED: Indica que se desconectó del AP, entonces inicia el proceso
*                                               de reconexion a la red hasta un maximo de MAX_RETRY intentos.
*                                               Si no se puede conectar, vuelve a modo AP reseteando el ESP.
*               4 - WIFI_EVENT_AP_START: Se inició el AP.
* IP_EVENT: Al ocurrir un evento con el IP, el único que puede suceder es que se obtuvo un IP para el 
*           dispositivo, de esta forma, se indica con un led verde que estamos conectados a WiFi. */
void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    if(event_base == WIFI_EVENT){
        if (event_id == WIFI_EVENT_STA_START){
            vTaskDelay(100/portTICK_PERIOD_MS);
            printf("CONECTANDO A WIFI...\n");
            esp_err_t wifi = esp_wifi_connect();    // Connecting
            if(wifi!=ESP_OK){
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                printf("NO SE PUDO CONECTAR A WIFI...\n");
            }
            else{
                printf("CONECTADO A WIFI...\n\n");
            }
            
        }
        else if(event_id == WIFI_EVENT_STA_CONNECTED){
            printf("CONEXION AL AP EXITOSA!\n");
        }
        else if(event_id == WIFI_EVENT_STA_DISCONNECTED){
            wifi_ok=false;
            if(retry_conn<MAX_RETRY){
                if(loop==true){
                    esp_wifi_connect(); // Trying to reconnect
                    retry_conn++;
                    printf("Intento de reconexion Nro: %d de %d\n", retry_conn, MAX_RETRY);
                    for(i=0; i<5;i++){
                        vTaskDelay(1000/portTICK_PERIOD_MS);
                    }
                }
            }
            else{
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT); // Flag to reset ESP
                retry_conn=0;
            }
        }
        else if(event_id == WIFI_EVENT_AP_START){
            acces_point=true;
            }
        else if(event_id == WIFI_EVENT_AP_STOP){
            acces_point=false;
        }
    }
    else if(event_base == IP_EVENT){
        if(event_id == IP_EVENT_STA_GOT_IP){
            wifi_ok=true;
            retry_conn=0;
            printf("IP OBTENIDA!\n\n");
            for(int i =0; i<10; i++){
                gpio_set_level(LED, OFF);
                vTaskDelay(50/portTICK_PERIOD_MS);
                gpio_set_level(LED, ON);
                vTaskDelay(50/portTICK_PERIOD_MS);
            }
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        }      
    }
}

static void wifi_csi_rx_cb(void *ctx, wifi_csi_info_t *info)
{
    if (!info || !info->buf || !info->mac) {
        ESP_LOGW(TAG, "<%s> wifi_csi_cb", esp_err_to_name(ESP_ERR_INVALID_ARG));
        return;
    }

    if (memcmp(info->mac, ctx, 6)) {
        return;
    }

    static uint32_t s_count = 0;
    const wifi_pkt_rx_ctrl_t *rx_ctrl = &info->rx_ctrl;

    if((s_count!=0)&&(guardar_datos==false)){
        while(s_count!=0){
            s_count--;
        }
    }

    if(guardar_datos){
        if(s_count==0){
            ets_printf("\n\n\nTipo,Seq,MAC,TimeStamp,RSSI,Noise floor,Rate,MCS,Sig. modeBW,aggregation,STBC,fec_coding,sgi,ampdu_cnt,channel,secondary_channel,sig_len,rx_state,len,first_word,data\n");
        }
        /** Only LLTF sub-carriers are selected. */
        info->len = 128;
        
        printf("CSI_DATA,%d,%d," MACSTR ",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                s_count++, rx_ctrl->timestamp, MAC2STR(info->mac), rx_ctrl->rssi, rx_ctrl->noise_floor, rx_ctrl->rate, 
                rx_ctrl->mcs, rx_ctrl->sig_mode, rx_ctrl->cwb, rx_ctrl->aggregation, rx_ctrl->stbc, rx_ctrl->fec_coding, rx_ctrl->sgi,
                rx_ctrl->ampdu_cnt, rx_ctrl->channel, rx_ctrl->secondary_channel,
                rx_ctrl->sig_len, rx_ctrl->rx_state);

        printf(",%d,%d,\"[%d", info->len, info->first_word_invalid, info->buf[0]);
        for (int i = 1; i < info->len; i++) {
            printf(",%d", info->buf[i]);
        }
        printf("]\"\n");
    }
}

static void wifi_csi_init()
{
    /**
     * @brief In order to ensure the compatibility of routers, only LLTF sub-carriers are selected.
     * referencias tecnicas aca: https://rfmw.em.keysight.com/wireless/helpfiles/n7617a/legacy_long_training_field.htm
     */
    wifi_csi_config_t csi_config = {
        .lltf_en           = true,
        .htltf_en          = false,
        .stbc_htltf2_en    = false,
        .ltf_merge_en      = true,
        .channel_filter_en = true,
        .manu_scale        = true,
        .shift             = true,
    };

    static wifi_ap_record_t s_ap_info = {0};
    ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&s_ap_info));
    ESP_ERROR_CHECK(esp_wifi_set_csi_config(&csi_config));
    ESP_ERROR_CHECK(esp_wifi_set_csi_rx_cb(wifi_csi_rx_cb, s_ap_info.bssid));
    ESP_ERROR_CHECK(esp_wifi_set_csi(true));
}

static esp_err_t wifi_ping_router_start()
{
    static esp_ping_handle_t ping_handle = NULL;
    esp_ping_config_t ping_config        = {
        .count           = 0,
        .interval_ms     = 1000 / CONFIG_SEND_FREQUENCY,
        .timeout_ms      = 1000,
        .data_size       = 1,
        .tos             = 0,
        .task_stack_size = 4096,
        .task_prio       = 0,
    };

    esp_netif_ip_info_t local_ip;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &local_ip);
    ESP_LOGI(TAG, "got ip:" IPSTR ", gw: " IPSTR, IP2STR(&local_ip.ip), IP2STR(&local_ip.gw));
    inet_addr_to_ip4addr(ip_2_ip4(&ping_config.target_addr), (struct in_addr *)&local_ip.gw);

    esp_ping_callbacks_t cbs = { 0 };
    esp_ping_new_session(&ping_config, &cbs, &ping_handle);
    esp_ping_start(ping_handle);

    return ESP_OK;
}

void vPrintData(void *pvParameters){
    while(1){
        // Leo si se presiono el pulsador
        if(gpio_get_level(PULSADOR)&&wifi_ok){
            guardar_datos=true;
            // Genero un retardo de MUESTREO segundos
            for(int i=0; i<MUESTREO; i++){
                vTaskDelay(1000/portTICK_PERIOD_MS);
            }
            // Desactivo el flag 
            guardar_datos=false;
            printf("\n\n\n\n"); // Para poder distinguir dos disparos en una unica sesion
        }
        else{
            vTaskDelay(100/portTICK_PERIOD_MS); // Para que no salte el watchdog
        }
    }
}

void vBlinking(void *pvParameters){
    while(1){
        // Loop para parpadear LED mientras se leen datos del ping al router
        while(guardar_datos){
            gpio_set_level(LED, OFF);
            vTaskDelay(50/portTICK_PERIOD_MS);
            gpio_set_level(LED, ON);
            vTaskDelay(50/portTICK_PERIOD_MS);
        }

        // Enclavo el LED si me conecto. Los 100mS son para que no salte el watchdog
        if(wifi_ok){
            gpio_set_level(LED, ON);
            vTaskDelay(300/portTICK_PERIOD_MS);
        }

        // Destello cada 600mS cuando estoy en modo AP
        else if(acces_point){
            gpio_set_level(LED, ON);
            vTaskDelay(300/portTICK_PERIOD_MS);
            gpio_set_level(LED, OFF);
            vTaskDelay(300/portTICK_PERIOD_MS);
        }

        // Si ninguna condicion se cumple, genero un retardo para que no salte el watchdog
        else{
            vTaskDelay(100/portTICK_PERIOD_MS);
        }
    }
}

void vDefault(void *pvParameters){
    while(1){
        if(gpio_get_level(PULSADOR)&&acces_point){
            config_default=true;
        }
        else{
            vTaskDelay(100/portTICK_PERIOD_MS);
        }
    }
}

void app_main(void)
{
    int time_out=TIME_OUT_WIFI;

    static httpd_handle_t server = NULL;

    s_wifi_event_group = xEventGroupCreate(); // Create event group for wifi events
   
   // Inicializo la memoria no volatil para almacenar datos de red. Si esta llena, la borro y continuo.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    printf("\nNVS INICIALIZADA CORRECTAMENTE.\n");

    // Inicializo protocolo TCP/IP
    ESP_ERROR_CHECK(esp_netif_init());
    printf("\nPROTOCOLO TCP/IP INICIALIZADO CORRECTAMENTE.\n");

    // Creo el event loop por defecto (responde a eventos de WiFi)
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    printf("\nEVENTO LOOP CREADO CORRECTAMENTE.\n");

    // Configuro el GPIO del led
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    gpio_set_level(LED, OFF);

    // Configuro el GPIO del pulsador
    gpio_reset_pin(PULSADOR);
    gpio_set_direction(PULSADOR, GPIO_MODE_INPUT);

    // Seteo los manejadores de WiFi y obtencion de IP
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);

    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);
    
    // Creo la tarea de señalizacion de estados de conexion inalambrica
    xTaskCreate(vBlinking, "Iluminacion", 1024, NULL, 1, NULL);
    xTaskCreate(vPrintData, "Leer datos", 1024, NULL, 1, NULL);
    xTaskCreate(vDefault, "Conectar en default", 1024, NULL, 1, NULL);

    // Inicio de super lazo.
    while(true){
        // Inicio al ESP32 en modo Acces Point y le adjunto los manejadores de evento.
        ap_object = wifi_init_softap();
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
        
        // Inicializo el WebServer y espero por el ingreso de parámetros por AP, luego, lo detengo y desadjunto los manejadores.
        server = start_webserver();
        while(parametters != true){
            vTaskDelay(1000/portTICK_PERIOD_MS);
            time_out--;
            if((time_out==0)||config_default){
                time_out=TIME_OUT_WIFI;                                     // Si se acabo el tiempo de espera, reinicio el contador
                parametters=true;                                           // Activo flag de parametros ingresados (se utilizan los default)
                config_default=false;

                nvs_open("wifi",NVS_READWRITE, &my_handle);                 // Abro la memoria no volatil en modo escritura/lectura
                
                nvs_get_str(my_handle, "SSID", NULL, &required_size);       // Obtengo el espacio en memoria requerido para almacenar el SSID
                char *wifi_ssid = malloc(required_size);                    // Lo asigno dinamicamente
                nvs_get_str(my_handle, "SSID", wifi_ssid, &required_size);  // Lo almaceno en el espacio generado dinamicamente

                nvs_get_str(my_handle, "PSWD", NULL, &required_size);       // Obtengo el espacio en memoria requerido para almacenar el SSID
                char *wifi_pswd = malloc(required_size);                    // Lo asigno dinamicamente
                nvs_get_str(my_handle, "PSWD", wifi_pswd, &required_size);  // Lo almaceno en el espacio generado dinamicamente

                nvs_get_str(my_handle, "PROTOCOLO", NULL, &required_size);  // Get the required size, and value of the SSID from NVS
                char *protocolo = malloc(required_size);
                nvs_get_str(my_handle, "PROTOCOLO", protocolo, &required_size);

                // Muestro por consola los valores utilizados por defecto
                printf("Iniciando conexion default...\n");
                printf("SSID: %s\n", wifi_ssid);
                printf("PSWD: %s\n", wifi_pswd);
                printf("PROTOCOLO: %s\n\n", protocolo);

                // Cierro el manejador de la memoria no volatil y libero memoria dinamica
                nvs_close(my_handle);
                free(wifi_pswd);
                free(wifi_ssid);
            }
        }
        stop_webserver(server);
        
        ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler));
        ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler));
        
        // Inicio al ESP32 en modo Station
        sta_object = wifi_init_sta();
        
        // Si la conexion a WiFi es exitosa, habilito el super loop. Sino, regreso al modo AP
        if(wifi_ok){
            loop=true;
            wifi_csi_init();
            wifi_ping_router_start();
        }
        else{
            loop=false;
            ESP_LOGW(TAG,"No se pudo conectar al AP. Volviendo a modo AP...");
        }

        while(loop){
            // Si algo sale mal con el WiFi, los manejadores externos setean wifi_ok=FALSE.
            if(!wifi_ok){
                EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                                        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                                        pdTRUE,
                                                        pdFALSE,
                                                        portMAX_DELAY); // <<---- portMAX_DELAY bloquea el programa hasta que uno de los parametros cambie
                // Analizo el flag de conexion, si esta en alto, continua el modo de operacion. Sino, se vuelve al modo AP
                if(bits & WIFI_CONNECTED_BIT){
                    wifi_ok=true;      
                }
                else if(bits & WIFI_FAIL_BIT){
                    loop=false;
                }
            }
            else{
                vTaskDelay(100/portTICK_PERIOD_MS);
            }
        } 
        
        // Reseteo flags de parametros ingresado, tiempo de espera, detengo el modulo WiFi, destruyo los manejadores de AP y STA
        // y vuelvo al modo AP.
        parametters=false;
        time_out = TIME_OUT_WIFI;
        esp_wifi_stop();
        esp_netif_destroy_default_wifi(sta_object);
        esp_netif_destroy_default_wifi(ap_object);
    }
}

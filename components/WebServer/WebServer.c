#include "WebServer.h"

// Homepage handler
esp_err_t homepage_get_handler(httpd_req_t *req)
{
    esp_err_t error;
    const char *response=(const char*)req->user_ctx;
    error=httpd_resp_send(req, response,strlen(response));
    if(error != ESP_OK){
        printf("Error imprimiendo la pagina web en modo server, codigo de error: %d \n",error);
    }
    printf("Ingreso a la configuración del WebServer exitosa!\n");
    return ESP_OK;
}
const httpd_uri_t homepage = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = homepage_get_handler,
    
    .user_ctx  = "<!DOCTYPE html>\
<html>\
<body>\
	<form action=\"/data\" method=\"GET\">\
      <h1>ESP32 Wi-Fi configuration </h1>\
        <strong>SSID:</strong> <input type=\"text\" name=\"SSID\"><br>\
        <strong>PSWD:</strong> <input type=\"text\" name=\"PSWD\"><br><br>\
        <input type=\"radio\" id=\"b\" name=\"PROTOCOLO\" value=\"b\">\
        <label for=\"b\">802.11b</label><br>\
        <input type=\"radio\" id=\"bg\" name=\"PROTOCOLO\" value=\"bg\">\
        <label for=\"bg\">802.11bg</label><br>\
        <input type=\"radio\" id=\"bgn\" name=\"PROTOCOLO\" value=\"bgn\">\
        <label for=\"bgn\">802.11bgn</label><br><br>\
        <input type=\"submit\" value=\"Enviar datos\" onclick=\"alert('Datos cargados!')\"><br><br>\
        <label for=\"nro-canal\">CANAL:</label>\
        <select name=\"CANAL\" id=\"nro-canal\">\
            <option value=\"1\">Canal 1</option>\
            <option value=\"2\">Canal 2</option>\
            <option value=\"3\">Canal 3</option>\
            <option value=\"4\">Canal 4</option>\
            <option value=\"5\">Canal 5</option>\
            <option value=\"6\">Canal 6</option>\
            <option value=\"7\">Canal 7</option>\
            <option value=\"8\">Canal 8</option>\
            <option value=\"9\">Canal 9</option>\
            <option value=\"10\">Canal 10</option>\
            <option value=\"11\">Canal 11</option>\
            <option value=\"12\">Canal 11</option>\
            <option value=\"13\">Canal 11</option>\
        </select>\
    </form>\
</body>\
</html>"
};

// Data handler
 esp_err_t data_get_handler(httpd_req_t *req)
{
    char *query, *buff;
    bool ssid=false;
    size_t query_len, buff_len;

    buff_len = 40;
    buff = malloc(++buff_len);
    
    query_len = httpd_req_get_url_query_len(req);
    query = malloc(++query_len);

    httpd_req_get_url_query_str(req, query, query_len);
    query = urlDecode(query);
    /* Preparing the NVS */
    nvs_open("wifi",NVS_READWRITE, &my_handle);

    /* Parametters configuration */
    // SSID:
    httpd_query_key_value(query, "SSID", buff, buff_len);
    if(buff[0]=='\0'){
        printf("SSID: default.\n");
    }
    else{
        nvs_set_str(my_handle,"SSID",buff);
        printf("SSID: %s\n",buff);
        ssid=true;
    }
    // Password:
    httpd_query_key_value(query, "PSWD", buff, buff_len);
    if(buff[0]=='\0'){
        if(ssid){
            nvs_set_str(my_handle,"PSWD",buff);
            printf("PSWD: %s\n",buff);
            ssid = false;
        }
        else{
            printf("PSWD: default.\n");
        }
    }
    else{
        nvs_set_str(my_handle,"PSWD",buff);
        printf("PSWD: %s\n",buff);
    }

    // PROTOCOLO:
    httpd_query_key_value(query, "PROTOCOLO", buff, buff_len);
    if(buff[0]=='\0'){
        printf("PROTOCOLO: default.\n");
    }
    else{
        nvs_set_str(my_handle,"PROTOCOLO",buff);
        printf("PROTOCOLO: 802.11%s\n",buff);
    }

    // CANAL:
    httpd_query_key_value(query, "CANAL", buff, buff_len);
    if(buff[0]=='\0'){
        printf("CANAL: default.\n");
    }
    else{
        nvs_set_str(my_handle,"CANAL",buff);
        printf("CANAL: %s\n",buff);
    }

    nvs_commit(my_handle);
    nvs_close(my_handle);
    free(query);
    free(buff);
    parametters = true;
    return ESP_OK;
}
const httpd_uri_t data = {
    .uri       = "/data",
    .method    = HTTP_GET,
    .handler   = data_get_handler,
    
    .user_ctx  = NULL
};

// Start WebServer function
httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    printf("Starting server on port: '%d' \n", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        printf("Registering URI handlers...\n");
        httpd_register_uri_handler(server, &homepage);
        httpd_register_uri_handler(server, &data);
        printf("Handlers registered! \n");

        return server;
    }

    printf("Error starting server!\n");
    return NULL;
}

// Stop WebServer function
void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    printf("Stoping the WebServer...\n");
    httpd_stop(server);
} 

// Disconnect WebServer handler
void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        printf("Stopping webserver\n");
        stop_webserver(*server);
        *server = NULL;
    }
}

// Connect WebServer handler
void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        printf("Connect handler executed...\n");
        *server = start_webserver();
    }
}

char *urlDecode(const char *str) {
  //int d = 0; /* whether or not the string is decoded */

  char *dStr = (char *) malloc(strlen(str) + 1);
  char eStr[] = "00"; /* for a hex code */

  strcpy(dStr, str);

  //while(!d) {
    //d = 1;
    int i; /* the counter for the string */

    for(i=0;i<strlen(dStr);++i) {

      if(dStr[i] == '%') {
        if(dStr[i+1] == 0)
          return dStr;

        if(isxdigit(dStr[i+1]) && isxdigit(dStr[i+2])) {

          //d = 0;

          /* combine the next to numbers into one */
          eStr[0] = dStr[i+1];
          eStr[1] = dStr[i+2];

          /* convert it to decimal */
          long int x = strtol(eStr, NULL, 16);

          /* remove the hex */
          memmove(&dStr[i+1], &dStr[i+3], strlen(&dStr[i+3])+1);

          dStr[i] = x;
        }
      }
      else if(dStr[i] == '+') { dStr[i] = ' '; }
    }
  //}
  return dStr;
}
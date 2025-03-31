#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "freertos/event_groups.h"

#include "soft-ap.h"
#include "wifi_scan.h"

#include "esp_http_server.h"

extern wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];

static const char* TAG = "http server config";
char ssid_value[64];
char pass_value[64];

void extract_values(char *input, int size, char *ssid, char *ipass) {
    char *token = strtok(input, "&");  // Split at '&'
    int tmp_size = size;
    while (tmp_size > 0) {
        if (strncmp(token, "ssid=", 5) == 0) {
            strcpy(ssid, token + 5);  // Extract ssid value
        } else if (strncmp(token, "ipass=", 6) == 0) {
            strcpy(ipass, token + 6); // Extract ipass value
        }
        token = strtok(NULL, "\0");
    }
}

/* Our URI handler function to be called during GET /uri request */
esp_err_t get_handler(httpd_req_t *req)
{
    /* Send a simple response */
    const char resp_begin[] =
                "<html>\n"
                "<body>\n"
                "<form action=\"/results.html\" target=\"_blank\" method=\"post\">\n"
                "<label for=\"fname\">Networks found:</label>\n"
                "<br>\n"
                "<select name=\"ssid\">\n";

    httpd_resp_send_chunk(req, resp_begin, HTTPD_RESP_USE_STRLEN);

    char option_str[128];
    for(int i = 0; i < DEFAULT_SCAN_LIST_SIZE;i++) {
        snprintf(option_str, sizeof(option_str), "<option value=\"%s\">%s</option>", ap_info[i].ssid, ap_info[i].ssid);
        httpd_resp_send_chunk(req, option_str, HTTPD_RESP_USE_STRLEN);        
    }

    const char resp_end[] =
                "</select>\n"
                "<br>\n"
                "<label for=\"ipass\">Security key:</label><br>\n"
                "<input type=\"password\" name=\"ipass\"><br>\n"
                "<input type=\"submit\" value=\"Submit\">\n"
                "</form>\n"
                "</body>\n"
                "</html>";

    httpd_resp_send_chunk(req, resp_end, HTTPD_RESP_USE_STRLEN);  // TODO: Trimitere sir de caractere ce contine pagina web prezentata in laborator (lista populata cu rezultatele scanarii)
    httpd_resp_send_chunk(req, NULL, 0);  // TODO: Trimitere sir de caractere ce contine pagina web prezentata in laborator (lista populata cu rezultatele scanarii)
    return ESP_OK;
}

/* Our URI handler function to be called during POST /uri request */
esp_err_t post_handler(httpd_req_t *req)
{
    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    char recv_content[256];
    char content[256];
    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(recv_content));
    ESP_LOGI(TAG, "Receveived size: %d", recv_size);

    int ret = httpd_req_recv(req, recv_content, recv_size);
    memcpy(content, recv_content, sizeof(char) * recv_size);

    ESP_LOGI(TAG, "Received: %s", content);
    extract_values(content, ssid_value, pass_value);

    ESP_LOGI(TAG, "Recevied: \nSSID: %s\nPASS: %s\n", ssid_value, pass_value);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Aici #1");
    /* Send a simple response */
    // sprintf(resp, "SSID: %s\n", EXAMPLE_ESP_WIFI_SSID);
    // sprintf(resp, "%s", content);
    httpd_resp_send_chunk(req, ssid_value, HTTPD_RESP_USE_STRLEN);  // TODO: Inlocuire cu un sir de caractere ce contine SSID selectat & parola introdusa
    httpd_resp_send_chunk(req, pass_value, HTTPD_RESP_USE_STRLEN);  // TODO: Inlocuire cu un sir de caractere ce contine SSID selectat & parola introdusa
    
    ESP_LOGI(TAG, "Aici #2");
    return ESP_OK;
}

/* URI handler structure for GET /uri */
httpd_uri_t uri_get = {
    .uri      = "/index.html",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

/* URI handler structure for POST /uri */
httpd_uri_t uri_post = {
    .uri      = "/results.html",
    .method   = HTTP_POST,
    .handler  = post_handler,
    .user_ctx = NULL
};

/* Function for starting the webserver */
httpd_handle_t start_webserver(void)
{
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
    }
    /* If server failed to start, handle will be NULL */
    return server;
}

/* Function for stopping the webserver */
void stop_webserver(httpd_handle_t server)
{
    if (server) {
        /* Stop the httpd server */
        httpd_stop(server);
    }
}
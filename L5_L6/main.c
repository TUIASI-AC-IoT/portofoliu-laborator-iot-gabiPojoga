/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"
#include "esp_http_server.h"


#include "lwip/err.h"
#include "lwip/sys.h"

#include "soft-ap.h"
#include "http-server.h"
#include "mdns_config.h"
#include "wifi_scan.h"

#include "../mdns/include/mdns.h"

static httpd_handle_t http_server_handler;
static nvs_handle_t nvs_handler;
static const char *TAG = "running from main";

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }

    ret = nvs_open("storage", NVS_READWRITE, &nvs_handler);
    if (ret != ESP_OK) {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Done!");

        // Read
        ESP_LOGI(TAG, "Reading restart counter from NVS ... ");
        int16_t restart_counter = 0; // value will default to 0, if not set yet in NVS
        ret = nvs_get_i16(nvs_handler, "restart_counter", &restart_counter);
        switch (ret) {
            case ESP_OK:
                ESP_LOGI(TAG, "Done\n");
                ESP_LOGI(TAG, "Restart counter = %d\n", restart_counter);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGI(TAG, "The value is not initialized yet!\n");
                break;
            default :
                ESP_LOGI(TAG, "Error (%s) reading!\n", esp_err_to_name(ret));
        }

        // Write
        ESP_LOGI(TAG, "Updating restart counter in NVS ... ");
        restart_counter++;
        ret = nvs_set_i16(nvs_handler, "restart_counter", restart_counter);
        if (ret != ESP_OK) 
          ESP_LOGI(TAG, "Failed!\n");
        else
          ESP_LOGI(TAG, "Done!\n");

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        ESP_LOGI(TAG, "Committing updates in NVS ... ");
        ret = nvs_commit(nvs_handler);
        if (ret != ESP_OK) 
          ESP_LOGI(TAG, "Failed!\n");
        else
          ESP_LOGI(TAG, "Done!\n");

        // Close
        nvs_close(nvs_handler);
    }
    // for (int i = 3; i > 0;i--) {
    //   ESP_LOGI(TAG, "Restarting in %d seconds..\n", i);
    //   vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    // ESP_LOGI(TAG, "Restarting now...\n");
    // fflush(stdout);
    // esp_restart(); 

    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // TODO: 3. SSID scanning in STA mode 
    wifi_scan();
    
    // TODO: 1. Start the softAP mode
    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

     // TODO: 4. mDNS init (if there is time left)   
    start_mdns_service();

    // TODO: 2. Start the web server
    http_server_handler = start_webserver();

    // stop_webserver(http_server_handler);

}
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

#include "../mdns/include/mdns.h"

static const char* TAG = "mdns config";

void start_mdns_service()
{
    //initialize mDNS service
    esp_err_t err = mdns_init();
    if (err) {
        printf("MDNS Init failed: %d\n", err);
        return;
    }

    //set hostname
    mdns_hostname_set(MDNS_LOCAL_HOSTNAME);
    //set default instance
    mdns_instance_name_set(MDNS_LOCAL_INSTANCE_NAME);
    mdns_service_add(NULL, "_myservice", "_udp", 1234, NULL, 0);
    mdns_service_add(NULL, "_control_led", "_dp", 10001, NULL, 0);
}

void resolve_mdns_host(const char * host_name)
{
    ESP_LOGI(TAG, "Query A: %s.local", host_name);

    esp_ip4_addr_t addr;
    addr.addr = 0;

    esp_err_t err = mdns_query_a(host_name, 2000,  &addr);
    if(err){
        if(err == ESP_ERR_NOT_FOUND){
            ESP_LOGI(TAG, "Host was not found!");
            return;
        }
        ESP_LOGI(TAG, "Query Failed");
        return;
    }

    ESP_LOGI(TAG, IPSTR, IP2STR(&addr));
}
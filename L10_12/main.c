/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_tls.h"
#include "driver/gptimer.h"
#include "driver/uart.h"


#include "lwip/err.h"
#include "lwip/sys.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"

#define UART_PORT_NUM      UART_NUM_0  // Default serial port for USB
#define UART_BAUD_RATE     115200
#define UART_TX_PIN        UART_PIN_NO_CHANGE
#define UART_RX_PIN        UART_PIN_NO_CHANGE
#define BUF_SIZE           1024


static gptimer_handle_t gptimer;
uint64_t temp_value = 0;
int total_len = 0;
static const char *TAG = "Data receiver";

void uart_init()
{
    const uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    uart_param_config(UART_PORT_NUM, &uart_config);
    uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
}

#define BUFF_SIZE 1024

static void generate_temp_task(void *pvParameter)
{
    uint64_t timer_value;
    esp_err_t status;
    uint64_t variant;

    while(1)
    {
        status = gptimer_get_raw_count(gptimer, &timer_value);
        if (status == ESP_OK) {
            variant = timer_value % 3;
            switch (variant)
            {
            case 0:
                temp_value = 15 + (timer_value % 6);
                break;
            case 1:
                temp_value = 21 + (timer_value % 6);
                break;
            case 2:
                temp_value = 26 + (timer_value % 6);
            default:
                break;
            }
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

static void read_solve_get_request_task(void *pvParameter) 
{
    uint8_t data[BUFF_SIZE];
    while (1) {
        int len = uart_read_bytes(UART_NUM_0, data, sizeof(data) - 1, 100 / portTICK_PERIOD_MS);
        total_len += len;
        if (len > 0) {
            if (data[total_len] == '\0')
            {
                // end of request
                if (strcmp((const char*)data, "get_temp") == 0) {
                    ESP_LOGI(TAG, "Data sent %llu", temp_value);
                    uart_write_bytes(UART_NUM_0, &temp_value, sizeof(temp_value));
                    ESP_LOGI(TAG, "READ REQUEST %s", data);
                }
                total_len = 0;
            }
        }
    }
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1ms
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
    
    uart_init();

    xTaskCreate(generate_temp_task, "generate_temp_task", 4096, NULL, 5, NULL);
    xTaskCreate(read_solve_get_request_task, "read_solve_get_request_task", 4096, NULL, 5, NULL);

}
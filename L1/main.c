#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#define GPIO_OUTPUT_IO 4
#define GPIO_OUTPUT_PIN_SEL (1ULL<<GPIO_OUTPUT_IO)

#define GPIO_INPUT_IO 2
#define GPIO_INPUT_PIN_SEL (1ULL<<GPIO_INPUT_IO)

#define ESP_INTR_FLAG_DEFAULT 0

bool old_state = false;
bool new_state = false;
void ex1() {
    // task for 1000 ms
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gpio_set_level(GPIO_OUTPUT_IO, 1);
    
    // task for 1000 ms
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(GPIO_OUTPUT_IO, 0);
    // task for 1000 ms
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(GPIO_OUTPUT_IO, 1);
    
    // task for 1000 ms
    vTaskDelay(750 / portTICK_PERIOD_MS);
    gpio_set_level(GPIO_OUTPUT_IO, 0);
}

// ex2
void vTaskCode(void *pvParameters){
    int* tmp_cnt = (int*)pvParameters;
    int curr_lvl_input_pin;
    int old_lvl_input_pin;
    // Task code goes here. Low active
    curr_lvl_input_pin = !gpio_get_level(GPIO_INPUT_IO);
    for( ;; )
    {
        // Task code goes here. Low active
        curr_lvl_input_pin = !gpio_get_level(GPIO_INPUT_IO);

        // long press > 750ms
        vTaskDelay(5 / portTICK_PERIOD_MS);
        if (curr_lvl_input_pin != gpio_get_level(GPIO_INPUT_IO)){
            (*tmp_cnt) += 1;
            printf("Counter value is: %d", *tmp_cnt);
        }
        old_lvl_input_pin = !curr_lvl_input_pin;
    }
}
void app_main() {{}
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    static int cnt = 0;
    TaskHandle_t xHandle = NULL;
    xTaskCreate(vTaskCode, "Ex 2", 2048, (void*)&cnt, tskIDLE_PRIORITY, &xHandle);
    configASSERT( xHandle );

    while(1) {
        printf("cnt: %d\n", cnt++);

        ex1();        

    }

    // Use the handle to delete the task.
    if( xHandle != NULL )
    {
        vTaskDelete( xHandle );
    }
}

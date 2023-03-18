#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "stm_flash.h"

static const char *TAG = "MAIN";

void initTask(void)
{
    initFlashUART();
    initGPIO();
    initSPIFFS();
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting flashing procedure...");

    initTask();
    flashSTM("blink.bin");
    endConn();
}


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


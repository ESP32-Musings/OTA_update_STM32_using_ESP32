#include "stm_flash.h"

uint8_t write_buff[32 * 1024];
static const char *TAG = "MAIN";

void flashTask(void)
{
    int page_count = 0;

    char *filepath = "/spiffs/lowled_stm32f072.bin";

    logI(TAG, "%s", "Putting File to Buffer");
    ESP_ERROR_CHECK(fileToBuffer(filepath, write_buff, &page_count));

    logI(TAG, "%s", "Writing STM32 Memory");
    ESP_ERROR_CHECK(writeTask(write_buff, page_count));
    
    logI(TAG, "%s", "Reading STM32 Memory");
    ESP_ERROR_CHECK(readTask(write_buff, page_count));
}

void initTask(void)
{
    initUART();
    initGPIO();
    initSPIFFS();
}

void app_main(void)
{
    initTask();
    flashTask();
    endConn();
}


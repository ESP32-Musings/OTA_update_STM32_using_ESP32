#include <stdint.h>
#include <string.h>

#include "esp_log.h"

#include "stm_flash.h"

#define FILE_PATH_MAX (128)
#define BASE_PATH     "/spiffs/"

static const char *TAG_STM_FLASH = "stm_flash";

esp_err_t writeTask(FILE *flash_file)
{
    ESP_LOGI(TAG_STM_FLASH, "Write Task");

    char loadAddress[4] = {0x08, 0x00, 0x00, 0x00};
    char block[256] = {0};
    int curr_block = 0, bytes_read = 0;

    fseek(flash_file, 0, SEEK_SET);
    setupSTM();

    while ((bytes_read = fread(block, 1, 256, flash_file)) > 0)
    {
        curr_block++;
        ESP_LOGI(TAG_STM_FLASH, "Writing block: %d", curr_block);
        ESP_LOG_BUFFER_HEXDUMP("Block:  ", block, sizeof(block), ESP_LOG_DEBUG);

        esp_err_t ret = flashPage(loadAddress, block);
        if (ret == ESP_FAIL)
        {
            return ESP_FAIL;
        }

        incrementLoadAddress(loadAddress);
        printf("\n");

        memset(block, 0xff, 256);
    }

    return ESP_OK;
}

esp_err_t readTask(FILE *flash_file)
{
    ESP_LOGI(TAG_STM_FLASH, "Read & Verification Task");
    char readAddress[4] = {0x08, 0x00, 0x00, 0x00};

    char block[257] = {0};
    int curr_block = 0, bytes_read = 0;

    fseek(flash_file, 0, SEEK_SET);

    while ((bytes_read = fread(block, 1, 256, flash_file)) > 0)
    {
        curr_block++;
        ESP_LOGI(TAG_STM_FLASH, "Reading block: %d", curr_block);
        ESP_LOG_BUFFER_HEXDUMP("Block:  ", block, sizeof(block), ESP_LOG_DEBUG);

        esp_err_t ret = readPage(readAddress, block);
        if (ret == ESP_FAIL)
        {
            return ESP_FAIL;
        }

        incrementLoadAddress(readAddress);
        printf("\n");

        memset(block, 0xff, 256);
    }

    return ESP_OK;
}

esp_err_t flashSTM(const char *file_name)
{
    esp_err_t err = ESP_FAIL;

    char file_path[FILE_PATH_MAX];
    sprintf(file_path, "%s%s", BASE_PATH, file_name);
    ESP_LOGD(TAG_STM_FLASH, "File name: %s", file_path);

    initGPIO();
    FILE *flash_file = fopen(file_path, "rb");
    if (flash_file != NULL)
    {
        // This while loop executes only once and breaks if any of the functions do not return ESP_OK
        do
        {
            ESP_LOGI(TAG_STM_FLASH, "Writing STM32 Memory");
            IS_ESP_OK(writeTask(flash_file));

            ESP_LOGI(TAG_STM_FLASH, "Reading STM32 Memory");
            IS_ESP_OK(readTask(flash_file));

            err = ESP_OK;
            ESP_LOGI(TAG_STM_FLASH, "STM32 Flashed Successfully!!!");
        } while (0);
    }

    ESP_LOGI(TAG_STM_FLASH, "Ending Connection");
    endConn();

    ESP_LOGI(TAG_STM_FLASH, "Closing file");
    fclose(flash_file);

    return err;
}
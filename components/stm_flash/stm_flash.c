#include "stm_flash.h"

static const char *TAG_STM_FLASH = "stm_flash";

esp_err_t writeTask(FILE *flash_file)
{
    logI(TAG_STM_FLASH, "%s", "Write Task");

    char loadAddress[4] = {0x08, 0x00, 0x00, 0x00};
    char block[256] = {0};
    int curr_block = 0, bytes_read = 0;

    fseek(flash_file, 0, SEEK_SET);
    setupSTM();

    while ((bytes_read = fread(block, 1, 256, flash_file)) > 0)
    {
        curr_block++;
        logI(TAG_STM_FLASH, "Writing block: %d", curr_block);
        // ESP_LOG_BUFFER_HEXDUMP("Block:  ", block, sizeof(block), ESP_LOG_DEBUG);

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
    logI(TAG_STM_FLASH, "%s", "Read & Verification Task");
    char readAddress[4] = {0x08, 0x00, 0x00, 0x00};

    char block[257] = {0};
    int curr_block = 0, bytes_read = 0;

    fseek(flash_file, 0, SEEK_SET);

    while ((bytes_read = fread(block, 1, 256, flash_file)) > 0)
    {
        curr_block++;
        logI(TAG_STM_FLASH, "Reading block: %d", curr_block);
        // ESP_LOG_BUFFER_HEXDUMP("Block:  ", block, sizeof(block), ESP_LOG_DEBUG);

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
    logD(TAG_STM_FLASH, "File name: %s", file_path);

    initGPIO();
    FILE *flash_file = fopen(file_path, "rb");
    if (flash_file != NULL)
    {
        // This while loop executes only once and breaks if any of the functions do not return ESP_OK
        do
        {
            logI(TAG_STM_FLASH, "%s", "Writing STM32 Memory");
            IS_ESP_OK(writeTask(flash_file));

            logI(TAG_STM_FLASH, "%s", "Reading STM32 Memory");
            IS_ESP_OK(readTask(flash_file));

            err = ESP_OK;
            logI(TAG_STM_FLASH, "%s", "STM32 Flashed Successfully!!!");
        } while (0);
    }

    logI(TAG_STM_FLASH, "%s", "Ending Connection");
    endConn();

    logI(TAG_STM_FLASH, "%s", "Closing file");
    fclose(flash_file);

    return err;
}
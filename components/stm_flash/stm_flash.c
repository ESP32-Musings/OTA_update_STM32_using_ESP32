#include "stm_flash.h"

static const char *TAG_STM_FLASH = "stm_flash";

esp_err_t fileToBuffer(char *filepath, uint8_t buff[], int *block_count)
{
    logD(TAG_STM_FLASH, "Reading File: %s", filepath);

    FILE *f = fopen(filepath, "rb");

    if (f == NULL)
    {
        logE(TAG_STM_FLASH, "%s", "Failed to open file for reading");
        return ESP_FAIL;
    }
    else
    {
        logI(TAG_STM_FLASH, "%s", "Found File");
    }

    fseek(f, 0, SEEK_END);
    int file_len = ftell(f);
    rewind(f);

    fread(buff, file_len, 1, f);

    while (file_len % 256 != 0)
    {
        buff[file_len] = 0xff;
        file_len++;
    }

    *block_count = file_len / 256;

    logD(TAG_STM_FLASH, "Final file size %d", file_len);

    fclose(f);

    return ESP_OK;
}

esp_err_t writeTask(uint8_t buff[], int block_count)
{
    int buff_start = 0, buff_index = 0, curr_block = 0;

    char loadAddress[4] = {0x08, 0x00, 0x00, 0x00};
    char block[256];

    setupSTM();

    while (block_count != 0)
    {
        memset(block, '\0', 256);
        int block_index = 0;
        curr_block++;

        logD(TAG_STM_FLASH, "Writing Block %d", block_count);

        while (buff_index < buff_start + 256)
        {
            block[block_index] = buff[buff_index];
            block_index++;
            buff_index++;
        }

        //ESP_LOG_BUFFER_HEXDUMP("Block:  ", block, sizeof(block), ESP_LOG_DEBUG);

        esp_err_t ret = flashPage(loadAddress, block);
        if (ret == ESP_FAIL)
        {
            return ESP_FAIL;
        }

        buff_start += 256;
        block_count--;

        incrementLoadAddress(loadAddress);
        printf("\n");
    }

    return ESP_OK;
}

esp_err_t readTask(uint8_t buff[], int block_count)
{
    char readAddress[4] = {0x08, 0x00, 0x00, 0x00};
    char bytes[] = {0x11, 0xEE};
    char param[] = {0xFF, 0x00};
    int resp = 1, offset = 0;

    while (block_count != 0)
    {
        logI(TAG_STM_FLASH, "%s", "Reading Memory");
        logD(TAG_STM_FLASH, "Reading Block %d", block_count);

        if (sendBytes(bytes, sizeof(bytes), resp))
        {
            if (loadAddress(readAddress[0], readAddress[1], readAddress[2], readAddress[3]))
            {
                sendData(TAG_STM_FLASH, param, sizeof(param));
                int length = waitForSerialData(257, SERIAL_TIMEOUT);
                if (length > 0)
                {
                    uint8_t data[length];
                    const int rxBytes = uart_read_bytes(UART_NUM_1, data, length, 1000 / portTICK_RATE_MS);

                    if (rxBytes > 0 && data[0] == 0x79)
                    {
                        logI(TAG_STM_FLASH, "%s", "Success");
                        if (!compare(buff, data, offset))
                        {
                            return ESP_FAIL;
                        }
                        offset += 256;

                        //ESP_LOG_BUFFER_HEXDUMP("READ MEMORY", data, rxBytes, ESP_LOG_DEBUG);
                    }
                    else
                    {
                        logI(TAG_STM_FLASH, "%s", "Failure");
                        return ESP_FAIL;
                    }
                }
                else
                {
                    logI(TAG_STM_FLASH, "%s", "Serial Timeout");
                    return ESP_FAIL;
                }
            }
        }
        block_count--;
        incrementLoadAddress(readAddress);

        printf("\n");
    }

    return ESP_OK;
}

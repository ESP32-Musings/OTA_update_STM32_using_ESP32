#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#include "esp_err.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"

#include "esp_event.h"
#include "esp_log.h"

#include "sdkconfig.h"
#include "stm_pro_mode.h"

#define HIGH              (1)
#define LOW               (0)
#define ACK               (0x79)

#define TXD_PIN           (CONFIG_ESP_STM_UART_TXD)
#define RXD_PIN           (CONFIG_ESP_STM_UART_RXD)
#define RESET_PIN         (CONFIG_ESP_STM_RESET_PIN)
#define BOOT0_PIN         (CONFIG_ESP_STM_BOOT0_PIN)

#define UART_BAUD_RATE    (CONFIG_ESP_STM_UART_BAUD_RATE)
#define UART_CONTROLLER   (CONFIG_ESP_STM_UART_PORT_NUM)
#define SERIAL_TIMEOUT_MS (CONFIG_ESP_STM_UART_SERIAL_TIMEOUT_MS)
#define UART_BUF_SIZE     (2048)

static const char *TAG_STM_PRO = "stm_pro_mode";

//Functions for custom adjustments
void initFlashUART(void)
{
    const uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(UART_CONTROLLER, UART_BUF_SIZE, 0, 0, NULL, 0);

    uart_param_config(UART_CONTROLLER, &uart_config);
    uart_set_pin(UART_CONTROLLER, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    ESP_LOGI(TAG_STM_PRO, "Initialized Flash UART");
}

void initSPIFFS(void)
{
    ESP_LOGI(TAG_STM_PRO, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG_STM_PRO, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG_STM_PRO, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG_STM_PRO, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total, used;
    if (esp_spiffs_info(NULL, &total, &used) == ESP_OK) {
        ESP_LOGI(TAG_STM_PRO, "Partition size: total: %d, used: %d", total, used);
    }
}

void initGPIO(void)
{
    gpio_set_direction(RESET_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RESET_PIN, HIGH);
    gpio_set_direction(BOOT0_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BOOT0_PIN, HIGH);

    ESP_LOGI(TAG_STM_PRO, "GPIO Initialized");
}

void resetSTM(void)
{
    ESP_LOGI(TAG_STM_PRO, "Starting RESET Procedure");

    gpio_set_level(RESET_PIN, LOW);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG_STM_PRO, "Finished RESET Procedure");
}

void endConn(void)
{
    gpio_set_level(RESET_PIN, LOW);
    gpio_set_level(BOOT0_PIN, LOW);

    resetSTM();

    ESP_LOGI(TAG_STM_PRO, "Ending Connection");
}

void setupSTM(void)
{
    resetSTM();
    cmdSync();
    cmdGet();
    cmdVersion();
    cmdId();
    cmdErase();
    cmdExtErase();
}

int cmdSync(void)
{
    ESP_LOGI(TAG_STM_PRO, "SYNC");

    char bytes[] = {0x7F};
    int resp = 1;
    return sendBytes(bytes, sizeof(bytes), resp);
}

int cmdGet(void)
{
    ESP_LOGI(TAG_STM_PRO, "GET");

    char bytes[] = {0x00, 0xFF};
    int resp = 15;
    return sendBytes(bytes, sizeof(bytes), resp);
}

int cmdVersion(void)
{
    ESP_LOGI(TAG_STM_PRO, "GET VERSION & READ PROTECTION STATUS");

    char bytes[] = {0x01, 0xFE};
    int resp = 5;
    return sendBytes(bytes, sizeof(bytes), resp);
}

int cmdId(void)
{
    ESP_LOGI(TAG_STM_PRO, "CHECK ID");
    char bytes[] = {0x02, 0xFD};
    int resp = 5;
    return sendBytes(bytes, sizeof(bytes), resp);
}

int cmdErase(void)
{
    ESP_LOGI(TAG_STM_PRO, "ERASE MEMORY");
    char bytes[] = {0x43, 0xBC};
    int resp = 1;
    int a = sendBytes(bytes, sizeof(bytes), resp);

    if (a == 1) {
        char params[] = {0xFF, 0x00};
        resp = 1;

        return sendBytes(params, sizeof(params), resp);
    }
    return 0;
}

int cmdExtErase(void)
{
    ESP_LOGI(TAG_STM_PRO, "EXTENDED ERASE MEMORY");
    char bytes[] = {0x44, 0xBB};
    int resp = 1;
    int a = sendBytes(bytes, sizeof(bytes), resp);

    if (a == 1) {
        char params[] = {0xFF, 0xFF, 0x00};
        resp = 1;

        return sendBytes(params, sizeof(params), resp);
    }
    return 0;
}

int cmdWrite(void)
{
    ESP_LOGI(TAG_STM_PRO, "WRITE MEMORY");
    char bytes[2] = {0x31, 0xCE};
    int resp = 1;
    return sendBytes(bytes, sizeof(bytes), resp);
}

int cmdRead(void)
{
    ESP_LOGI(TAG_STM_PRO, "READ MEMORY");
    char bytes[2] = {0x11, 0xEE};
    int resp = 1;
    return sendBytes(bytes, sizeof(bytes), resp);
}

int loadAddress(const char adrMS, const char adrMI, const char adrLI, const char adrLS)
{
    char xor = adrMS ^ adrMI ^ adrLI ^ adrLS;
    char params[] = {adrMS, adrMI, adrLI, adrLS, xor};
    int resp = 1;

    // ESP_LOG_BUFFER_HEXDUMP("LOAD ADDR", params, sizeof(params), ESP_LOG_DEBUG);
    return sendBytes(params, sizeof(params), resp);
}

int sendBytes(const char *bytes, int count, int resp)
{
    sendData(TAG_STM_PRO, bytes, count);
    int length = waitForSerialData(resp, SERIAL_TIMEOUT_MS);

    if (length > 0) {
        uint8_t data[length];
        const int rxBytes = uart_read_bytes(UART_CONTROLLER, data, length, 1000 / portTICK_RATE_MS);

        if (rxBytes > 0 && data[0] == ACK) {
            ESP_LOGI(TAG_STM_PRO, "Sync Success");
            ESP_LOG_BUFFER_HEXDUMP("SYNC", data, rxBytes, ESP_LOG_DEBUG);
            return 1;
        } else {
            ESP_LOGE(TAG_STM_PRO, "Sync Failure");
            return 0;
        }
    } else {
        ESP_LOGE(TAG_STM_PRO, "Serial Timeout");
        return 0;
    }

    return 0;
}

int sendData(const char *logName, const char *data, const int count)
{
    const int txBytes = uart_write_bytes(UART_CONTROLLER, data, count);
    ESP_LOGD(logName, "Wrote %d bytes", count);
    return txBytes;
}

int waitForSerialData(int dataCount, int timeout)
{
    int timer = 0;
    int length = 0;
    while (timer < timeout) {
        uart_get_buffered_data_len(UART_CONTROLLER, (size_t *)&length);
        if (length >= dataCount) {
            return length;
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
        timer++;
    }
    return 0;
}

void incrementLoadAddress(char *loadAddr)
{
    loadAddr[2] += 0x1;

    if (loadAddr[2] == 0) {
        loadAddr[1] += 0x1;

        if (loadAddr[1] == 0) {
            loadAddr[0] += 0x1;
        }
    }
}

esp_err_t flashPage(const char *address, const char *data)
{
    ESP_LOGI(TAG_STM_PRO, "Flashing Page");

    cmdWrite();

    loadAddress(address[0], address[1], address[2], address[3]);

    //ESP_LOG_BUFFER_HEXDUMP("FLASH PAGE", data, 256, ESP_LOG_DEBUG);

    char xor = 0xFF;
    char sz = 0xFF;

    sendData(TAG_STM_PRO, &sz, 1);

    for (int i = 0; i < 256; i++) {
        sendData(TAG_STM_PRO, &data[i], 1);
        xor ^= data[i];
    }

    sendData(TAG_STM_PRO, &xor, 1);

    int length = waitForSerialData(1, SERIAL_TIMEOUT_MS);
    if (length > 0) {
        uint8_t data[length];
        const int rxBytes = uart_read_bytes(UART_CONTROLLER, data, length, 1000 / portTICK_RATE_MS);
        if (rxBytes > 0 && data[0] == ACK) {
            ESP_LOGI(TAG_STM_PRO, "Flash Success");
            return ESP_OK;
        } else {
            ESP_LOGE(TAG_STM_PRO, "Flash Failure");
            return ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG_STM_PRO, "Serial Timeout");
    }
    return ESP_FAIL;
}

esp_err_t readPage(const char *address, const char *data)
{
    ESP_LOGI(TAG_STM_PRO, "Reading page");
    char param[] = {0xFF, 0x00};

    cmdRead();

    loadAddress(address[0], address[1], address[2], address[3]);

    sendData(TAG_STM_PRO, param, sizeof(param));
    int length = waitForSerialData(257, SERIAL_TIMEOUT_MS);
    if (length > 0) {
        uint8_t uart_data[length];
        const int rxBytes = uart_read_bytes(UART_NUM_1, uart_data, length, 1000 / portTICK_RATE_MS);

        if (rxBytes > 0 && uart_data[0] == 0x79) {
            ESP_LOGI(TAG_STM_PRO, "Success");
            if (!memcpy((void *)data, uart_data, 257)) {
                return ESP_FAIL;
            }
            ESP_LOG_BUFFER_HEXDUMP("READ MEMORY", data, rxBytes, ESP_LOG_DEBUG);
        } else {
            ESP_LOGE(TAG_STM_PRO, "Failure");
            return ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG_STM_PRO, "Serial Timeout");
        return ESP_FAIL;
    }

    return ESP_OK;
}
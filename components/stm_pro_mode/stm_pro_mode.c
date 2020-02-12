#include "stm_pro_mode.h"

static const char *TAG_STM_PRO = "stm_pro_mode";

//Functions for custom adjustments
void initUART(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, RTS_PIN, CTS_PIN);
    uart_set_rts(UART_NUM_1, 1);
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);

    logI(TAG_STM_PRO, "%s", "UART Initialized");
}

void initGPIO(void)
{
    gpio_set_direction(RESET_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RESET_PIN, HIGH);
    gpio_set_direction(BOOT0_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BOOT0_PIN, HIGH);

    logI(TAG_STM_PRO, "%s", "GPIO Initialized");
}

void initSPIFFS(void)
{
    logI(TAG_STM_PRO, "%s", "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            logE(TAG_STM_PRO, "%s", "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            logE(TAG_STM_PRO, "%s", "Failed to find SPIFFS partition");
        }
        else
        {
            logE(TAG_STM_PRO, "%s", "Failed to initialize SPIFFS");
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK)
    {
        logE(TAG_STM_PRO, "%s", "Failed to get SPIFFS partition information");
    }
    else
    {
        logD(TAG_STM_PRO, "Partition size: total: %d, used: %d", total, used);
    }
}

void resetSTM(void)
{
    logI(TAG_STM_PRO, "%s", "Starting RESET Procedure");

    gpio_set_level(RESET_PIN, LOW);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    logI(TAG_STM_PRO, "%s", "Finished RESET Procedure");
}

void endConn(void)
{
    gpio_set_level(RESET_PIN, LOW);
    resetSTM();

    logI(TAG_STM_PRO, "%s", "Ending Connection");
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
    logI(TAG_STM_PRO, "%s", "SYNC");

    char bytes[] = {0x7F};
    int resp = 1;
    return sendBytes(bytes, sizeof(bytes), resp);
}

int cmdGet(void)
{
    logI(TAG_STM_PRO, "%s", "GET");

    char bytes[] = {0x00, 0xFF};
    int resp = 15;
    return sendBytes(bytes, sizeof(bytes), resp);
}

int cmdVersion(void)
{
    logI(TAG_STM_PRO, "%s", "GET VERSION & READ PROTECTION STATUS");

    char bytes[] = {0x01, 0xFE};
    int resp = 5;
    return sendBytes(bytes, sizeof(bytes), resp);
}

int cmdId(void)
{
    logI(TAG_STM_PRO, "%s", "CHECK ID");
    char bytes[] = {0x02, 0xFD};
    int resp = 5;
    return sendBytes(bytes, sizeof(bytes), resp);
}

int cmdErase(void)
{
    logI(TAG_STM_PRO, "%s", "ERASE MEMORY");
    char bytes[] = {0x43, 0xBC};
    int resp = 1;
    int a = sendBytes(bytes, sizeof(bytes), resp);

    if (a == 1)
    {
        char params[] = {0xFF, 0x00};
        resp = 1;

        return sendBytes(params, sizeof(params), resp);
    }
    return 0;
}

int cmdExtErase(void)
{
    logI(TAG_STM_PRO, "%s", "EXTENDED ERASE MEMORY");
    char bytes[] = {0x44, 0xBB};
    int resp = 1;
    int a = sendBytes(bytes, sizeof(bytes), resp);

    if (a == 1)
    {
        char params[] = {0xFF, 0xFF, 0x00};
        resp = 1;

        return sendBytes(params, sizeof(params), resp);
    }
    return 0;
}
int cmdWrite(void)
{
    logI(TAG_STM_PRO, "%s", "WRITE MEMORY");
    char bytes[2] = {0x31, 0xCE};
    int resp = 1;
    return sendBytes(bytes, sizeof(bytes), resp);
}

int loadAddress(char adrMS, char adrMI, char adrLI, char adrLS)
{
    char xor = adrMS ^ adrMI ^ adrLI ^ adrLS;
    char params[] = {adrMS, adrMI, adrLI, adrLS, xor};
    int resp = 1;

    //ESP_LOG_BUFFER_HEXDUMP("LOAD ADDR", params, sizeof(params), ESP_LOG_DEBUG);
    return sendBytes(params, sizeof(params), resp);
}

int sendBytes(char *bytes, int count, int resp)
{
    sendData(TAG_STM_PRO, bytes, count);
    int length = waitForSerialData(resp, SERIAL_TIMEOUT);

    if (length > 0)
    {
        uint8_t data[length];
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, length, 1000 / portTICK_RATE_MS);

        if (rxBytes > 0 && data[0] == ACK)
        {
            logI(TAG_STM_PRO, "%s", "Sync Success");
            //ESP_LOG_BUFFER_HEXDUMP("SYNC", data, rxBytes, ESP_LOG_DEBUG);
            return 1;
        }
        else
        {
            logI(TAG_STM_PRO, "%s", "Sync Failure");
            return 0;
        }
    }
    else
    {
        logI(TAG_STM_PRO, "%s", "Serial Timeout");
        return 0;
    }

    return 0;
}

int sendData(const char *logName, const char *data, const int count)
{
    const int txBytes = uart_write_bytes(UART_NUM_1, data, count);
    //log_debug(logName, "Wrote %d bytes", count);
    return txBytes;
}

int waitForSerialData(int dataCount, int timeout)
{
    int timer = 0;
    int length = 0;
    while (timer < timeout)
    {
        uart_get_buffered_data_len(UART_NUM_1, (size_t *)&length);
        if (length >= dataCount)
        {
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

    if (loadAddr[2] == 0)
    {
        loadAddr[1] += 0x1;

        if (loadAddr[1] == 0)
        {
            loadAddr[0] += 0x1;
        }
    }
}

esp_err_t flashPage(char *address, char *data)
{
    logI(TAG_STM_PRO, "%s", "Flashing Page");

    cmdWrite();

    loadAddress(address[0], address[1], address[2], address[3]);

    //ESP_LOG_BUFFER_HEXDUMP("FLASH PAGE", data, 256, ESP_LOG_DEBUG);

    char xor = 0xFF;
    char sz = 0xFF;

    sendData(TAG_STM_PRO, &sz, 1);

    for (int i = 0; i < 256; i++)
    {
        sendData(TAG_STM_PRO, &data[i], 1);
        xor ^= data[i];
    }

    sendData(TAG_STM_PRO, &xor, 1);

    int length = waitForSerialData(1, SERIAL_TIMEOUT);
    if (length > 0)
    {
        uint8_t data[length];
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, length, 1000 / portTICK_RATE_MS);
        if (rxBytes > 0 && data[0] == ACK)
        {
            logI(TAG_STM_PRO, "%s", "Flash Success");
            return ESP_OK;
        }
        else
        {
            logI(TAG_STM_PRO, "%s", "Flash Failure");
            return ESP_FAIL;
        }
    }
    else
    {
        logI(TAG_STM_PRO, "%s", "Serial Timeout");
    }
    return ESP_FAIL;
}

int compare(uint8_t buff[], uint8_t data[], int offset)
{
    int c = 0;
    const int size = 256;
    uint8_t temp[1 + size];

    temp[0] = ACK;

    while (c < size)
    {
        temp[c + 1] = buff[offset + c];
        c++;
    }

    if (!memcmp(temp, data, 1 + size))
    {
        logI(TAG_STM_PRO, "%s", "Verification Success");
        return 1;
    }
    else
    {
        logI(TAG_STM_PRO, "%s", "Verification Failure");
        return 0;
    }
}
#include "logger.h"

static bool logToFile = false;
const char *logLevel[] = {"E", "W", "I", "D", "V"};

bool isLoggingToFileEnabled(void)
{
    return logToFile;
}

bool setLogToFile(void)
{
    logToFile = !logToFile;
    return logToFile;
}

bool doesLogFileExist(const char *log_file_path)
{
    struct stat st;
    return (stat(log_file_path, &st) == 0);
}

void writelogToFile(esp_log_level_t level, const char *log_print_buffer)
{
    char log_file_buffer[LOG_BUFFER_SIZE];
    sprintf(log_file_buffer, "[%s] %s\n", logLevel[level - 1], log_print_buffer);

    FILE *log_file = fopen(LOG_FILE_PATH, "a");
    if (log_file != NULL)
    {
        fputs(log_file_buffer, log_file);
    }
    fclose(log_file);
}

void logger(esp_log_level_t level, const char *TAG, int line, const char *func, const char *fmt, ...)
{
    char log_print_buffer[LOG_BUFFER_SIZE];
    memset(log_print_buffer, '\0', LOG_BUFFER_SIZE);
    sprintf(log_print_buffer, "%s (%s:%d) ", TAG, func, line);

    va_list args;
    va_start(args, fmt);
    int len = strlen(log_print_buffer);
    vsprintf(&log_print_buffer[len], fmt, args);
    va_end(args);

    if (isLoggingToFileEnabled())
    {
        writelogToFile(level, log_print_buffer);
    }

    switch (level)
    {
    case ESP_LOG_ERROR:
        ESP_LOGE("", "%s", log_print_buffer);
        break;
    case ESP_LOG_WARN:
        ESP_LOGW("", "%s", log_print_buffer);
        break;
    case ESP_LOG_INFO:
        ESP_LOGI("", "%s", log_print_buffer);
        break;
    case ESP_LOG_DEBUG:
        ESP_LOGD("", "%s", log_print_buffer);
        break;
    case ESP_LOG_VERBOSE:
        ESP_LOGV("", "%s", log_print_buffer);
        break;
    default:
        break;
    }
}

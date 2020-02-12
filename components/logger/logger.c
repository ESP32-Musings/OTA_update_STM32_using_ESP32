#include "logger.h"

void logger(esp_log_level_t level, const char *TAG, int line, const char *func, const char *fmt, ...)
{
    char log_print_buffer[BUFFER_SIZE];
    memset(log_print_buffer, '\0', BUFFER_SIZE);
    sprintf(log_print_buffer, "%s (%s:%d) ", TAG, func, line);
    va_list args;
    va_start(args, fmt);
    int len = strlen(log_print_buffer);
    vsprintf(&log_print_buffer[len], fmt, args);
    va_end(args);

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

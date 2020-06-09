#ifndef _LOGGER_H
#define _LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/unistd.h>
#include <sys/stat.h>

#include "esp_system.h"
#include "esp_err.h"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

#define LOG_BUFFER_SIZE 512
#define LOG_FILE_PATH "/spiffs/log_data.txt"

#define logV(TAG, fmt, ...) logger(ESP_LOG_VERBOSE, TAG, __LINE__, __func__, fmt, __VA_ARGS__)
#define logD(TAG, fmt, ...) logger(ESP_LOG_DEBUG, TAG, __LINE__, __func__, fmt, __VA_ARGS__)
#define logI(TAG, fmt, ...) logger(ESP_LOG_INFO, TAG, __LINE__, __func__, fmt, __VA_ARGS__)
#define logW(TAG, fmt, ...) logger(ESP_LOG_WARN, TAG, __LINE__, __func__, fmt, __VA_ARGS__)
#define logE(TAG, fmt, ...) logger(ESP_LOG_ERROR, TAG, __LINE__, __func__, fmt, __VA_ARGS__)

bool isLoggingToFileEnabled(void);

bool setLogToFile(void);

bool doesLogFileExist(const char *log_file_path);

void logger(esp_log_level_t level, const char *TAG, int line, const char *func, const char *fmt, ...);

#endif
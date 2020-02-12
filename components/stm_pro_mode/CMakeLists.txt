idf_component_register(SRCS "stm_pro_mode.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_event esp_http_server esp_wifi
                       			freertos logger spiffs nvs_flash)

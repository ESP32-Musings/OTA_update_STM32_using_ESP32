#include <sys/param.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "stm_flash.h"

esp_err_t start_file_server(const char *base_path);

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    esp_netif_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect());

    /* Initialize file storage */
    initSPIFFS();

    /* Start the file server */
    ESP_ERROR_CHECK(start_file_server("/spiffs"));
    
    initFlashUART();
}

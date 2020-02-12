#ifndef _STM_FLASH_H
#define _STM_FLASH_H

#include "stm_pro_mode.h"

/**
 * @brief Write the .bin file data to a local buffer
 *  
 * @param filepath SPIFFS path of the .bin file to be flashed
 * @param buff To store the result
 * @param block_count Total no. of blocks in page (sets of 256 bytes each) 
 * 
 * @return ESP-OK : success, ESP_FAIL : failure
 */
esp_err_t fileToBuffer(char *filepath, uint8_t buff[], int *block_count);

/**
 * @brief Write the code into the flash memory of STM32Fxx
 * 
 * The  local buffer, with the data from the .bin file, 
 * is written into the flash memory of the client, block-by-block 
 * 
 * @param buff buffer with binary data, from the SPIFFS .bin file
 * @param block_count Total no. of blocks in page (sets of 256 bytes each)
 *   
 * @return ESP_OK - success, ESP_FAIL - failed
 */
esp_err_t writeTask(uint8_t buff[], int block_count);

/**
 * @brief Read the flash memory of the STM32Fxx, for verification
 * 
 * It reads the flash memory of the STM32 block-by-block and 
 * checks it with the 'buffer' of data intended to be written
 * 
 * @param buff buffer with binary data, from the SPIFFS .bin file
 * @param block_count Total no. of blocks in page (sets of 256 bytes each)
 *   
 * @return ESP_OK - success, ESP_FAIL - failed
 */
esp_err_t readTask(uint8_t buff[], int block_count);

#endif

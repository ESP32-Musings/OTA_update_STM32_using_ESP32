#ifndef _STM_PRO_MODE_H
#define _STM_PRO_MODE_H

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#include "esp_err.h"
#include "esp_vfs.h"
#include "esp_system.h"
#include "esp_spiffs.h"

#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_http_server.h"

#include "nvs_flash.h"
#include "logger.h"

#define TXD_PIN (GPIO_NUM_4) //(GPIO_NUM_1)
#define RXD_PIN (GPIO_NUM_5) //(GPIO_NUM_3)
#define RTS_PIN (UART_PIN_NO_CHANGE)
#define CTS_PIN (UART_PIN_NO_CHANGE)

#define RESET_PIN (GPIO_NUM_19) //(GPIO_NUM_12)
#define BOOT0_PIN (GPIO_NUM_21) //(GPIO_NUM_2)
#define HIGH 1
#define LOW 0

#define ACK 0x79
#define SERIAL_TIMEOUT 5000

#define PAGE_SIZE_MAX 24 * 1024

static const int RX_BUF_SIZE = 1024;

//Initialize UART functionalities
void initUART(void);

//Initialize GPIO functionalities
void initGPIO(void);

//Initialize SPIFFS functionalities
void initSPIFFS(void);

//Reset the STM32Fxx
void resetSTM(void);

//Increment the memory address for the next write operation
void incrementLoadAddress(char *loadAddr);

//End the connection with STM32Fxx
void endConn(void);

//Compare the STM32Fxx's memory after flashing, with the buffer of binary data for verification purposes
int compare(uint8_t buff[], uint8_t data[], int offset);

//Get in sync with STM32Fxx
int cmdSync(void);

//Get the version and the allowed commands supported by the current version of the bootloader
int cmdGet(void);

//Get the bootloader version and the Read Protection status of the Flash memory
int cmdVersion(void);

//Get the chip ID
int cmdId(void);

//Erase from one to all the Flash memory pages
int cmdErase(void);

//Erases from one to all the Flash memory pages using 2-byte addressing mode
int cmdExtErase(void);

//Setup STM32Fxx for the 'flashing' process
void setupSTM(void);

//Write data to flash memory address
int cmdWrite(void);

//UART send data to STM32Fxx & wait for response
int sendBytes(char *bytes, int count, int resp);

//UART send data byte-by-byte to STM32Fxx
int sendData(const char *logName, const char *data, const int count);

//Wait for response from STM32Fxx
int waitForSerialData(int dataCount, int timeout);

//Send the STM32Fxx the memory address, to be written
int loadAddress(char adrMS, char adrMI, char adrLI, char adrLS);

//UART write the flash memory address of the STM32Fxx with blocks of data 
esp_err_t flashPage(char *address, char *data);

#endif

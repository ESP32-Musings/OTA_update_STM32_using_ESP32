#pragma once

#include <stdio.h>
#include "esp_err.h"

//Macro for error checking
#define IS_ESP_OK(x) if ((x) != ESP_OK) break;

//Initialize UART functionalities
void initFlashUART(void);

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

//Read data from flash memory address
int cmdRead(void);

//UART send data to STM32Fxx & wait for response
int sendBytes(const char *bytes, int count, int resp);

//UART send data byte-by-byte to STM32Fxx
int sendData(const char *logName, const char *data, const int count);

//Wait for response from STM32Fxx
int waitForSerialData(int dataCount, int timeout);

//Send the STM32Fxx the memory address, to be written
int loadAddress(const char adrMS, const char adrMI, const char adrLI, const char adrLS);

//UART write the flash memory address of the STM32Fxx with blocks of data 
esp_err_t flashPage(const char *address, const char *data);

//UART read the flash memory address of the STM32Fxx and verify with the given block of data 
esp_err_t readPage(const char *address, const char *data);

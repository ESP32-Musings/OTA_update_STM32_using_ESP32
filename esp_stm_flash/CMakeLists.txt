# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# (Not part of the boilerplate)

set(EXTRA_COMPONENT_DIRS $ENV{HOME}/projects/OTA_update_STM32_using_ESP32/components)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp_stm_flash)

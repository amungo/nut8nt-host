cmake_minimum_required(VERSION 3.5)

project(nut8_init LANGUAGES C)

project(ad9361_no_os_driver_l)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ad9361_no_os_driver_l ad9361.h ad9361.c ad9361_api.h ad9361_api.c adc_core.h adc_core.c common.h config.h dac_core.h dac_core.c parameters.h platform.c platform.h util.h util.c)

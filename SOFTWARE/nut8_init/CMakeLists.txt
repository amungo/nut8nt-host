cmake_minimum_required(VERSION 3.5)

project(nut8_init LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(${CMAKE_CURRENT_SOURCE_DIR})

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/ad9361_no_os_driver/b)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ad9361_no_os_driver)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ad9361_no_os_driver)

add_executable(nut8_init main.cpp spi_master.cpp spi_master.h nt1065.cpp nt1065.h ads5292.cpp ads5292.h max2871.cpp max2871.h gpio.cpp gpio.h spi_master_ivan.cpp spi_master_ivan.h defines.h axi_dma.cpp axi_dma.h
 ad9361class.cpp ad9361class.h ad9361cfg.h max5717.cpp max5717.h ConfigReader.cpp ConfigReader.h
# ad9361.h ad9361.c ad9361_api.h ad9361_api.c adc_core.h adc_core.c common.h config.h dac_core.h dac_core.c parameters.h platform.c platform.h util.h util.c

 )

target_link_libraries(nut8_init ad9361_no_os_driver_l)

set(CMAKE_INSTALL_PREFIX /home/def)
install(TARGETS nut8_init DESTINATION /home/root/build)

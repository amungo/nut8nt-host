cmake_minimum_required(VERSION 3.5)

project(nut8_init LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(nut8_init main.cpp spi_master.cpp spi_master.h nt1065.cpp nt1065.h ads5292.cpp ads5292.h max2871.cpp max2871.h gpio.cpp gpio.h spi_master_ivan.cpp spi_master_ivan.h defines.h axi_dma.cpp axi_dma.h)
set(CMAKE_INSTALL_PREFIX /home/def)
install(TARGETS nut8_init DESTINATION /home/root/build)

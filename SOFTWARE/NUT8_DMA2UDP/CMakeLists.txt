cmake_minimum_required(VERSION 3.5)

project(NUT8_DMA2UDP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(NUT8_DMA2UDP main.cpp GNU_UDP_client.cpp GNU_UDP_client.h dma-proxy.h gpio.cpp gpio.h)
set (CMAKE_CXX_FLAGS "-Ofast")

find_package (Threads)
target_link_libraries (NUT8_DMA2UDP ${CMAKE_THREAD_LIBS_INIT})
set(CMAKE_INSTALL_PREFIX /home/def)
install(TARGETS NUT8_DMA2UDP DESTINATION /home/root/build)

cmake_minimum_required(VERSION 3.5)

project(NUT8_DMA2TCP_5MHz LANGUAGES C)

find_package (Threads)

add_executable(NUT8_DMA2TCP_5MHz main.c dma-proxy.h)

target_link_libraries (NUT8_DMA2TCP_5MHz ${CMAKE_THREAD_LIBS_INIT})

set(CMAKE_INSTALL_PREFIX /home/root/build)
install(TARGETS NUT8_DMA2TCP_5MHz DESTINATION /home/root/build)


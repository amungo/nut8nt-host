cmake_minimum_required(VERSION 3.5)

project(AXI_DMA_test LANGUAGES C)

find_package (Threads)
add_executable(NUT8_DMA2TCP main.c dma-proxy.h
)

include_directories(xaxidma)
target_link_libraries (NUT8_DMA2TCP ${CMAKE_THREAD_LIBS_INIT})

set (CMAKE_C_FLAGS "-Ofast")

set(CMAKE_INSTALL_PREFIX /home/def)
install(TARGETS NUT8_DMA2TCP DESTINATION /home/root/)

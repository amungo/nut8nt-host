cmake_minimum_required(VERSION 2.8)

project(NUT8_Soft_Compensator_V3)

find_package (Threads)

add_library(AXIDMA STATIC  "AXIDMA/axidma.cpp" "AXIDMA/axidma.h" "AXIDMA/dma-proxy.h" "AXIDMA/dma-proxy2.h" "AXIDMA/config.h")
target_link_libraries (AXIDMA LINK_PUBLIC Control)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(install_directory "/home/root/")

file(GLOB Control_src
     "Control/*.h"
     "Control/*.cpp"
)

add_library(Control STATIC  ${Control_src})

include_directories("AXIDMA/")
include_directories("Control/")

add_executable(NUT8_DMA2UDP "NUT8_DMA2UDP/NUT8_DMA2UDP.cpp" "NUT8_DMA2UDP/GNU_UDP_client.cpp" "NUT8_DMA2UDP/GNU_UDP_client.h")
target_link_libraries (NUT8_DMA2UDP LINK_PUBLIC AXIDMA)
target_link_libraries (NUT8_DMA2UDP ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS NUT8_DMA2UDP DESTINATION ${install_directory})

add_executable(NUT8_DMA2TCP "NUT8_DMA2TCP/NUT8_DMA2TCP.cpp")
target_link_libraries (NUT8_DMA2TCP LINK_PUBLIC AXIDMA)
target_link_libraries (NUT8_DMA2TCP ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS NUT8_DMA2TCP DESTINATION ${install_directory})

add_executable(NUT8_Init "NUT8_Init/main.cpp")
target_link_libraries (NUT8_Init LINK_PUBLIC AXIDMA)
target_link_libraries (NUT8_Init LINK_PUBLIC Control)
install(TARGETS NUT8_Init DESTINATION ${install_directory})

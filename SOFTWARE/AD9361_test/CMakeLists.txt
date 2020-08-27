cmake_minimum_required(VERSION 3.5)

project(AD9361_test LANGUAGES C)

add_executable(AD9361_test main.c ads5292.c ads5292.h gpio_functions.c gpio_functions.h gpio_pins.h max2871.c max2871.h nt1065.c nt1065.h spi_functions.c spi_functions.h
    ad9361_no_os_driver/ad9361.h ad9361_no_os_driver/ad9361.c ad9361_no_os_driver/ad9361_api.h ad9361_no_os_driver/ad9361_api.c ad9361_no_os_driver/adc_core.h ad9361_no_os_driver/adc_core.c
    ad9361_no_os_driver/common.h ad9361_no_os_driver/config.h ad9361_no_os_driver/dac_core.h ad9361_no_os_driver/dac_core.c ad9361_no_os_driver/parameters.h ad9361_no_os_driver/platform.c
    ad9361_no_os_driver/platform.h ad9361_no_os_driver/util.h ad9361_no_os_driver/util.c ad9361_no_os_driver/ad9361_conv.c
    ad9361_wrapper/platform_wrap.c ad9361_wrapper/sleep.h ad9361_wrapper/xil_cache.h ad9361_wrapper/xil_io.c ad9361_wrapper/xil_io.h ad9361_wrapper/xil_printf.h ad9361_wrapper/xparameters.h
    )

include_directories(ad9361_no_os_driver ad9361_wrapper)

set(CMAKE_INSTALL_PREFIX /home/def)
install(TARGETS AD9361_test DESTINATION /home/root/build)

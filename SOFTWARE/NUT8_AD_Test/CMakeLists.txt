cmake_minimum_required(VERSION 3.5)

project(NUT8_AD_Test LANGUAGES C)

add_executable(NUT8_AD_Test main.c
    ad9361.h ad9361.c ad9361_api.h ad9361_api.c adc_core.h adc_core.c common.h config.h dac_core.h dac_core.c parameters.h platform.c platform.h util.h util.c
    )


install(TARGETS NUT8_AD_Test DESTINATION /home/root/build)

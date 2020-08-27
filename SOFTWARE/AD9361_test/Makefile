all:
	mkdir -p bin
	mkdir -p obj
	aarch64-linux-gnu-gcc main.c -c -o obj/main.o -Iad9361_wrapper -Iad9361_no_os_driver -g3
	aarch64-linux-gnu-gcc gpio_functions.c -c -o obj/gpio_functions.o -g3
	aarch64-linux-gnu-gcc spi_functions.c -c -o obj/spi_functions.o -g3
	aarch64-linux-gnu-gcc nt1065.c -c -o obj/nt1065.o -g3
	aarch64-linux-gnu-gcc max2871.c -c -o obj/max2871.o -g3
	aarch64-linux-gnu-gcc ads5292.c -c -o obj/ads5292.o -g3
	aarch64-linux-gnu-gcc ad9361_no_os_driver/ad9361_api.c -c -o obj/ad9361_api.o -Iad9361_wrapper -g3
	aarch64-linux-gnu-gcc ad9361_no_os_driver/ad9361.c -c -o obj/ad9361.o -Iad9361_wrapper -g3
	aarch64-linux-gnu-gcc ad9361_no_os_driver/ad9361_conv.c -c -o obj/ad9361_conv.o -Iad9361_wrapper -g3
	aarch64-linux-gnu-gcc ad9361_no_os_driver/util.c -c -o obj/util.o -Iad9361_wrapper -g3
	aarch64-linux-gnu-gcc ad9361_no_os_driver/adc_core.c -c -o obj/adc_core.o -Iad9361_wrapper -g3
	aarch64-linux-gnu-gcc ad9361_no_os_driver/dac_core.c -c -o obj/dac_core.o -Iad9361_wrapper -g3
	aarch64-linux-gnu-gcc ad9361_no_os_driver/platform.c -c -o obj/platform.o -Iad9361_wrapper -g3
	aarch64-linux-gnu-gcc ad9361_wrapper/xil_io.c -c -o obj/xil_io.o -Iad9361_wrapper -g3
	aarch64-linux-gnu-gcc ad9361_wrapper/platform_wrap.c -c -o obj/platform_wrap.o -Iad9361_wrapper -Iad9361_no_os_driver -I. -g3

	aarch64-linux-gnu-gcc obj/main.o obj/gpio_functions.o obj/spi_functions.o obj/nt1065.o obj/max2871.o obj/ads5292.o obj/ad9361_api.o obj/ad9361.o obj/ad9361_conv.o obj/util.o obj/platform.o obj/adc_core.o obj/dac_core.o obj/xil_io.o obj/platform_wrap.o -o bin/fsp_nut8_set_periph.elf

clean:
	rm ./obj/*.o
	rm ./bin/*.elf
	
distclean:
	rm -rfv ./bin
	rm -rfv ./obj

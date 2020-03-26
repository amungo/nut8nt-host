SUMMARY = "Recipe for  build an external xaxidma Linux kernel module"
SECTION = "PETALINUX/modules"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = "file://Makefile \
           file://xaxidma.c \
           file://xaxidma.h \
           file://xaxidma_bd.c \
           file://xaxidma_bd.h \
           file://xaxidma_bdring.c \
           file://xaxidma_bdring.h \
           file://xaxidma_g.c \
           file://xaxidma_hw.h \
           file://xaxidma_sinit.c \
           file://xbasic_types.c \
           file://xbasic_types.h \
           file://xdebug.h \
           file://xil_io.c \
           file://xil_io.h \
           file://xil_printf.h \
           file://xil_types.h \
           file://xparameters.h \
           file://xpseudo_asm.h \
           file://xpseudo_asm_armclang.h \
           file://xpseudo_asm_gcc.h \
           file://xreg_cortexa53.h \
           file://xstatus.h \
	   file://COPYING \
          "

S = "${WORKDIR}"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.

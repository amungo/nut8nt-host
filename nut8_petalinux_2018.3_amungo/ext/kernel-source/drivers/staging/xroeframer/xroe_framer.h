// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 Xilinx, Inc.
 *
 * Vasileios Bimpikas <vasileios.bimpikas@xilinx.com>
 */
#include "roe_framer_ctrl.h"
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>
#include <uapi/linux/stat.h> /* S_IRUSR, S_IWUSR */

/* TODO: Remove hardcoded addresses once the subsequent, individual devices
 * ("stats" & "radio_ctrl") have been properly implemented in the DT
 */
#define STATS_BASE 0x0000C000
#define STATS_SIZE 0x00000050
#define RADIO_CTRL_BASE 0xA0060000
#define RADIO_CTRL_SIZE 0x0000FFFF
/* TODO: Remove hardcoded value of number of Ethernet ports and read the value
 * from the device tree.
 */
#define MAX_NUM_ETH_PORTS 0x4
/* TODO: to be made static as well, so that multiple instances can be used. As
 * of now, the following 3 structures are shared among the multiple
 * source files
 */
extern struct framer_local *lp;
extern struct kobject *root_xroe_kobj;
extern struct kobject *kobj_framer;
extern struct kobject *kobj_eth_ports[MAX_NUM_ETH_PORTS];
struct framer_local {
	int irq;
	unsigned long mem_start;
	unsigned long mem_end;
	void __iomem *base_addr;
};

struct ioctl_arguments {
	 u32 *offset;
	 u32 *value;
};

int xroe_sysfs_init(void);
int xroe_sysfs_ipv4_init(void);
int xroe_sysfs_ipv6_init(void);
int xroe_sysfs_udp_init(void);
void xroe_sysfs_exit(void);
void xroe_sysfs_ipv4_exit(void);
void xroe_sysfs_ipv6_exit(void);
void xroe_sysfs_udp_exit(void);
int utils_write32withmask(void __iomem *working_address, u32 value,
			  u32 mask, u32 offset);
int utils_check_address_offset(u32 offset, size_t device_size);
void utils_sysfs_store_wrapper(u32 address, u32 offset, u32 mask, u32 value,
			       struct kobject *kobj);
u32 utils_sysfs_show_wrapper(u32 address, u32 offset, u32 mask,
			     struct kobject *kobj);

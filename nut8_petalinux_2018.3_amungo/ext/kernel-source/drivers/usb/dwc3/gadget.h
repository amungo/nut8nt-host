/*
 * gadget.h - DesignWare USB3 DRD Gadget Header
 *
 * Copyright (C) 2010-2011 Texas Instruments Incorporated - http://www.ti.com
 *
 * Authors: Felipe Balbi <balbi@ti.com>,
 *	    Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2  of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __DRIVERS_USB_DWC3_GADGET_H
#define __DRIVERS_USB_DWC3_GADGET_H

#include <linux/list.h>
#include <linux/usb/gadget.h>
#include "io.h"

struct dwc3;
#define to_dwc3_ep(ep)		(container_of(ep, struct dwc3_ep, endpoint))
#define gadget_to_dwc(g)	(container_of(g, struct dwc3, gadget))

/* DEPCFG parameter 1 */
#define DWC3_DEPCFG_INT_NUM(n)		(((n) & 0x1f) << 0)
#define DWC3_DEPCFG_XFER_COMPLETE_EN	BIT(8)
#define DWC3_DEPCFG_XFER_IN_PROGRESS_EN	BIT(9)
#define DWC3_DEPCFG_XFER_NOT_READY_EN	BIT(10)
#define DWC3_DEPCFG_FIFO_ERROR_EN	BIT(11)
#define DWC3_DEPCFG_STREAM_EVENT_EN	BIT(12)
#define DWC3_DEPCFG_BINTERVAL_M1(n)	(((n) & 0xff) << 16)
#define DWC3_DEPCFG_STREAM_CAPABLE	BIT(24)
#define DWC3_DEPCFG_EP_NUMBER(n)	(((n) & 0x1f) << 25)
#define DWC3_DEPCFG_BULK_BASED		BIT(30)
#define DWC3_DEPCFG_FIFO_BASED		BIT(31)

/* DEPCFG parameter 0 */
#define DWC3_DEPCFG_EP_TYPE(n)		(((n) & 0x3) << 1)
#define DWC3_DEPCFG_MAX_PACKET_SIZE(n)	(((n) & 0x7ff) << 3)
#define DWC3_DEPCFG_FIFO_NUMBER(n)	(((n) & 0x1f) << 17)
#define DWC3_DEPCFG_BURST_SIZE(n)	(((n) & 0xf) << 22)
#define DWC3_DEPCFG_DATA_SEQ_NUM(n)	((n) << 26)
/* This applies for core versions earlier than 1.94a */
#define DWC3_DEPCFG_IGN_SEQ_NUM		BIT(31)
/* These apply for core versions 1.94a and later */
#define DWC3_DEPCFG_ACTION_INIT		(0 << 30)
#define DWC3_DEPCFG_ACTION_RESTORE	BIT(30)
#define DWC3_DEPCFG_ACTION_MODIFY	(2 << 30)

/* DEPXFERCFG parameter 0 */
#define DWC3_DEPXFERCFG_NUM_XFER_RES(n)	((n) & 0xffff)

/* Below used in hibernation */
#define DWC3_NON_STICKY_RESTORE_RETRIES	500
#define DWC3_NON_STICKY_SAVE_RETRIES	500
#define DWC3_DEVICE_CTRL_READY_RETRIES	20000
#define DWC3_NON_STICKY_RESTORE_DELAY	100
#define DWC3_NON_STICKY_SAVE_DELAY	100
#define DWC3_DEVICE_CTRL_READY_DELAY	5

/* -------------------------------------------------------------------------- */

#define to_dwc3_request(r)	(container_of(r, struct dwc3_request, request))

/**
 * next_request - gets the next request on the given list
 * @list: the request list to operate on
 *
 * Caller should take care of locking. This function return %NULL or the first
 * request available on @list.
 */
static inline struct dwc3_request *next_request(struct list_head *list)
{
	return list_first_entry_or_null(list, struct dwc3_request, list);
}

/**
 * dwc3_gadget_move_started_request - move @req to the started_list
 * @req: the request to be moved
 *
 * Caller should take care of locking. This function will move @req from its
 * current list to the endpoint's started_list.
 */
static inline void dwc3_gadget_move_started_request(struct dwc3_request *req)
{
	struct dwc3_ep		*dep = req->dep;

	req->started = true;
	list_move_tail(&req->list, &dep->started_list);
}

void dwc3_gadget_giveback(struct dwc3_ep *dep, struct dwc3_request *req,
		int status);

void dwc3_ep0_interrupt(struct dwc3 *dwc,
		const struct dwc3_event_depevt *event);
void dwc3_ep0_out_start(struct dwc3 *dwc);
void dwc3_gadget_enable_irq(struct dwc3 *dwc);
void dwc3_gadget_disable_irq(struct dwc3 *dwc);
int __dwc3_gadget_ep0_set_halt(struct usb_ep *ep, int value);
int dwc3_gadget_ep0_set_halt(struct usb_ep *ep, int value);
int dwc3_gadget_ep0_queue(struct usb_ep *ep, struct usb_request *request,
		gfp_t gfp_flags);
int __dwc3_gadget_ep_set_halt(struct dwc3_ep *dep, int value, int protocol);
int __dwc3_gadget_ep_enable(struct dwc3_ep *dep, bool modify, bool restore);
int __dwc3_gadget_ep_disable(struct dwc3_ep *dep);
int __dwc3_gadget_kick_transfer(struct dwc3_ep *dep, u16 cmd_param);
void dwc3_stop_active_transfer(struct dwc3 *dwc, u32 epnum, bool force);
int dwc3_gadget_run_stop(struct dwc3 *dwc, int is_on, int suspend);
dma_addr_t dwc3_trb_dma_offset(struct dwc3_ep *dep, struct dwc3_trb *trb);
void gadget_hibernation_interrupt(struct dwc3 *dwc);
void gadget_wakeup_interrupt(struct dwc3 *dwc);

/**
 * dwc3_gadget_ep_get_transfer_index - Gets transfer index from HW
 * @dep: dwc3 endpoint
 *
 * Caller should take care of locking. Returns the transfer resource
 * index for a given endpoint.
 */
static inline u32 dwc3_gadget_ep_get_transfer_index(struct dwc3_ep *dep)
{
	u32			res_id;

	res_id = dwc3_readl(dep->regs, DWC3_DEPCMD);

	return DWC3_DEPCMD_GET_RSC_IDX(res_id);
}

#endif /* __DRIVERS_USB_DWC3_GADGET_H */

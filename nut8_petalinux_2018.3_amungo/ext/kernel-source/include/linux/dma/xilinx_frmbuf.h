/*
 * Xilinx Framebuffer DMA support header file
 *
 * Copyright (C) 2017 Xilinx, Inc. All rights reserved.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __XILINX_FRMBUF_DMA_H
#define __XILINX_FRMBUF_DMA_H

#include <linux/dmaengine.h>

/**
 * enum vid_frmwork_type - Linux video framework type
 * @XDMA_DRM: fourcc is of type DRM
 * @XDMA_V4L2: fourcc is of type V4L2
 */
enum vid_frmwork_type {
	XDMA_DRM = 0,
	XDMA_V4L2,
};

/**
 * enum operation_mode - FB IP control register field settings to select mode
 * @DEFAULT : Use default mode, No explicit bit field settings required.
 * @AUTO_RESTART : Use auto-restart mode by setting BIT(7) of control register.
 */
enum operation_mode {
	DEFAULT = 0x0,
	AUTO_RESTART = BIT(7),
};

#if IS_ENABLED(CONFIG_XILINX_FRMBUF)
/**
 * xilinx_xdma_set_mode - Set operation mode for framebuffer IP
 * @chan: dma channel instance
 * @mode: Famebuffer IP operation mode.
 * This routine is used when utilizing "video format aware" Xilinx DMA IP
 * (such as Video Framebuffer Read or Video Framebuffer Write).  This call
 * must be made prior to dma_async_issue_pending(). This routine should be
 * called by client driver to set the operation mode for framebuffer IP based
 * upon the use-case, for e.g. for non-streaming usecases (like MEM2MEM) it's
 * more appropriate to use default mode unlike streaming usecases where
 * auto-restart mode is more suitable.
 *
 * auto-restart or free running mode.
 */
void xilinx_xdma_set_mode(struct dma_chan *chan, enum operation_mode mode);

/**
 * xilinx_xdma_drm_config - configure video format in video aware DMA
 * @chan: dma channel instance
 * @drm_fourcc: DRM fourcc code describing the memory layout of video data
 *
 * This routine is used when utilizing "video format aware" Xilinx DMA IP
 * (such as Video Framebuffer Read or Video Framebuffer Write).  This call
 * must be made prior to dma_async_issue_pending() to establish the video
 * data memory format within the hardware DMA.
 */
void xilinx_xdma_drm_config(struct dma_chan *chan, u32 drm_fourcc);

/**
 * xilinx_xdma_v4l2_config - configure video format in video aware DMA
 * @chan: dma channel instance
 * @v4l2_fourcc: V4L2 fourcc code describing the memory layout of video data
 *
 * This routine is used when utilizing "video format aware" Xilinx DMA IP
 * (such as Video Framebuffer Read or Video Framebuffer Write).  This call
 * must be made prior to dma_async_issue_pending() to establish the video
 * data memory format within the hardware DMA.
 */
void xilinx_xdma_v4l2_config(struct dma_chan *chan, u32 v4l2_fourcc);

/**
 * xilinx_xdma_get_drm_vid_fmts - obtain list of supported DRM mem formats
 * @chan: dma channel instance
 * @fmt_cnt: Output param - total count of supported DRM fourcc codes
 * @fmts: Output param - pointer to array of DRM fourcc codes (not a copy)
 *
 * Return: a reference to an array of DRM fourcc codes supported by this
 * instance of the Video Framebuffer Driver
 */
int xilinx_xdma_get_drm_vid_fmts(struct dma_chan *chan, u32 *fmt_cnt,
				 u32 **fmts);

/**
 * xilinx_xdma_get_v4l2_vid_fmts - obtain list of supported V4L2 mem formats
 * @chan: dma channel instance
 * @fmt_cnt: Output param - total count of supported V4L2 fourcc codes
 * @fmts: Output param - pointer to array of V4L2 fourcc codes (not a copy)
 *
 * Return: a reference to an array of V4L2 fourcc codes supported by this
 * instance of the Video Framebuffer Driver
 */
int xilinx_xdma_get_v4l2_vid_fmts(struct dma_chan *chan, u32 *fmt_cnt,
				  u32 **fmts);

/**
 * xilinx_xdma_get_fid - Get the Field ID of the buffer received.
 * This function should be called from the callback function registered
 * per descriptor in prep_interleaved.
 *
 * @chan: dma channel instance
 * @async_tx: descriptor whose parent structure contains fid.
 * @fid: Output param - Field ID of the buffer. 0 - even, 1 - odd.
 *
 * Return: 0 on success, -EINVAL in case of invalid chan
 */
int xilinx_xdma_get_fid(struct dma_chan *chan,
			struct dma_async_tx_descriptor *async_tx, u32 *fid);

/**
 * xilinx_xdma_set_fid - Set the Field ID of the buffer to be transmitted
 * @chan: dma channel instance
 * @async_tx: dma async tx descriptor for the buffer
 * @fid: Field ID of the buffer. 0 - even, 1 - odd.
 *
 * Return: 0 on success, -EINVAL in case of invalid chan
 */
int xilinx_xdma_set_fid(struct dma_chan *chan,
			struct dma_async_tx_descriptor *async_tx, u32 fid);

/**
 * xilinx_xdma_get_earlycb - Get info if early callback has been enabled.
 *
 * @chan: dma channel instance
 * @async_tx: descriptor whose parent structure contains fid.
 * @enable: Output param - Early callback enabled
 *
 * Return: 0 on success, -EINVAL in case of invalid chan
 */
int xilinx_xdma_get_earlycb(struct dma_chan *chan,
			    struct dma_async_tx_descriptor *async_tx,
			    bool *enable);

/**
 * xilinx_xdma_set_earlycb - Enable/Disable early callback
 * @chan: dma channel instance
 * @async_tx: dma async tx descriptor for the buffer
 * @enable: Flag to enable or disable early callback for descriptor.
 *
 * Return: 0 on success, -EINVAL in case of invalid chan
 */
int xilinx_xdma_set_earlycb(struct dma_chan *chan,
			    struct dma_async_tx_descriptor *async_tx,
			    bool enable);
#else
static inline void xilinx_xdma_drm_config(struct dma_chan *chan, u32 drm_fourcc)
{ }

static inline void xilinx_xdma_v4l2_config(struct dma_chan *chan,
					   u32 v4l2_fourcc)
{ }

static int xilinx_xdma_get_drm_vid_fmts(struct dma_chan *chan, u32 *fmt_cnt,
					u32 **fmts)
{
	return -ENODEV;
}

static int xilinx_xdma_get_v4l2_vid_fmts(struct dma_chan *chan, u32 *fmt_cnt,
					 u32 **fmts)
{
	return -ENODEV;
}

static inline int xilinx_xdma_get_fid(struct dma_chan *chan,
				      struct dma_async_tx_descriptor *async_tx,
				      u32 *fid)
{
	return -ENODEV;
}

static inline int xilinx_xdma_set_fid(struct dma_chan *chan,
				      struct dma_async_tx_descriptor *async_tx,
				      u32 fid)
{
	return -ENODEV;
}

static inline int xilinx_xdma_get_earlycb(struct dma_chan *chan,
					  struct dma_async_tx_descriptor *atx,
					  bool *enable)
{
	return -ENODEV;
}

static inline int xilinx_xdma_set_earlycb(struct dma_chan *chan,
					  struct dma_async_tx_descriptor *atx,
					  bool enable)
{
	return -ENODEV;
}
#endif

#endif /*__XILINX_FRMBUF_DMA_H*/

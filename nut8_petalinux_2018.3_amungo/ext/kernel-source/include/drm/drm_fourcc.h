/*
 * Copyright (c) 2016 Laurent Pinchart <laurent.pinchart@ideasonboard.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */
#ifndef __DRM_FOURCC_H__
#define __DRM_FOURCC_H__

#include <linux/types.h>
#include <uapi/drm/drm_fourcc.h>

struct drm_device;
struct drm_mode_fb_cmd2;

/**
 * struct drm_format_info - information about a DRM format
 */
struct drm_format_info {
	/**
	 * @format:
	 *
	 * 4CC format identifier (DRM_FORMAT_*)
	 */
	u32 format;

	/**
	 * @depth:
	 *
	 * Color depth (number of bits per pixel excluding padding bits),
	 * valid for a subset of RGB formats only. This is a legacy field,
	 * do not use in new code and set to 0 for new formats.
	 */
	u8 depth;

	/**
	 * @num_planes:
	 *
	 * Number of color planes (1 to 3)
	 */
	u8 num_planes;

	/**
	 * @cpp:
	 *
	 * Number of bytes per pixel (per plane). @cpp shouldn't be used when
	 * @pixels_per_macropixel and @bytes_per_macropixel are used.
	 */
	u8 cpp[3];

	/**
	 * @pixels_per_macropixel:
	 *
	 * Number of pixels per macro-pixel (per plane). A macro-pixel is
	 * composed of multiple pixels, and there can be extra bits between
	 * pixels. This must be used along with @bytes_per_macropixel, only
	 * when single pixel size is not byte-aligned. In this case, @cpp
	 * is not valid and should be 0.
	 */
	u8 pixels_per_macropixel[3];

	/**
	 * @bytes_per_macropixel:
	 *
	 * Number of bytes per macro-pixel (per plane). A macro-pixel is
	 * composed of multiple pixels. The size of single macro-pixel should
	 * be byte-aligned. This should be used with @pixels_per_macropixel,
	 * and @cpp should be 0.
	 */
	u8 bytes_per_macropixel[3];

	/**
	 * @hsub:
	 *
	 * Horizontal chroma subsampling factor
	 */
	u8 hsub;

	/**
	 * @vsub:
	 *
	 * Vertical chroma subsampling factor
	 */
	u8 vsub;
	/**
	 * @has_alpha:
	 *
	 * Does the format embed an alpha component?
	 */
	bool has_alpha;
};

/**
 * struct drm_format_name_buf - name of a DRM format
 * @str: string buffer containing the format name
 */
struct drm_format_name_buf {
	char str[32];
};

const struct drm_format_info *__drm_format_info(u32 format);
const struct drm_format_info *drm_format_info(u32 format);
const struct drm_format_info *
drm_get_format_info(struct drm_device *dev,
		    const struct drm_mode_fb_cmd2 *mode_cmd);
uint32_t drm_mode_legacy_fb_format(uint32_t bpp, uint32_t depth);
int drm_format_num_planes(uint32_t format);
int drm_format_plane_cpp(uint32_t format, int plane);
int drm_format_horz_chroma_subsampling(uint32_t format);
int drm_format_vert_chroma_subsampling(uint32_t format);
int drm_format_plane_width(int width, uint32_t format, int plane);
int drm_format_plane_height(int height, uint32_t format, int plane);
int drm_format_plane_width_bytes(const struct drm_format_info *info,
				 int plane, int width);
const char *drm_get_format_name(uint32_t format, struct drm_format_name_buf *buf);

#endif /* __DRM_FOURCC_H__ */

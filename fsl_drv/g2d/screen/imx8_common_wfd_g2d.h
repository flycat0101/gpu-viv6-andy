/*
 * $QNXLicenseC:
 * Copyright 2016-2018, QNX Software Systems.
 * Copyright 2016, Freescale Semiconductor, Inc.
 * Copyright 2017-2019 NXP
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#ifndef __IMX8_COMMON_WFD_G2D_INCLUDED
#define __IMX8_COMMON_WFD_G2D_INCLUDED

#define GET_TILING_MODE(format) ((format) & WFD_FORMAT_IMX8X_TILING_MODE_MASK)

/* Alignment requirements (in pixels) for tiling */
typedef struct _tiling_align {
	int width;
	int height;
} tiling_align;

static inline void
get_tiling_alignment_requirement(enum wfd_imx8_tiling_mode tiling_mode,
		tiling_align *align) {
	assert(align);

	switch (tiling_mode) {
	case WFD_FORMAT_IMX8X_TILING_MODE_LINEAR:
		align->width = 1;
		align->height = 1;
		return;
	case WFD_FORMAT_IMX8X_TILING_MODE_VIVANTE_TILED:
		align->width = 4;
		align->height = 4;
		return;
	case WFD_FORMAT_IMX8X_TILING_MODE_VIVANTE_SUPER_TILED:
		align->width = 64;
		align->height = 64;
		return;
	case WFD_FORMAT_IMX8X_TILING_MODE_AMPHION_TILED:
		align->width = 8;
		align->height = 256;
		return;
	case WFD_FORMAT_IMX8X_TILING_MODE_AMPHION_INTERLACED:
		/* TODO - add the correct tiling requirement when
		 * info available */
		align->width = 1;
		align->height = 1;
		return;
	default:
		align->width = -1;
		align->height = -1;
		return;
	}
}

static inline bool
is_valid_tiling_mode(enum wfd_imx8_tiling_mode tiling_mode) {
	tiling_align align;

	get_tiling_alignment_requirement(tiling_mode, &align);

	return (align.width != -1 && align.height != -1);
}

static inline bool
is_tiling_mode_linear(enum wfd_imx8_tiling_mode tiling_mode) {
	return tiling_mode == WFD_FORMAT_IMX8X_TILING_MODE_LINEAR;
}

#endif /* __IMX8_COMMON_WFD_G2D_INCLUDED */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif

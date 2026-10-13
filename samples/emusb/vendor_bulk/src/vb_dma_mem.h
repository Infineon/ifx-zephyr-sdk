/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 Infineon Technologies AG,
 * SPDX-FileCopyrightText: or an affiliate of Infineon Technologies AG. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SAMPLES_EMUSB_VENDOR_BULK_VB_DMA_MEM_H_
#define SAMPLES_EMUSB_VENDOR_BULK_VB_DMA_MEM_H_

#include <zephyr/kernel.h>

/*
 * Attribute for buffers the USB controller DMAs to/from. On M55 (D-cache
 * enabled) they must be non-cacheable to stay coherent with the DMA engine; on
 * M33 (no data cache) nocache is unavailable, so only the cache-line alignment
 * applies.
 */
#if defined(CONFIG_NOCACHE_MEMORY)
#define VB_DMA_MEM __nocache __aligned(32)
#else
#define VB_DMA_MEM __aligned(32)
#endif

#endif /* SAMPLES_EMUSB_VENDOR_BULK_VB_DMA_MEM_H_ */

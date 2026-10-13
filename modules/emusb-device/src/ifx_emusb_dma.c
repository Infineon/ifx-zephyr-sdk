/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 Infineon Technologies AG,
 * SPDX-FileCopyrightText: or an affiliate of Infineon Technologies AG. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * emUSB-Device DMA driver glue for the Infineon PSE84 USBHS (Zephyr).
 *
 * Cache-maintenance hooks and DMA-address validation for the DMA variant of
 * the pre-compiled PSE84 driver (USB_Driver_Infineon_PSOCE84_DMA). The
 * application selects the DMA driver and then calls ifx_emusb_dma_configure()
 * from USBD_X_Config().
 *
 * Cortex-M33 has no data cache, so the clean/invalidate hooks are no-ops.
 * Cortex-M55 runs with the D-cache enabled, so they perform real SCB cache
 * maintenance; on M55 the DMA buffers must additionally live in non-cacheable
 * (or SOCMEM) memory.
 */

#include <zephyr/kernel.h>
#include <cmsis_core.h>

#include "USB.h"
#include "ifx_emusb_usbhs.h"

/* DMA-reachable SRAM window on PSE84 (device-specific; matches the
 * ModusToolbox reference usbd_config.c). Buffers outside this range are
 * bounce-copied by the stack instead of DMA'd directly. */
#define IFX_DMA_ADDR_START 0x24000000u
#define IFX_DMA_ADDR_END   0x40000000u

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1)

static void ifx_dcache_clean(void *p, unsigned long num_bytes)
{
	SCB_CleanDCache_by_Addr((uint32_t *)p, (int32_t)num_bytes);
}

static void ifx_dcache_invalidate(void *p, unsigned long num_bytes)
{
	SCB_InvalidateDCache_by_Addr((void *)p, (int32_t)num_bytes);
}

#else /* no data cache (Cortex-M33) */

static void ifx_dcache_clean(void *p, unsigned long num_bytes)
{
	ARG_UNUSED(p);
	ARG_UNUSED(num_bytes);
}

static void ifx_dcache_invalidate(void *p, unsigned long num_bytes)
{
	ARG_UNUSED(p);
	ARG_UNUSED(num_bytes);
}

#endif

static const SEGGER_CACHE_CONFIG ifx_cache_config = {
	32,   /* CacheLineSize */
	NULL, /* pfDMB (not required) */
	ifx_dcache_clean,
	ifx_dcache_invalidate,
};

/* Returns 0 if the buffer is DMA-reachable, non-zero otherwise. */
static int ifx_dma_addr_valid(const void *mem)
{
	uint32_t addr = (uint32_t)mem;

	return (addr >= IFX_DMA_ADDR_START && addr < IFX_DMA_ADDR_END) ? 0 : 1;
}

void ifx_emusb_dma_configure(void)
{
	USBD_SetCacheConfig(&ifx_cache_config, sizeof(ifx_cache_config));
	USBD_SetCheckAddress(ifx_dma_addr_valid);
}

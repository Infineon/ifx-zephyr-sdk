/*
 * Application platform configuration for the pre-compiled emUSB-Device library
 * on Zephyr / Infineon PSE84.
 *
 * Copyright (c) 2026 Infineon Technologies AG,
 * or an affiliate of Infineon Technologies AG.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Provides the emUSB USBD_X_Config() hook: bring up the USBHS PHY, select the
 * PSE84 hardware driver (the DMA variant when CONFIG_EMUSB_DEVICE_DMA is set,
 * otherwise the slave/FIFO variant), assign the endpoint memory pool, and
 * register the ISR-enable callback. Also supplies the emUSB log/panic hooks.
 * The USBHS clock/PHY bring-up and IRQ wiring live in the SDK emUSB
 * integration (ifx_emusb_usbhs.h).
 */

#include <zephyr/kernel.h>

#include "USB.h"
#include "USB_HW_Infineon_PSOCE84.h"
#include "ifx_emusb_usbhs.h"

#include "vb_dma_mem.h"

/* Memory pool the stack uses for endpoint buffers and transfer descriptors.
 * It is a DMA target for the USB controller, so it lives in VB_DMA_MEM
 * (non-cacheable on M55). */
#define EP_BUFFER_SIZE (4096u)
static uint32_t memory_pool[EP_BUFFER_SIZE / 4u] VB_DMA_MEM;

void USBD_X_Config(void)
{
	ifx_emusb_usbhs_phy_enable();

#if defined(CONFIG_EMUSB_DEVICE_DMA)
	USBD_AddDriver(&USB_Driver_Infineon_PSOCE84_DMA);
	ifx_emusb_dma_configure();
#else
	USBD_AddDriver(&USB_Driver_Infineon_PSOCE84);
#endif
	USBD_AssignMemory(memory_pool, sizeof(memory_pool));
	USBD_SetISREnableFunc(ifx_emusb_enable_isr);
}

void USB_OS_Panic(const char *pErrMsg)
{
	printk("emUSB PANIC: %s\n", (pErrMsg != NULL) ? pErrMsg : "");
	k_panic();

	for (;;) {
		/* unreachable */
	}
}

void USB_X_Log(const char *s)
{
	ARG_UNUSED(s);
}

void USB_X_Warn(const char *s)
{
	printk("emUSB: %s\n", (s != NULL) ? s : "");
}

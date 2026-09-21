/*
 * emUSB-Device platform glue for the Infineon PSE84 USBHS controller (Zephyr).
 *
 * Copyright (c) 2026 Infineon Technologies AG,
 * or an affiliate of Infineon Technologies AG.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IFX_EMUSB_USBHS_H
#define IFX_EMUSB_USBHS_H

#include "USB.h" /* USB_ISR_HANDLER */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Bring up the Infineon USBHS subsystem (SS) wrapper: enable and divide the
 * peripheral-group clock and enable the SS/PHY in device mode, so the
 * pre-compiled emUSB driver can access the DWC2 core registers. Call once,
 * before USBD_Init() reaches the hardware (e.g. from USBD_X_Config()).
 */
void ifx_emusb_usbhs_phy_enable(void);

/*
 * ISR-enable callback for USBD_SetISREnableFunc(): connects and enables the
 * USBHS controller interrupt (from the devicetree 'usbhs' node) and routes it
 * to the emUSB-supplied handler.
 */
void ifx_emusb_enable_isr(USB_ISR_HANDLER *pfISRHandler);

/*
 * Configure emUSB DMA support: install the cache-maintenance hooks and the
 * DMA-address validator (USBD_SetCacheConfig + USBD_SetCheckAddress). Call
 * once from USBD_X_Config() after selecting USB_Driver_Infineon_PSOCE84_DMA.
 * Only built when CONFIG_EMUSB_DEVICE_DMA=y.
 */
void ifx_emusb_dma_configure(void);

#ifdef __cplusplus
}
#endif

#endif /* IFX_EMUSB_USBHS_H */

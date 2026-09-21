/*
 * emUSB-Device platform glue for the Infineon PSE84 USBHS controller (Zephyr).
 *
 * Copyright (c) 2026 Infineon Technologies AG,
 * or an affiliate of Infineon Technologies AG.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Provides the reusable, board-independent hardware hooks the pre-compiled
 * emUSB-Device library needs on PSE84:
 *   - USBHS subsystem (SS) clock + PHY bring-up (device mode)
 *   - the controller interrupt wiring (USBD_SetISREnableFunc callback)
 *   - USBD_X_EnableInterrupt() / USBD_X_DisableInterrupt() platform hooks
 *
 * The bring-up sequence mirrors the Infineon USBHS quirk used elsewhere in
 * this SDK (drivers/usb/udc/udc_dwc2_infineon_usbhs.h and the TinyUSB port's
 * dwc2_ifx.h): the USBHS block is a Synopsys DWC2 core wrapped by an Infineon
 * SS register group that gates clocking and PHY power. All the SoC-specific
 * numbers come from the devicetree 'usbhs' node, so this file stays generic.
 */

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/irq.h>

#include "USB.h"
#include "ifx_emusb_usbhs.h"

/* Device header (USBHS_Type + SS masks) and SysClk helpers. We avoid the
 * <cy_pdl.h> umbrella (which drags in every noisy PDL driver header) and pull
 * only what the bring-up needs, exactly as the TinyUSB dwc2_ifx.h port does. */
#include <cy_device_headers.h>
#include <cy_sysclk.h>

#define USBHS_NODE DT_NODELABEL(usbhs)

#if !DT_NODE_EXISTS(USBHS_NODE)
#error "emUSB-Device requires a devicetree node labelled 'usbhs'"
#endif

/* The emUSB driver services the USBHS control interrupt (index 0 of the node's
 * 'interrupts' property, == usbhs_interrupt_usbhsctrl_IRQn). */
#define USBHS_IRQN     DT_IRQN(USBHS_NODE)
#define USBHS_IRQ_PRIO DT_IRQ(USBHS_NODE, priority)

/* emUSB installs its ISR at runtime via USBD_SetISREnableFunc(); Zephyr's
 * IRQ_CONNECT() needs a compile-time vector, so trampoline through here. */
static USB_ISR_HANDLER *s_emusb_isr;

static void ifx_emusb_usbhs_isr(const void *arg)
{
	ARG_UNUSED(arg);

	if (s_emusb_isr != NULL) {
		s_emusb_isr();
	}
}

void ifx_emusb_usbhs_phy_enable(void)
{
	USBHS_Type *wrapper = (USBHS_Type *)DT_REG_ADDR(USBHS_NODE);

	/* Enable and divide the USBHS peripheral-group clock (SS clock tree).
	 * Parameters are taken from the devicetree node so this stays generic
	 * across the secure / non-secure PSE84 variants. */
	Cy_SysClk_PeriGroupSlaveInit(DT_PROP(USBHS_NODE, mmio_peri_inst),
				     DT_PROP(USBHS_NODE, mmio_peri_group),
				     DT_PROP(USBHS_NODE, mmio_reg_bit_pos),
				     DT_PROP(USBHS_NODE, mmio_peri_hf_src));
	Cy_SysClk_PeriGroupSetDivider((DT_PROP(USBHS_NODE, mmio_peri_inst) << 8) |
					      DT_PROP(USBHS_NODE, mmio_peri_group),
				      DT_PROP(USBHS_NODE, mmio_peri_div));
	__DSB();

	/* SUBSYSTEM_CTL.USB_MODE = 0 selects device mode (asserts the D+ pull-up
	 * once the core connects); 1 would select host. Must match the emUSB
	 * device role or an idle port floats and reports a phantom connect. */
	wrapper->SS.SUBSYSTEM_CTL &= ~USBHS_SS_SUBSYSTEM_CTL_USB_MODE_Msk;
	wrapper->SS.SUBSYSTEM_CTL |= USBHS_SS_SUBSYSTEM_CTL_AHB_MASTER_SYNC_Msk;
	wrapper->SS.SUBSYSTEM_CTL &= ~USBHS_SS_SUBSYSTEM_CTL_USB_CTRL_SCALEDOWN_MODE_Msk;
	/* SS_ENABLE must be set before configuring the other SS registers. */
	wrapper->SS.SUBSYSTEM_CTL |= USBHS_SS_SUBSYSTEM_CTL_SS_ENABLE_Msk;

	wrapper->SS.PHY_FUNC_CTL_2 |= USBHS_SS_PHY_FUNC_CTL_2_EFUSE_SEL_Msk;
	wrapper->SS.PHY_FUNC_CTL_2 |= USBHS_SS_PHY_FUNC_CTL_2_RES_TUNING_SEL_Msk;
	__DSB();
}

void ifx_emusb_enable_isr(USB_ISR_HANDLER *pfISRHandler)
{
	s_emusb_isr = pfISRHandler;

	IRQ_CONNECT(USBHS_IRQN, USBHS_IRQ_PRIO, ifx_emusb_usbhs_isr, NULL, 0);
	irq_enable(USBHS_IRQN);
}

/* Platform hooks called by the emUSB library around critical sections. */
void USBD_X_EnableInterrupt(void)
{
	irq_enable(USBHS_IRQN);
}

void USBD_X_DisableInterrupt(void)
{
	irq_disable(USBHS_IRQN);
}

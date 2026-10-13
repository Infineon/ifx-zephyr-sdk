/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 Infineon Technologies AG,
 * SPDX-FileCopyrightText: or an affiliate of Infineon Technologies AG. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Vendor-class bulk USB function on emUSB-Device (Infineon PSE84).
 *
 * The data path runs from a dedicated worker thread (see main.c):
 *   - The producer uses overlapped (non-blocking) writes gated by
 *     USBD_BULK_WaitForTXReady. So, the IN endpoint streams without gaps.
 *   - Loopback uses blocking USBD_BULK_Read + USBD_BULK_Write.
 * Both avoid a completion-callback resubmit pattern (which recurses on this
 * stack); emUSB handles buffering/backpressure internally.
 */

#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>

#include "USB.h"
#include "USB_Bulk.h"

#include "vendor_bulk.h"
#include "vb_dma_mem.h"

/* High-speed bulk max packet size (512). emUSB negotiates FS/HS internally. */
#define VB_BULK_MPS USB_HS_BULK_MAX_PACKET_SIZE

/* Receive (staging) buffer for the bulk OUT endpoint. emUSB requires an OUT
 * endpoint buffer of at least MaxPacketSize (unlike IN, which can be NULL for
 * the zero-copy DMA path). The controller DMAs incoming packets here, so it
 * lives in VB_DMA_MEM; a few packets deep to absorb bursts between reads. */
#define VB_EP_OUT_BUF_SIZE (8u * VB_BULK_MPS)

static uint8_t vb_ep_out_buf[VB_EP_OUT_BUF_SIZE] VB_DMA_MEM;

static USB_BULK_HANDLE vb_handle;
static atomic_t vb_enabled;

void vendor_bulk_add(void)
{
	USB_ADD_EP_INFO ep_in;
	USB_ADD_EP_INFO ep_out;
	USB_BULK_INIT_DATA init;

	memset(&ep_in, 0, sizeof(ep_in));
	ep_in.Flags = 0;
	ep_in.InDir = USB_DIR_IN;
	ep_in.Interval = 0; /* ignored for bulk */
	ep_in.MaxPacketSize = VB_BULK_MPS;
	ep_in.TransferType = USB_TRANSFER_TYPE_BULK;

	memset(&ep_out, 0, sizeof(ep_out));
	ep_out.Flags = 0;
	ep_out.InDir = USB_DIR_OUT;
	ep_out.Interval = 0;
	ep_out.MaxPacketSize = VB_BULK_MPS;
	ep_out.TransferType = USB_TRANSFER_TYPE_BULK;

	memset(&init, 0, sizeof(init));
	/* IN: NULL/0 keeps the zero-copy DMA path (DMA straight from the
	 * caller's buffer). OUT: emUSB requires a receive buffer >= MaxPacketSize
	 * to land incoming packets; NULL here silently drops all OUT data. */
	init.EPIn = USBD_AddEPEx(&ep_in, NULL, 0);
	init.EPOut = USBD_AddEPEx(&ep_out, vb_ep_out_buf, sizeof(vb_ep_out_buf));

	vb_handle = USBD_BULK_Add(&init);

	/* Default interface class is vendor-specific (0xFF), matching the
	 * upstream sample. Use USBD_BULK_Add_Ex + USB_BULK_INIT_DATA_EX for a
	 * named interface or custom class IDs. */
}

void vendor_bulk_set_enabled(bool enabled)
{
	atomic_set(&vb_enabled, enabled ? 1 : 0);
}

bool vendor_bulk_is_ready(void)
{
	return atomic_get(&vb_enabled) != 0;
}

int vendor_bulk_write(const uint8_t *data, size_t len)
{
	if (!atomic_get(&vb_enabled)) {
		return -ENETDOWN;
	}

	/* Timeout 0 = block until the host has read the data. */
	return USBD_BULK_Write(vb_handle, data, (unsigned)len, 0);
}

int vendor_bulk_write_start(const uint8_t *data, size_t len)
{
	if (!atomic_get(&vb_enabled)) {
		return -ENETDOWN;
	}

	/* Timeout -1 = overlapped: queue the transfer and return immediately so
	 * the next one can be submitted while this one is still in flight. The
	 * stack queues a small number of transfers, keeping the endpoint fed
	 * across transfer boundaries. @p data must stay valid and unchanged
	 * until the transfer completes (the producer streams a persistent,
	 * never-modified buffer, so reusing it for several in-flight transfers
	 * is safe). Returns 0 when the transfer was started. */
	return USBD_BULK_Write(vb_handle, data, (unsigned)len, -1);
}

int vendor_bulk_wait_tx_ready(int timeout_ms)
{
	if (!atomic_get(&vb_enabled)) {
		return -ENETDOWN;
	}

	/* 0 = queue can accept another write, 1 = queue full (timeout), <0 err. */
	return USBD_BULK_WaitForTXReady(vb_handle, timeout_ms);
}

unsigned vendor_bulk_bytes_pending(void)
{
	return USBD_BULK_GetNumBytesRemToWrite(vb_handle);
}

int vendor_bulk_read(uint8_t *data, size_t len)
{
	if (!atomic_get(&vb_enabled)) {
		return -ENETDOWN;
	}

	/* Timeout 0 = block until at least one packet is received. */
	return USBD_BULK_Read(vb_handle, data, (unsigned)len, 0);
}

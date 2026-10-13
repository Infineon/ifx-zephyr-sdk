/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 Infineon Technologies AG,
 * SPDX-FileCopyrightText: or an affiliate of Infineon Technologies AG. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * emUSB-Device vendor-class bulk sample (Zephyr, Infineon PSE84 USBHS).
 *
 * A dedicated worker thread drives the data path:
 *   - Producer mode: continuously stream a counter pattern on bulk IN using
 *     overlapped (non-blocking) writes, keeping the stack's transfer queue
 *     full so the endpoint is never starved between transfers (maximum
 *     throughput; see USBD_BULK_WaitForTXReady in the emUSB manual).
 *   - Loopback mode: blocking read on bulk OUT, echoed back on bulk IN.
 *
 * When CONFIG_APP_VENDOR_BULK_STATS is enabled the producer reports the
 * device-side transfer rate once per second (see the main loop).
 */

#include <string.h>

#include <zephyr/kernel.h>

#include "USB.h"

#include "vendor_bulk.h"
#include "vb_dma_mem.h"

/* Zephyr project test IDs (do NOT ship outside samples). */
#define VENDOR_ID  (0x2FE3)
#define PRODUCT_ID (0x000A)

#define XFER_SIZE CONFIG_APP_VENDOR_BULK_XFER_SIZE

/* Worker thread that runs the bulk data path once the device is configured. */
#define VB_THREAD_STACK_SIZE 2048
#define VB_THREAD_PRIO       5

#if defined(CONFIG_APP_VENDOR_BULK_MODE_SOURCE)
#define VB_MODE_NAME "source"
#elif defined(CONFIG_APP_VENDOR_BULK_MODE_SINK)
#define VB_MODE_NAME "sink"
#else
#define VB_MODE_NAME "loopback"
#endif

static const USB_DEVICE_INFO usb_dev_info = {
	VENDOR_ID,
	PRODUCT_ID,
	"Infineon Technologies",
	"emUSB vendor bulk sample",
	"0123456789",
};

#if defined(CONFIG_APP_VENDOR_BULK_STATS)
/* Throughput diagnostics (updated by the worker, sampled once/sec by main).
 * Counters are cumulative and read as per-second deltas; 32-bit wrap is
 * handled by unsigned subtraction of the delta. */
static atomic_t vb_bytes;  /* bytes transferred */
static atomic_t vb_xfers;  /* transfer count */
#if defined(CONFIG_APP_VENDOR_BULK_MODE_SOURCE)
static atomic_t vb_drains; /* times the TX queue had fully drained before refill */
#define VB_DIR_LABEL "IN"
#else
#define VB_DIR_LABEL "OUT"
#endif
#endif /* CONFIG_APP_VENDOR_BULK_STATS */

#if defined(CONFIG_APP_VENDOR_BULK_MODE_SOURCE)

/* Source buffer streamed on bulk IN. */
static uint8_t in_buf[XFER_SIZE] VB_DMA_MEM;

static void fill_pattern_once(void)
{
	for (size_t i = 0; i < XFER_SIZE; i += sizeof(uint32_t)) {
		uint32_t v = (uint32_t)(i / sizeof(uint32_t));

		in_buf[i + 0] = (uint8_t)(v);
		in_buf[i + 1] = (uint8_t)(v >> 8);
		in_buf[i + 2] = (uint8_t)(v >> 16);
		in_buf[i + 3] = (uint8_t)(v >> 24);
	}
}

static void vb_worker(void *a, void *b, void *c)
{
	ARG_UNUSED(a);
	ARG_UNUSED(b);
	ARG_UNUSED(c);

	fill_pattern_once();

	for (;;) {
		int r;

		if (!vendor_bulk_is_ready()) {
			k_msleep(10);
			continue;
		}

		/* Keep the stack's write queue full for maximum send speed:
		 * wait until it can accept another transfer, then submit an
		 * overlapped (non-blocking) write. Several transfers stay in
		 * flight, so the IN endpoint is not starved between them. */
		r = vendor_bulk_wait_tx_ready(100);
		if (r < 0) {
			/* Disconnected / reset while waiting. */
			k_msleep(10);
			continue;
		}
		if (r == 1) {
			/* Queue full within the timeout (host not draining). */
			continue;
		}

#if defined(CONFIG_APP_VENDOR_BULK_STATS)
		/* If nothing is pending at refill time, the endpoint went idle
		 * between transfers (a closable gap). Near-zero drains means we
		 * are streaming at the driver/bus ceiling. */
		if (vendor_bulk_bytes_pending() == 0) {
			atomic_inc(&vb_drains);
		}
#endif

		if (vendor_bulk_write_start(in_buf, XFER_SIZE) < 0) {
			k_msleep(10);
			continue;
		}

#if defined(CONFIG_APP_VENDOR_BULK_STATS)
		atomic_add(&vb_bytes, XFER_SIZE);
		atomic_inc(&vb_xfers);
#endif
	}
}

#elif defined(CONFIG_APP_VENDOR_BULK_MODE_SINK)

/* Discard buffer for draining bulk OUT (host-to-device throughput). */
static uint8_t sink_buf[XFER_SIZE] VB_DMA_MEM;

static void vb_worker(void *a, void *b, void *c)
{
	ARG_UNUSED(a);
	ARG_UNUSED(b);
	ARG_UNUSED(c);

	for (;;) {
		int n;

		if (!vendor_bulk_is_ready()) {
			k_msleep(10);
			continue;
		}

		/* Blocking read drains whatever the host writes on bulk OUT;
		 * the data is discarded. A large read receives many packets per
		 * call, keeping the endpoint armed for the whole transfer. */
		n = vendor_bulk_read(sink_buf, sizeof(sink_buf));
		if (n > 0) {
#if defined(CONFIG_APP_VENDOR_BULK_STATS)
			atomic_add(&vb_bytes, n);
			atomic_inc(&vb_xfers);
#endif
		} else if (n < 0) {
			k_msleep(10);
		}
	}
}

#else /* CONFIG_APP_VENDOR_BULK_MODE_LOOPBACK */

static uint8_t io_buf[XFER_SIZE] VB_DMA_MEM;

static void vb_worker(void *a, void *b, void *c)
{
	ARG_UNUSED(a);
	ARG_UNUSED(b);
	ARG_UNUSED(c);

	for (;;) {
		int n;

		if (!vendor_bulk_is_ready()) {
			k_msleep(10);
			continue;
		}

		n = vendor_bulk_read(io_buf, sizeof(io_buf));
		if (n > 0) {
			(void)vendor_bulk_write(io_buf, (size_t)n);
		} else if (n < 0) {
			k_msleep(10);
		}
	}
}

#endif /* mode */

K_THREAD_DEFINE(vb_worker_tid, VB_THREAD_STACK_SIZE, vb_worker, NULL, NULL, NULL,
		VB_THREAD_PRIO, 0, 0);

int main(void)
{
	bool ready = false;
#if defined(CONFIG_APP_VENDOR_BULK_STATS)
	int64_t stats_last = k_uptime_get();
	atomic_val_t bytes_last = 0;
	atomic_val_t xfers_last = 0;
#if defined(CONFIG_APP_VENDOR_BULK_MODE_SOURCE)
	atomic_val_t drains_last = 0;
#endif
#endif

	printk("\nPSE84 emUSB-Device: vendor bulk sample (" VB_MODE_NAME " mode)\n");

	USBD_Init();
	vendor_bulk_add();

	USBD_SetDeviceInfo(&usb_dev_info);
	USBD_Start();

	/* Track configured state; the worker thread runs the data path while
	 * enabled. */
	for (;;) {
		bool configured =
			(USBD_GetState() &
			 (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) == USB_STAT_CONFIGURED;

		if (configured != ready) {
			ready = configured;
			vendor_bulk_set_enabled(ready);
			printk("vendor bulk %s\n", ready ? "ready" : "not ready");
		}

		k_msleep(100);

#if defined(CONFIG_APP_VENDOR_BULK_STATS)
		if (ready) {
			int64_t now = k_uptime_get();
			int64_t dt = now - stats_last;

			if (dt >= 1000) {
				atomic_val_t b = atomic_get(&vb_bytes);
				atomic_val_t x = atomic_get(&vb_xfers);
				uint32_t dbytes =
					(uint32_t)b - (uint32_t)bytes_last;
				uint32_t mib_x100 =
					(uint32_t)(((uint64_t)dbytes * 100U * 1000U) /
						   ((uint64_t)dt * 1024U * 1024U));

#if defined(CONFIG_APP_VENDOR_BULK_MODE_SOURCE)
				atomic_val_t d = atomic_get(&vb_drains);

				printk(VB_DIR_LABEL ": %u.%02u MiB/s  %u xfers/s  %u drains/s\n",
				       mib_x100 / 100U, mib_x100 % 100U,
				       (unsigned)(x - xfers_last),
				       (unsigned)(d - drains_last));
				drains_last = d;
#else
				printk(VB_DIR_LABEL ": %u.%02u MiB/s  %u xfers/s\n",
				       mib_x100 / 100U, mib_x100 % 100U,
				       (unsigned)(x - xfers_last));
#endif
				bytes_last = b;
				xfers_last = x;
				stats_last = now;
			}
		}
#endif
	}

	return 0;
}

/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 Infineon Technologies AG,
 * SPDX-FileCopyrightText: or an affiliate of Infineon Technologies AG. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SAMPLES_EMUSB_VENDOR_BULK_H_
#define SAMPLES_EMUSB_VENDOR_BULK_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Register the vendor-class bulk interface with emUSB.
 *
 * Allocates the bulk IN and bulk OUT endpoints (USBD_AddEPEx) and adds the
 * emUSB Bulk class (USBD_BULK_Add, vendor class 0xFF). Must be called after
 * USBD_Init() and before USBD_Start().
 */
void vendor_bulk_add(void);

/**
 * Enable/disable the data path. Set true once the host has configured the
 * device (USB_STAT_CONFIGURED), false on disconnect/suspend.
 */
void vendor_bulk_set_enabled(bool enabled);

/** True after the host has configured the device and the class is enabled. */
bool vendor_bulk_is_ready(void);

/**
 * Blocking bulk write on the vendor-class IN endpoint.
 *
 * Must be called from a thread (never from a USB completion callback). Blocks
 * until the host has read all of @p data. @p data must stay valid until the
 * call returns. Used by loopback mode; the producer uses the overlapped
 * variant below.
 *
 * @return number of bytes written (>=0), or a negative emUSB error code.
 */
int vendor_bulk_write(const uint8_t *data, size_t len);

/**
 * Start an overlapped (non-blocking) bulk write on the IN endpoint.
 *
 * Queues the transfer and returns immediately; the data is sent
 * asynchronously. Gate submissions with vendor_bulk_wait_tx_ready() so the
 * stack's transfer queue is kept full — this keeps the IN endpoint fed across
 * transfer boundaries for maximum throughput. @p data must remain valid and
 * unchanged until the transfer completes.
 *
 * @return 0 if the transfer was started, negative on error/queue-full.
 */
int vendor_bulk_write_start(const uint8_t *data, size_t len);

/**
 * Wait until the IN transfer queue can accept another overlapped write.
 *
 * @param timeout_ms 0 = block forever (never reports full), >0 = bounded wait,
 *                   <0 = poll and return immediately.
 * @return 0 = a new write will be accepted, 1 = queue full (timeout),
 *         negative on error.
 */
int vendor_bulk_wait_tx_ready(int timeout_ms);

/**
 * Number of bytes still queued/pending on the IN endpoint (across all
 * in-flight overlapped writes). Returns 0 when the write pipeline has fully
 * drained. Used for throughput diagnostics. Does not block.
 */
unsigned vendor_bulk_bytes_pending(void);

/**
 * Blocking bulk read on the vendor-class OUT endpoint.
 *
 * @return number of bytes received (>=0), or a negative emUSB error code.
 */
int vendor_bulk_read(uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SAMPLES_EMUSB_VENDOR_BULK_H_ */

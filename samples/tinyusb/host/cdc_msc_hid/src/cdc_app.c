/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022, Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This file is part of the TinyUSB stack.
 */

#include "tusb.h"
#include "app.h"

void cdc_app_task(void) {
  const uint8_t idx = 0;

  if (!tuh_cdc_mounted(idx)) {
    return;
  }

  // Drain data from the device. This port has no console bridge, so the bytes
  // are discarded; reading keeps the CDC RX pipe from stalling.
  uint8_t buf[64];
  tuh_cdc_read(idx, buf, sizeof(buf));
}

//--------------------------------------------------------------------+
// TinyUSB callbacks
//--------------------------------------------------------------------+

// Invoked when a device with CDC interface is mounted
// idx is index of cdc interface in the internal pool.
void tuh_cdc_mount_cb(uint8_t idx) {
  tuh_itf_info_t itf_info = {0};
  tuh_cdc_itf_get_info(idx, &itf_info);

  printf("CDC Interface is mounted: address = %u, itf_num = %u\r\n", itf_info.daddr,
         itf_info.desc.bInterfaceNumber);

  // If CFG_TUH_CDC_LINE_CODING_ON_ENUM is defined, line coding will be set by tinyusb stack
  // while eneumerating new cdc device
  cdc_line_coding_t line_coding = {0};
  if (tuh_cdc_get_line_coding_local(idx, &line_coding)) {
    printf("  Baudrate: %" PRIu32 ", Stop Bits : %u\r\n", line_coding.bit_rate, line_coding.stop_bits);
    printf("  Parity  : %u, Data Width: %u\r\n", line_coding.parity, line_coding.data_bits);
  }
}

// Invoked when a device with CDC interface is unmounted
void tuh_cdc_umount_cb(uint8_t idx) {
  tuh_itf_info_t itf_info = {0};
  tuh_cdc_itf_get_info(idx, &itf_info);

  printf("CDC Interface is unmounted: address = %u, itf_num = %u\r\n", itf_info.daddr,
         itf_info.desc.bInterfaceNumber);
}

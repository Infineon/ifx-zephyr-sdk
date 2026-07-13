/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
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
 */

#include <stdio.h>

#include "tusb.h"
#include "app.h"

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTOTYPES
//--------------------------------------------------------------------+
void led_blinking_task(void);

// Run tuh_task() in a dedicated COOPERATIVE (higher-than-main) thread so the
// host event queue is drained promptly even while the main thread is busy in
// blocking console printf (Zephyr uart_poll_out busy-waits and does not yield).
#define USB_HOST_STACK_SIZE 3072
static K_THREAD_STACK_DEFINE(usb_host_stack, USB_HOST_STACK_SIZE);
static struct k_thread usb_host_thread_data;

static void usb_host_thread(void *p1, void *p2, void *p3) {
  (void) p1; (void) p2; (void) p3;

  // Init the host stack on the configured roothub port.
  tusb_rhport_init_t host_init = {
    .role = TUSB_ROLE_HOST,
    .speed = TUSB_SPEED_AUTO
  };
  tusb_init(BOARD_TUH_RHPORT, &host_init);

  while (1) {
    tuh_task(); // blocks on the host event queue; wakes on USB events
  }
}

int main(void) {
  board_init();

  printf("TinyUSB Host CDC MSC HID Example\r\n");

  k_thread_create(&usb_host_thread_data, usb_host_stack,
                  K_THREAD_STACK_SIZEOF(usb_host_stack),
                  usb_host_thread, NULL, NULL, NULL,
                  K_PRIO_COOP(4), 0, K_NO_WAIT);
  k_thread_name_set(&usb_host_thread_data, "tuh_task");

  // Wait for the host stack to come up before running the app tasks.
  while (!tuh_inited()) {
    k_msleep(1);
  }

  while (1) {
    // Class app tasks + LED run in the main thread; the host thread owns
    // tuh_task(). The cooperative host thread preempts main on USB events, so
    // the event queue is drained even while these tasks block in printf.
    led_blinking_task();
    cdc_app_task();
    hid_app_task();
    k_msleep(1);
  }
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

void tuh_mount_cb(uint8_t dev_addr) {
  // application set-up
  printf("A device with address %u is mounted\r\n", dev_addr);
}

void tuh_umount_cb(uint8_t dev_addr) {
  // application tear-down
  printf("A device with address %u is unmounted \r\n", dev_addr);
}


//--------------------------------------------------------------------+
// Blinking Task
//--------------------------------------------------------------------+
void led_blinking_task(void) {
  const uint32_t interval_ms = 1000;
  static uint32_t start_ms = 0;

  static bool led_state = false;

  // Blink every interval ms
  if (tusb_time_millis_api() - start_ms < interval_ms) {
    return;// not enough time
  }
  start_ms += interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}

//--------------------------------------------------------------------+
// Board glue (Zephyr-native; replaces the TinyUSB hw/bsp board layer)
//--------------------------------------------------------------------+
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});

#if CFG_TUH_ENABLED
// KIT_PSE84_EVAL routes the USB Host signals (Type-A connector J27) through a
// mux (U25) and a VBUS power switch (U21/U22), both gated by USB_HOST_EN
// (P17[5]). The pin is low by default (device USB connected, host VBUS off);
// firmware must drive it high to switch the mux to J27 and enable 5V VBUS_HOST.
// USB_FAULT (P16[4], active-low) reports host VBUS overcurrent. Both pins come
// from the sample's board overlay via the zephyr,user node.
static const struct gpio_dt_spec usb_host_en = GPIO_DT_SPEC_GET_OR(DT_PATH(zephyr_user), host_en_gpios, {0});
static const struct gpio_dt_spec usb_fault = GPIO_DT_SPEC_GET_OR(DT_PATH(zephyr_user), usb_fault_gpios, {0});
#endif

void board_init(void) {
  // LED
  if (gpio_is_ready_dt(&led)) {
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
  }

#if CFG_TUH_ENABLED
  // Enable the USB Host path: drive USB_HOST_EN high (switches mux U25 to J27
  // and turns on VBUS_HOST). Without this the host port is disconnected and
  // unpowered, so no device can be enumerated.
  if (gpio_is_ready_dt(&usb_host_en)) {
    gpio_pin_configure_dt(&usb_host_en, GPIO_OUTPUT_ACTIVE);
  }
  // USB_FAULT is an input (active-low overcurrent flag); configure for reading.
  if (gpio_is_ready_dt(&usb_fault)) {
    gpio_pin_configure_dt(&usb_fault, GPIO_INPUT);
  }
#endif
}

void board_led_write(bool state) {
  if (gpio_is_ready_dt(&led)) {
    gpio_pin_set_dt(&led, state ? 1 : 0);
  }
}

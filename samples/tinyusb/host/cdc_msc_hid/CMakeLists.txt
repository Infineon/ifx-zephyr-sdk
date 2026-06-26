# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(tinyusb_host_cdc_msc_hid)

# TinyUSB module location (provides the USB stack + DWC2 controller driver and
# the reusable hw/bsp board glue). ZEPHYR_TINYUSB_MODULE_DIR points at the
# tinyusb-zephyr asset; the upstream TinyUSB sources (incl. hw/bsp) are checked
# out into its 'tinyusb' subdirectory.
set(TINYUSB_DIR ${ZEPHYR_TINYUSB_MODULE_DIR}/tinyusb)

# Application sources.
target_sources(app PRIVATE
  src/main.c
  src/cdc_app.c
  src/hid_app.c
  src/msc_app.c
)

# Board support glue reused from the TinyUSB BSP layer:
#  - board.c       : generic printf/console retarget + weak board API defaults
#  - pse84/family.c: PSE84 board_init(), USBHS IRQ wiring, host VBUS enable
target_sources(app PRIVATE
  ${TINYUSB_DIR}/hw/bsp/board.c
  ${TINYUSB_DIR}/hw/bsp/pse84/family.c
)

target_include_directories(app PRIVATE
  src
  ${TINYUSB_DIR}/hw         # for "bsp/board_api.h"
  ${TINYUSB_DIR}/hw/bsp     # for "board_api.h"
  ${TINYUSB_DIR}/hw/bsp/pse84
)

# tusb_config.h lives in this sample's src/. It must also be visible to the
# TinyUSB module sources, which are compiled in the module's own CMake scope,
# so expose it on the global include path.
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

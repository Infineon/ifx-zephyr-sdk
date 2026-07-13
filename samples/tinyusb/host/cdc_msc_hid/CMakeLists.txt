# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(tinyusb_host_cdc_msc_hid)

# Application sources. This sample is fully Zephyr-native: the USBHS controller
# interrupt is connected by the ifx-tinyusb module
target_sources(app PRIVATE
  src/main.c
  src/cdc_app.c
  src/hid_app.c
  src/msc_app.c
)

target_include_directories(app PRIVATE
  src
)

# tusb_config.h lives in this sample's src/. It must also be visible to the
# TinyUSB module sources, which are compiled in the module's own CMake scope,
# so expose it on the global include path.
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

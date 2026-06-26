.. zephyr:code-sample:: tinyusb-host-cdc-msc-hid
   :name: TinyUSB host CDC/MSC/HID

   Enumerate USB CDC, MSC and HID devices using the TinyUSB host stack.

Overview
********

This sample exercises the :ref:`TinyUSB <tinyusb>` Zephyr module by running the
TinyUSB **host** stack. It enumerates attached USB devices and prints
information about CDC (serial), MSC (mass storage) and HID (keyboard/mouse)
interfaces.

The sample drives the TinyUSB module (``CONFIG_TINYUSB``) rather than the
in-tree Zephyr USB stack. Zephyr's own USB host/device stacks are disabled on
the target board so that TinyUSB owns the USB controller.

Requirements
************

A board with a TinyUSB-supported USB controller. This sample is configured for
the Infineon PSOC Edge E84 evaluation kit (high-speed DWC2 controller) on the
secure Cortex-M33 partition.

Building and Running
********************

.. code-block:: console

   west build -b kit_pse84_eval/pse846gps2dbzc4a/m33 \
       ifx-zephyr-sdk/samples/tinyusb/host_cdc_msc_hid
   west flash

Attach a USB device to the host Type-A connector (J27) and observe the
enumeration output.

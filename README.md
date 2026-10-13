# Infineon Zephyr SDK

This repository contains the Infineon Zephyr SDK: a collection of sample applications and board
support targeting Infineon PSE84 hardware, together with the west manifest that ties the full
software stack together.  Using `west init` against this repo pulls in the Zephyr kernel,
standard Zephyr modules, and Infineon-specific dependencies (ML Middleware, TFLite Micro
precompiled libraries, and the Infineon HAL) in one step.

## Workspace Structure

The venv and all west-managed projects live together under a single directory
(named `workspace` in the examples below — change it to suit your project):

```
workspace\
├── .venv\                   ← Python virtual environment
├── .west\                   ← west metadata
├── ifx-zephyr-sdk\          ← this repo (west manifest); samples\ lives here
├── ifx-zephyr\              ← Infineon Zephyr fork (fetched by west)
└── modules\                 ← all west-managed modules
```

## Prerequisites

- Python 3.10+ (to create the virtual environment)
- [Zephyr SDK](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html) installed
- Infineon OpenOCD: https://github.com/infineon/openocd/releases
- See the Zephyr getting started guide for additional information:
  https://docs.zephyrproject.org/latest/develop/getting_started/index.html
---

## Setting Up the Environment

### 1. Create the workspace directory and activate a Python virtual environment

```bat
mkdir workspace
python -m venv workspace\.venv
workspace\.venv\Scripts\activate
```

### 2. Install west

```bat
pip install west
```

### 3. Initialise the west workspace

`west init` targets the existing `workspace\` directory, cloning `ifx-zephyr-sdk` and initializing
a west workspace

```bat
west init -m https://www.github.com/infineon/ifx-zephyr-sdk workspace
```

### 4. Fetch all projects and finish setup

```bat
cd workspace
west update
west zephyr-export
west packages pip --install
```

### 5. (Optional) Configure Infineon OpenOCD path

Required if your OpenOCD is not on `PATH`. Rebuild after changing this setting.

```bat
west config build.cmake-args -- -DOPENOCD=path\to\infineon\openocd\bin\openocd.exe
```

All `west build` commands are run from the workspace root (`workspace\`).

## Included Samples

All samples live under `ifx-zephyr-sdk\samples\`. Run `west build` commands from the workspace
root (`workspace\`).

### emusb/vendor_bulk
Demonstrates a high-speed USB vendor-class (`0xFF`) bulk device on the PSE84 USBHS controller
using the SEGGER emUSB-Device stack (Infineon precompiled library, fetched by `west update` as the
`emusb-device` module). Zephyr's native USB stack is disabled; emUSB owns the controller and uses
the DMA driver for bulk throughput.

The device enumerates as VID `0x2FE3` / PID `0x000A` (Zephyr test IDs - do not ship) with one
interface containing a bulk OUT (`0x01`) and a bulk IN (`0x81`) endpoint. The data path mode is
selected at build time:

| Kconfig option                       | Device behavior                                   | Host test (`--mode`) |
|--------------------------------------|---------------------------------------------------|----------------------|
| `CONFIG_APP_VENDOR_BULK_MODE_SOURCE` | Streams a counter pattern on bulk IN (default)    | `in` or `in-async`   |
| `CONFIG_APP_VENDOR_BULK_MODE_SINK`   | Reads and discards bulk OUT                       | `out`                |
| `CONFIG_APP_VENDOR_BULK_MODE_LOOPBACK` | Echoes bulk OUT back on bulk IN                 | Not currently supported |

Other options: `CONFIG_APP_VENDOR_BULK_XFER_SIZE` (per-transfer size, default 65536 bytes) and
`CONFIG_APP_VENDOR_BULK_STATS` (prints device-side MiB/s once per second in source/sink mode).

Supported targets: `kit_pse84_eval/pse846gps2dbzc4a/m55` and
`kit_pse84_eval/pse846gps2dbzc4a/m33/ns`.

**Build and flash** (default source mode, M55):
```bat
west build -p -b kit_pse84_eval/pse846gps2dbzc4a/m55 ifx-zephyr-sdk\samples\emusb\vendor_bulk --sysbuild
west flash
```

To select a different mode, override the Kconfig choice on the command line, e.g. sink mode:
```bat
west build -p -b kit_pse84_eval/pse846gps2dbzc4a/m55 ifx-zephyr-sdk\samples\emusb\vendor_bulk --sysbuild -- -DCONFIG_APP_VENDOR_BULK_MODE_SINK=y
```

Connect the kit's USB device port to the host. The console prints
`PSE84 emUSB-Device: vendor bulk sample (<mode> mode)`, then `vendor bulk ready` once the host
configures the device, followed by throughput statistics if enabled.

**Host throughput test**

`vendor_bulk_speed_test.py` (in the sample directory) measures bulk throughput from the host.
On Windows, first use [Zadig](https://zadig.akeo.ie) to bind WinUSB to interface 0 of the device.
On Linux, run with sufficient permissions for the USB device (e.g. a udev rule).
```bat
pip install pyusb libusb-package
python ifx-zephyr-sdk\samples\emusb\vendor_bulk\vendor_bulk_speed_test.py --mode in
```
Use `--mode in-async` (requires `pip install libusb1`) for a pipelined IN test that keeps
multiple transfers in flight, or `--mode out` against a sink-mode build. Run with `--help` for
options such as `--xfer`, `--seconds`, and `--queue`.

### lvgl-7in
A clone of `samples/subsys/display/lvgl` with a board overlay for the PSE84 Eval Kit's 7″
Waveshare display module instead of the standard 4.3″ module.
```bat
west build -p -b kit_pse84_eval/pse846gps2dbzc4a/m55 ifx-zephyr-sdk\samples\lvgl-7in --sysbuild
west flash
```

### ml-middleware
Demonstrates running a [DEEPCRAFT Studio](https://imagimob.com/deepcraft) model on Zephyr using
the Infineon ML Middleware, targeting both the M33 + NNLite and M55 + Ethos-U55 configurations.
See [samples/ml-middleware/README.md](samples/ml-middleware/README.md) for full build and usage
details.

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

## Branch/Repo Structure

Currently some features are not supported on the main branches of ifx-zephyr and 
ifx-zephyr-hal_infineon. 

Features supported on the main branches:
- emusb-device

Additional feature support status (by branch):
- lvgl-7in and ml-middleware
    - ifx-zephyr-sdk: topic/downstream-display-and-ml
    - ifx-zephyr: topic/downstream-display
    - ifx-zephyr-hal_infineon: topic/downstream-ml

Running `west update` while on the topic/downstream-display-and-ml branch will fetch the correct 
code revisions from ifx-zephyr and ifx-zephyr-hal_infineon.

## Prerequisites

- Python 3.10+ (to create the virtual environment)
- Git CLI
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

| Sample | Description |
|--------|-------------|
| [MTB-STL POST tests](#mtb-stl-post-tests) | Eight power-on self-test samples running Infineon MTB-STL safety tests on PSC3M5 and PSoC4 |
| [emusb/vendor_bulk](#emusbvendor_bulk) | High-speed USB vendor-class bulk device on PSE84 using SEGGER emUSB-Device |
| [lvgl-7in](#lvgl-7in)\* | LVGL display sample for the PSE84 Eval Kit's 7″ Waveshare display |
| [ml-middleware](#ml-middleware)\* | DEEPCRAFT Studio model on PSE84 through the Infineon ML Middleware (NNLite or Ethos-U55) |

\* Currently not fully supported on the main branch of ifx-zephyr-sdk.

### MTB-STL POST tests

The [POST samples](samples/post) use the `ifx-zephyr-stl` module for the generic
POST framework and Infineon's MTB-STL adaptation. The following eight samples
are included. Board names below refer to PSC3M5 (`kit_psc3m5_evk`) and PSoC4
(`cy8cproto_041tp`); each sample's README describes its hardware setup and test
limitations.

| Sample | Tests | Board configurations |
|--------|-------|----------------------|
| [post-mtb-stl](samples/post/post-mtb-stl/README.rst) | CPU, RAM/stack/flash, GPIO, watchdog, interrupt, clock, DMA and startup registers; FPU on PSC3M5 | PSC3M5, PSoC4 |
| [post-tcpwm-test-mtb-stl](samples/post/post-tcpwm-test-mtb-stl/README.rst) | Counter, PWM duty cycle and PWM gate/kill | PSC3M5, PSoC4 |
| [post-i2c-test-mtb-stl](samples/post/post-i2c-test-mtb-stl/README.rst) | I2C master/slave communication | PSC3M5, PSoC4 |
| [post-spi-test-mtb-stl](samples/post/post-spi-test-mtb-stl/README.rst) | SPI MOSI-to-MISO loopback | PSC3M5, PSoC4 |
| [post-uart-test-mtb-stl](samples/post/post-uart-test-mtb-stl/README.rst) | UART loopback and master/slave communication | PSC3M5, PSoC4 |
| [post-adc-test-mtb-stl](samples/post/post-adc-test-mtb-stl/README.rst) | ADC with internal stimulus on PSC3M5 or external input on PSoC4 | PSC3M5, PSoC4 |
| [post-comparator-test-mtb-stl](samples/post/post-comparator-test-mtb-stl/README.rst) | LPCOMP output compared with the configured expected level | PSC3M5 |
| [post-dac-test-mtb-stl](samples/post/post-dac-test-mtb-stl/README.rst) | DAC zero, midpoint and full-scale values measured through ADC | PSC3M5 |

After completing workspace setup above, run the following from the workspace
root with the Python environment activated:

```bat
west update
west build -b kit_psc3m5_evk -d build-post-core ifx-zephyr-sdk/samples/post/post-mtb-stl
```

For the PSoC4 core sample, use a separate build directory:

```bat
west build -b cy8cproto_041tp -d build-post-core-psoc4 ifx-zephyr-sdk/samples/post/post-mtb-stl
```

Build any peripheral sample independently on PSC3M5:

```bat
west build -b kit_psc3m5_evk -d build-post-tcpwm ifx-zephyr-sdk/samples/post/post-tcpwm-test-mtb-stl
west build -b kit_psc3m5_evk -d build-post-i2c ifx-zephyr-sdk/samples/post/post-i2c-test-mtb-stl
west build -b kit_psc3m5_evk -d build-post-spi ifx-zephyr-sdk/samples/post/post-spi-test-mtb-stl
west build -b kit_psc3m5_evk -d build-post-uart ifx-zephyr-sdk/samples/post/post-uart-test-mtb-stl
west build -b kit_psc3m5_evk -d build-post-adc ifx-zephyr-sdk/samples/post/post-adc-test-mtb-stl
west build -b kit_psc3m5_evk -d build-post-comparator ifx-zephyr-sdk/samples/post/post-comparator-test-mtb-stl
west build -b kit_psc3m5_evk -d build-post-dac ifx-zephyr-sdk/samples/post/post-dac-test-mtb-stl
```

For peripheral samples with a PSoC4 board configuration in the table, replace
`-b kit_psc3m5_evk` with `-b cy8cproto_041tp` and use a separate build directory,
such as `-d build-post-i2c-psoc4`. Comparator and DAC samples only provide PSC3M5
board configurations.

The SDK manifest lists `ifx-zephyr-stl` for this integration. Its revision
includes both the POST integration and the unchanged MTB-STL vendor assets
under `modules/lib/ifx-zephyr-stl/mtb-stl/`. A single `west update` fetches both;
no separate asset download command is needed. If only refreshing the STL
project, `west update ifx-zephyr-stl` also includes its vendor files.
Fetching this project requires access to the internal GitLab host configured
in [west.yml](west.yml).

`mtb-stl` is an ordinary directory inside the integration repository, not a
submodule or separate west project. `west update mtb-stl` is not available.
The asset updater is used only by maintainers preparing a vendor release
update. Normal builds do not fetch or update assets.

Read the [core sample instructions](samples/post/post-mtb-stl/README.rst) for
startup-reference provisioning, reserved resources and result interpretation
before running the tests. Peripheral samples under [samples/post](samples/post)
document their own wiring and driver-handoff requirements. Successful asset
verification or compilation does not replace hardware validation.

After completing the selected sample's hardware setup (and startup-reference
provisioning for the core sample), flash its build directory, for example:

```bat
west flash -d build-post-core
```

For a peripheral sample, select its corresponding directory, such as
`west flash -d build-post-i2c`. Connect the serial console before reset to
capture boot POST results.

See the [adaptation contracts](../modules/lib/ifx-zephyr-stl/zephyr-adaptation-layer/README.rst)
for test coverage and resource ownership, and the
[asset-maintenance guide](../modules/lib/ifx-zephyr-stl/scripts/README.md) for
vendor release updates. Cross-repository links assume the workspace layout above.

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

### lvgl-7in*
A clone of `samples/subsys/display/lvgl` with a board overlay for the PSE84 Eval Kit's 7″
Waveshare display module instead of the standard 4.3″ module.
```bat
west build -p -b kit_pse84_eval/pse846gps2dbzc4a/m55 ifx-zephyr-sdk\samples\lvgl-7in --sysbuild
west flash
```

### ml-middleware*
Demonstrates running a [DEEPCRAFT Studio](https://imagimob.com/deepcraft) model on Zephyr using
the Infineon ML Middleware, targeting both the M33 + NNLite and M55 + Ethos-U55 configurations.
See [samples/ml-middleware/README.md](samples/ml-middleware/README.md) for full build and usage
details.

### * - Currently not fully supported on the main branch of ifx-zephyr-sdk

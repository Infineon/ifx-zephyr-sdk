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
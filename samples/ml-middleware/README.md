# PSE84 ML Middleware Example

Demonstrates running a [DEEPCRAFT Studio](https://imagimob.com/deepcraft) model on
Zephyr using the Infineon ML Middleware as the integration layer.  The same application
code and MTB-style API calls (`IMAI_init`, `IMAI_enqueue`, `IMAI_dequeue`) compile for
either of two hardware targets:

| Target | Board | NPU | Overlay |
|---|---|---|---|
| Cortex-M33 + NNLite | `kit_pse84_eval/pse846gps2dbzc4a/m33` | NNLite | `overlay-nnlite.conf` |
| Cortex-M55 + Ethos-U55 | `kit_pse84_ai/pse846gps2dbzc4a/m55` | ARM Ethos-U55 | `overlay-m55-imai.conf` |

---


## West workspace

`ifx-zephyr-sdk` is the west manifest repo for the full Infineon Zephyr downstream workspace.
This sample is one of several delivered under `ifx-zephyr-sdk/samples/`.  See the root repo
readme for full setup instructions.

| Module | Why it is here |
|---|---|
| `ml-middleware` | Infineon-internal; provides the MTB-style `mtb_ml_*` API and NNLite / Ethos-U55 backend glue |
| `ml-tflite-micro` | Infineon-internal; precompiled `libtensorflow-microlite.a` variants per NPU target |

The resulting layout will be:

```
my-workspace/
  ifx-zephyr-sdk/          ← this repo (west manifest); samples live under samples/
  ifx-zephyr/              ← Zephyr kernel + PSE84 board support
  modules/
    hal/infineon/
      ml-middleware/       ← MTB-style ML Middleware
      ml-tflite-micro/     ← precompiled TFLite Micro libraries
      ...                  ← other hal_infineon modules (from Zephyr import)
    ...
```

---

## Building

All build commands are run from the workspace root (the `my-workspace/` directory that
contains `ifx-zephyr-sdk/`).

### M33 + NNLite (PSE84 Eval Kit)

```bash
west build --pristine -b kit_pse84_eval/pse846gps2dbzc4a/m33 ifx-zephyr-sdk/samples/ml-middleware -- -DOVERLAY_CONFIG="overlay-nnlite.conf"
```

### M55 + Ethos-U55 (PSE84 Eval Kit)

```bash
west build --pristine -b kit_pse84_eval/pse846gps2dbzc4a/m55 --sysbuild ifx-zephyr-sdk/samples/ml-middleware -- -DOVERLAY_CONFIG="overlay-m55-imai.conf"
```

### Flashing

```bash
west flash
```

---

## How the ML Middleware integration works

The application uses the **MTB-style ML Middleware API**, the same API used in
ModusToolbox projects.  There is no Zephyr-specific ML API to learn.

```
DEEPCRAFT model.c  (IMAI_init / IMAI_enqueue / IMAI_dequeue)
        │
        ▼
Infineon ML Middleware  (mtb_ml_model_init / mtb_ml_model_run)
        │
        ├── M33 path → NNLite NPU driver
        └── M55 path → TFLite Micro + Ethos-U delegate
```

`CMakeLists.txt` selects the correct `models/` subdirectory at configure time
based on `CONFIG_CPU_CORTEX_M33` or `CONFIG_CPU_CORTEX_M55`:

```
models/
  nnlite/    ← compiled into M33 builds  (model.c + model.h)
  ethosu55/  ← compiled into M55 builds  (model.c + model.h)
```

`src/main.c` uses only the IMAI API and is identical for both targets:

```c
IMAI_init();                      // Init middleware + interpreter + NPU
IMAI_enqueue(input_float_array);  // Feed one input frame
IMAI_dequeue(output_float_array); // Run inference, read output
IMAI_finalize();                  // Release model resources
```

---

## Replacing the model with your own

DEEPCRAFT Studio exports a pair of files — `model.c` and `model.h` — for each
compiled model variant.  To swap in your own model:

1. **Export from DEEPCRAFT Studio** — select the target NPU (NNLite or Ethos-U55)
   and export "IMAI C source".  This gives you `model.c` and `model.h`.

2. **Drop the files into the correct subdirectory:**

   ```
   models/nnlite/model.c      ← NNLite (M33) variant
   models/nnlite/model.h
   models/ethosu55/model.c    ← Ethos-U55 (M55) variant
   models/ethosu55/model.h
   ```

   You only need to populate the subdirectory for the target you are building.

3. **Check the IMAI constants in `model.h`** — `main.c` reads these at compile
   time so it adapts automatically to your model's input/output dimensions:

   | Constant | Meaning |
   |---|---|
   | `IMAI_DATA_IN_COUNT` | Number of `float` input elements |
   | `IMAI_DATA_OUT_COUNT` | Number of `float` output classes |
   | `IMAI_DATA_OUT_SYMBOLS` | String array of class label names |
   | `IMAI_MODEL_ID` | 16-byte GUID identifying the model |

4. **Update `src/main.c`** — change the input synthesis (the `memset` loop) to
   feed real data from your sensor or data source.  Everything else — init,
   enqueue, dequeue, logging — adapts automatically via the constants above.

5. **Rebuild:**

   ```bash
   west build --pristine -b kit_pse84_eval/pse846gps2dbzc4a/m33 \
     ifx-zephyr-sdk/samples/ml-middleware -- -DOVERLAY_CONFIG="overlay-nnlite.conf"
   ```

### Memory sizing

If your model is larger than the example, adjust these values in the overlay:

```kconfig
CONFIG_HEAP_MEM_POOL_SIZE=16384   # heap for mtb_ml_model_t + C++ objects
CONFIG_MAIN_STACK_SIZE=8192
```

The `model.h` header reports the required RAM (`Buffers` + `State`) and Flash
(`Readonly`) at the top of the file — use these numbers to guide sizing.

---

## Project structure

This sample is located at `ifx-zephyr-sdk/samples/ml-middleware/` within the west workspace.

```
ifx-zephyr-sdk/samples/ml-middleware/
  CMakeLists.txt          — build definition; model dir selection
  Kconfig                 — sample-specific Kconfig symbols
  prj.conf                — base project config
  overlay-nnlite.conf     — M33 + NNLite overlay
  overlay-m55-imai.conf   — M55 + Ethos-U55 overlay
  src/
    main.c                — application (IMAI API calls)
    cy_retarget_io.h      — printf stub (Zephyr console used directly)
  models/
    nnlite/               — NNLite model files (model.c, model.h)
    ethosu55/             — Ethos-U55 model files (model.c, model.h)
```

---

## License

SPDX-License-Identifier: Apache-2.0

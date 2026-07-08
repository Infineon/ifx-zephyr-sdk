# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# Register the Infineon ML modules explicitly so that Zephyr's CMake sources
# their module.yml / Kconfig files regardless of which branch zephyr/west.yml
# is on.  ZEPHYR_EXTRA_MODULES must be set before find_package(Zephyr).
#
# Note: The modules are already registered in the SDK's west.yml manifest
# at modules/lib/ml-middleware and modules/lib/ml-tflite-micro, so they
# should be automatically available via west's module discovery.

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(kit_pse84_ai_ml_demo)

# Select the DEEPCRAFT model based on target core:
#   Cortex-M55 → models/ethosu55/  (Vela-compiled for Ethos-U55 NPU)
#   Cortex-M33 → models/nnlite/    (compiled for NNLite NPU)
#
# CONFIG_CPU_CORTEX_M55 / CONFIG_CPU_CORTEX_M33 are set by the board Kconfig
# and are available to CMake via the Zephyr Kconfig cache.
if(CONFIG_CPU_CORTEX_M55)
  set(MODEL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/models/ethosu55)
  message(STATUS "ML model: ethosu55 (Cortex-M55 / Ethos-U55)")
elseif(CONFIG_CPU_CORTEX_M33)
  set(MODEL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/models/nnlite)
  message(STATUS "ML model: nnlite (Cortex-M33 / NNLite NPU)")
else()
  message(FATAL_ERROR "No model directory defined for this core. "
    "Add a models/<core>/ directory and extend the selection above.")
endif()

# Application source — model.c implements IMAI_init/enqueue/dequeue via the
# Infineon ML Middleware (mtb_ml_model_init / mtb_ml_run_inference internally).
target_sources(app PRIVATE
  src/main.c
  ${MODEL_DIR}/model.c
)

# model.c is generated code (ImagiNet Compiler) and does not include <stdio.h>
# despite calling printf.  Force-include it for that file only so the
# generated source is never modified.
set_source_files_properties(${MODEL_DIR}/model.c PROPERTIES
  COMPILE_OPTIONS "-include;stdio.h"
)

# Include directories:
#   src/        — cy_retarget_io.h stub (printf already works via Zephyr console)
#   MODEL_DIR   — model.h with IMAI API declarations and constants
target_include_directories(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${MODEL_DIR}
)

# model.c compile-time guards (checked via #error if missing):
#   COMPONENT_ML_TFLM   — provided by CONFIG_ML_MIDDLEWARE_TFLITE_INTERPRETER=y
#   COMPONENT_ML_INT8x8 — provided by CONFIG_ML_MIDDLEWARE_INT8=y
# Both are set as zephyr_compile_definitions by ml-middleware/zephyr/CMakeLists.txt
# and propagate to all targets including app. Listed here for documentation only.

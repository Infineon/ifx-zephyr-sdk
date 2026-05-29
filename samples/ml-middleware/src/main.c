/*
 * Copyright (c) 2026 Infineon Technologies AG
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file main.c
 * @brief DEEPCRAFT ML Model Integration via Infineon ML Middleware on Zephyr
 *
 * Demonstrates how a DEEPCRAFT Studio-generated model.c/.h integrates with
 * Zephyr RTOS using the Infineon ML Middleware as the integration layer.
 *
 * The DEEPCRAFT IMAI API encapsulates the full inference pipeline:
 *   IMAI_init()     — initialises ML middleware + TFLite interpreter + NPU
 *   IMAI_enqueue()  — feeds input data
 *   IMAI_dequeue()  — retrieves output
 *
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <string.h>

/* DEEPCRAFT-generated IMAI API — implemented in model.c.
 * model.h also exposes: IMAI_DATA_IN_COUNT (128), IMAI_DATA_OUT_COUNT (3),
 * IMAI_RET_SUCCESS, IMAI_RET_NODATA, IMAI_RET_ERROR */
#include "model.h"

/* Infineon ML Middleware API for lifecycle verification */
#include "mtb_ml.h"
/* Zephyr-specific ML Middleware helpers — defines mtb_ml_get_npu_type() */
#include "mtb_ml_zephyr.h"

LOG_MODULE_REGISTER(ml_imai_demo, LOG_LEVEL_INF);

/* Class labels come directly from model.h via IMAI_DATA_OUT_SYMBOLS.
 * This macro expands to a brace-enclosed string list, so we use it to
 * initialise the array — no hardcoded strings in main.c. */
static const char *const class_labels[] = IMAI_DATA_OUT_SYMBOLS;

/* Model GUID bytes from model.h — printed at startup for traceability. */
static const uint8_t model_id[] = IMAI_MODEL_ID;

int main(void)
{
	int ret;
	float input[IMAI_DATA_IN_COUNT];   /* dummy input frame     */
	float output[IMAI_DATA_OUT_COUNT]; /* class probabilities  */

	printk("\n=== DEEPCRAFT ML Middleware Demo ===\n");
	printk("NPU: %s\n", mtb_ml_get_npu_type());
	/* Model GUID from model.h — uniquely identifies which DEEPCRAFT model
	 * was compiled in; no hardcoded name needed. */
	printk("Model ID: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
	       model_id[0], model_id[1], model_id[2], model_id[3],
	       model_id[4], model_id[5], model_id[6], model_id[7],
	       model_id[8], model_id[9], model_id[10], model_id[11],
	       model_id[12], model_id[13], model_id[14], model_id[15]);
	printk("Input:   float[%d]\n", IMAI_DATA_IN_COUNT);
	printk("Output:  float[%d] classes:", IMAI_DATA_OUT_COUNT);
	for (int i = 0; i < IMAI_DATA_OUT_COUNT; i++) {
		printk(" %s", class_labels[i]);
	}
	printk("\n======================================\n\n");

	/* --- Step 1: Initialise the model via Infineon ML Middleware ---
	 * Internally calls:
	 *   mtb_ml_model_init()  — sets up TFLite interpreter with AllOpsResolver
	 *                          (includes EthosuDelegate for Vela-compiled ops)
	 *   mtb_ml_init()        — lightweight init; hardware already set up by
	 *                          Zephyr board driver (CONFIG_ETHOS_U_INFINEON) */
	LOG_INF("Initialising DEEPCRAFT model via Infineon ML Middleware...");
	ret = IMAI_init();
	if (ret != IMAI_RET_SUCCESS) {
		LOG_ERR("IMAI_init() failed: %d", ret);
		printk("FATAL: Model initialisation failed!\n");
		return -EIO;
	}
	LOG_INF("Model initialised successfully");

	/* --- Step 1b: Verify middleware init state via mtb_ml_get_init_state() ---
	 * mtb_ml_get_init_state() returns the internal reference count incremented
	 * by each call to mtb_ml_init(). IMAI_init() calls mtb_ml_init() once,
	 * so the count should be 1. */
	uint32_t init_state = mtb_ml_get_init_state();

	printk("mtb_ml_get_init_state() = %u (expected 1)\n", init_state);
	if (init_state != 1U) {
		LOG_WRN("Unexpected init state: %u", init_state);
	} else {
		LOG_INF("ML Middleware init state verified: %u", init_state);
	}

	/* --- Step 2: Run a simple inference loop ---
	 * Feed zero-valued input frames (simulated radar data) to demonstrate
	 * the pipeline.  Real usage would feed actual ADC/radar samples. */
	printk("Running inference loop (5 frames of zero-valued input)...\n");

	for (int frame = 0; frame < 5; frame++) {
		/* Synthesise a simple test pattern: zero frame */
		memset(input, 0, sizeof(input));

		/* Enqueue one input frame (float[128]).
		 * Returns IMAI_RET_SUCCESS (0) or IMAI_RET_NODATA (-1) if the
		 * fixed-window buffer is not yet full enough. */
		ret = IMAI_enqueue(input);
		if (ret == IMAI_RET_NODATA) {
			/* Normal — fixed window accumulating frames */
			LOG_DBG("Frame %d: window filling (%d)", frame, ret);
			continue;
		}
		if (ret != IMAI_RET_SUCCESS) {
			LOG_ERR("IMAI_enqueue() failed: %d", ret);
			break;
		}

		/* Dequeue output — runs inference on buffered window.
		 * Returns IMAI_RET_SUCCESS when output is ready. */
		ret = IMAI_dequeue(output);
		if (ret == IMAI_RET_NODATA) {
			LOG_DBG("Frame %d: no output yet", frame);
			continue;
		}
		if (ret != IMAI_RET_SUCCESS) {
			LOG_ERR("IMAI_dequeue() failed: %d", ret);
			break;
		}

		/* Print classification results */
		printk("Frame %d — Inference result:\n", frame);
		int best_idx = 0;
		for (int i = 0; i < IMAI_DATA_OUT_COUNT; i++) {
			printk("  [%d] %-12s: %.4f\n", i, class_labels[i],
			       (double)output[i]);
			if (output[i] > output[best_idx]) {
				best_idx = i;
			}
		}
		printk("  >> Detected: %s\n\n", class_labels[best_idx]);
	}

	/* --- Step 3: Cleanup ---
	 * IMAI_finalize() releases the TFLite interpreter via mtb_model_free(). */
	IMAI_finalize();
	LOG_INF("Model finalised");

	/* --- Step 4: Deinit the ML Middleware ---
	 * mtb_ml_deinit() decrements the internal reference count.
	 * After this call, mtb_ml_get_init_state() should return 0. */
	cy_rslt_t deinit_result = mtb_ml_deinit();

	if (deinit_result != CY_RSLT_SUCCESS) {
		LOG_WRN("mtb_ml_deinit() returned: 0x%08x", deinit_result);
	}
	init_state = mtb_ml_get_init_state();
	printk("mtb_ml_get_init_state() after deinit = %u (expected 0)\n", init_state);
	LOG_INF("ML Middleware deinit state: %u", init_state);

	printk("======================================================\n");
	printk("DEEPCRAFT ML Middleware demo complete.\n");

	return 0;
}

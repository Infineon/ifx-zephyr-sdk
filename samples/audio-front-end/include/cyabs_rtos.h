/*
 * Copyright (c) 2026 Infineon Technologies AG,
 * or an affiliate of Infineon Technologies AG.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file cyabs_rtos.h
 * @brief Minimal RTOS abstraction compatibility for AFE sample
 */

#ifndef CYABS_RTOS_H_
#define CYABS_RTOS_H_

#include <zephyr/kernel.h>
#include "cy_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/* RTOS types mapped to Zephyr equivalents */
typedef struct k_thread cy_thread_t;
typedef struct k_mutex cy_mutex_t;

/* Include configurator settings only if not already included */
#ifndef CY_AFE_ENABLE_TUNING_FEATURE
#if __has_include("cy_afe_configurator_settings.h")
#include "cy_afe_configurator_settings.h"
#elif __has_include("generated/cy_afe_configurator_settings.h") 
#include "generated/cy_afe_configurator_settings.h"
#elif __has_include("../zephyr/include/cy_afe_configurator_settings.h")
#include "../zephyr/include/cy_afe_configurator_settings.h"
#endif
#endif

/* Provide stub definitions only when tuning is disabled */
#if !CY_AFE_ENABLE_TUNING_FEATURE
/* Note: When tuning is disabled but AFE headers are included, the real types are still defined
 * in cy_audio_front_end.h within conditional blocks. We should not redefine them here.
 * This section is only for cases where AFE headers are not available. */
#endif

/* Result codes */
#define CY_RTOS_SUCCESS                (CY_RSLT_SUCCESS)
#define CY_RTOS_GENERAL_ERROR          (CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, 1))

/* Timeout definitions */
#define CY_RTOS_NEVER_TIMEOUT          K_FOREVER

/* Function stubs */
static inline cy_rslt_t cy_rtos_init_mutex(cy_mutex_t* mutex) { 
    return k_mutex_init(mutex) == 0 ? CY_RTOS_SUCCESS : CY_RTOS_GENERAL_ERROR; 
}

static inline cy_rslt_t cy_rtos_get_mutex(cy_mutex_t* mutex, uint32_t timeout_ms) { 
    return k_mutex_lock(mutex, K_MSEC(timeout_ms)) == 0 ? CY_RTOS_SUCCESS : CY_RTOS_GENERAL_ERROR; 
}

static inline cy_rslt_t cy_rtos_set_mutex(cy_mutex_t* mutex) { 
    return k_mutex_unlock(mutex) == 0 ? CY_RTOS_SUCCESS : CY_RTOS_GENERAL_ERROR; 
}

static inline cy_rslt_t cy_rtos_deinit_mutex(cy_mutex_t* mutex) { 
    (void)mutex; return CY_RTOS_SUCCESS; 
}

#ifdef __cplusplus
} /*extern "C" */
#endif

#endif /* CYABS_RTOS_H_ */
/*
 * Copyright 2025, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/**
 * @file cy_afe_tuner_task.h
 * @brief Set of routines to setup audio tuner task & queue
 *
 */

#ifndef AUDIO_FRONT_END_TUNER_TASK_H__
#define AUDIO_FRONT_END_TUNER_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
#include "cy_afe_audio_internal.h"
/******************************************************
 *                     Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/**
 * Setup audio tuner task and queue
 *
 * @param[in]  context      Audio front end middleware handle
 * @param[in]  config       Audio front end configuration
 */
cy_rslt_t afe_setup_audio_tuner_task(afe_internal_context_t *context, cy_afe_config_t *config);

/**
 * Cleanup audio tuner task and queue
 *
 * @param[in]  context      Audio front end middleware handle
 */
cy_rslt_t afe_cleanup_audio_tuner_task(afe_internal_context_t *context);

/**
 * Allocate memory for AFE debug output
 *
 * @param[in]  context      Audio front end middleware handle
 */
cy_rslt_t afe_allocate_memory_for_dbg_output(afe_internal_context_t* context);

/**
 * Cleanup/free memory for AFE debug output
 *
 * @param[in]  context      Audio front end middleware handle
 */
cy_rslt_t afe_cleanup_memory_for_dbg_output(afe_internal_context_t* context);

#endif

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_FRONT_END_TUNER_TASK_H__ */

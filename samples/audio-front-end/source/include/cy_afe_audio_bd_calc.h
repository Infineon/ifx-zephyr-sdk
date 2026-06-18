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
 * @file cy_afe_audio_speech_enh.h
 * @brief APIs which are used to interact with system component from AFE middleware
 *
 */

#ifndef AFE_BD_CALC_H__
#define AFE_BD_CALC_H__

#ifdef CY_AFE_ENABLE_TUNING_FEATURE

#include "cy_result.h"
#include <stdbool.h>

#include "cy_afe_audio_internal.h"
#include "cyabs_rtos_impl.h"
#include "cyabs_rtos.h"
#include "cy_audio_front_end.h"
#include "bulk_delay_measurement.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
 *                     Macros
 ******************************************************/

#ifndef BULK_DELAY_ERROR
#define BULK_DELAY_ERROR (-1)
#endif

#ifndef BULK_DELAY_IDENTIFY_MAX_REPEAT_COUNT
#define BULK_DELAY_IDENTIFY_MAX_REPEAT_COUNT (100)
#endif

#ifndef BDM_CALCULATION_WAIT_TIMEOUT
#define BDM_CALCULATION_WAIT_TIMEOUT (60*1000)
#endif

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

cy_rslt_t cy_afe_bd_calc_process(afe_internal_context_t *context,
        afe_sp_enh_input_output_t *sp_enh_input_output);

cy_rslt_t cy_afe_bd_calc_wait_for_complete(unsigned int ui_timout_ms,
        int *bulk_delay);

#endif

#ifdef __cplusplus
}
#endif

#endif /* AFE_BD_CALC_H__ */

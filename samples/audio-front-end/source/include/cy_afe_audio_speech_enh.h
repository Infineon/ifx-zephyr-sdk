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

#ifndef AUDIO_FRONT_END_SPEECH_ENHANCEMENT_H__
#define AUDIO_FRONT_END_SPEECH_ENHANCEMENT_H__

#include "cy_result.h"
#include <stdbool.h>

#include "cy_afe_audio_internal.h"
#include "cyabs_rtos_impl.h"
#include "cyabs_rtos.h"
#include "cy_audio_front_end.h"
#include "ifx_sp_enh.h"
#include "cy_afe_types_compat.h"

#ifdef __cplusplus
extern "C"
{
#endif

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
typedef struct
{
    /*
     * Pointer to audio data from mic1
     */
    CY_AFE_DATA_T *input1;

    /*
     * Pointer to audio data from mic2
     */
    CY_AFE_DATA_T *input2;

    /*
     * Pointer to AEC reference data
     */
    CY_AFE_DATA_T *aec_reference_input;

    /*
     * Pointer to AEC reference data
     */
    CY_AFE_DATA_T *output;

    /*
     * Pointer to internal output used for IFX wake word detection
     */
    CY_AFE_DATA_T *ifx_internal_output;

} afe_sp_enh_input_output_t;

typedef struct
{
    /* Data pointer container
     *
     */
    void* data_point_container;

    ifx_stc_sp_enh_info_t sp_enh_info;

    char* persistent_memory;
    char* scratch_memory;
} ifx_sp_enh_context_t;
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
 * Initialize the system audio front end APIs for speech enhancement
 *
 * @param[in] filter_settings       filter settings format generated
 * @param[in] mw_settings           Middleware settings format generated
 * @param[in] mw_settings_length    Middleware settings length
 * @param[in] context               Pointer to internal AFE context
 * @return cy_rslt_t
 */
cy_rslt_t afe_speech_enhancement_init(int32_t *filter_settings,uint8_t *mw_settings,
                                    uint32_t mw_settings_length, void **context);

/**
 * Process the incoming audio data and generate the output
 *
 * @param[in] context               Pointer to internal AFE context
 * @param[in] sp_enh_input_output   Pointer to speech enhancement input/output structure
 *
 * @return    cy_rslt_t
 */
cy_rslt_t afe_speech_enhancement_process(void *context, afe_sp_enh_input_output_t* sp_enh_input_output);

/**
 * Deinitialize afe speech enhancement
 *
 * @param[in] context               Pointer to internal AFE context
 * @param[in] sp_enh_input_output   Pointer to speech enhancement input/output structure
 *
 * @return    cy_rslt_t
 */
cy_rslt_t afe_speech_enhancement_deinit(void *context);

/**
 * Enable/disable component
 *
 * @param[in] context              Pointer to internal AFE context
 * @param[in] component_name       component name
 * @param[in] enable               set value to true if want to enable component else set it to false
 *
 * @return    int
 */
cy_rslt_t afe_speech_enhancement_enable_disable_component(void *context, ifx_sp_enh_ip_component_config_t component_name, bool enable);

/**
 * Update configuration value
 *
 * @param[in] context              Pointer to internal AFE context
 * @param[in] config_name          configuration name
 * @param[in] value                Value to be updated
 *
 * @return    int
 */
cy_rslt_t afe_speech_enhancement_update_config_value(void *context, ifx_sp_enh_ip_component_config_t config_name, int32_t value);

/**
 * Get configuration value
 *
 * @param context                  Pointer to internal AFE context
 * @param component_name           configuration name
 * @param value                    Value to get
 * @return cy_rslt_t
 */
cy_rslt_t afe_speech_enhancement_get_config_value(void *context, ifx_sp_enh_ip_component_config_t component_name, int32_t* value);

/**
 * Get component status
 *
 * @param context               Pointer to internal AFE context
 * @param component_name        configuration name
 * @param enable                Value to get
 * @return cy_rslt_t
 */
cy_rslt_t afe_speech_enhancement_get_component_status(void *context, ifx_sp_enh_ip_component_config_t component_name, bool* enable);

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
cy_rslt_t afe_speech_enhancement_update_dbg_out_config(void *sp_enh_context,afe_usb_settings_t *pMY_AFE_USB_SETTINGS);

cy_rslt_t afe_speech_enhancement_get_sound_meter(void *context, int16_t *audio_meter);

cy_rslt_t afe_speech_enhancement_get_component_params(void *context, ifx_sp_enh_ip_component_config_t component_name,
                                                        int32_t* value, int32_t* value_count);
#endif /* CY_AFE_ENABLE_TUNING_FEATURE */

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_FRONT_END_SPEECH_ENHANCEMENT_H__ */

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
 * @file cy_afe.c
 * @brief Implementation of AFE APIs provided in cy_audio_front_end.h header file.
 *
 */

#include "cy_audio_front_end.h"
#include "cy_afe_audio_internal.h"
#include "cy_afe_audio_speech_enh.h"
#include "cy_afe_audio_task.h"
#ifdef CY_AFE_ENABLE_TUNING_FEATURE
#include "cy_afe_tuner_task.h"
#endif
#include "cyabs_rtos_internal.h"
#include "cy_afe_license_compat.h"

/******************************************************
 *                     Macros
 ******************************************************/
#if CY_AFE_FRAME_SIZE_MS != 10 || CY_AFE_SAMPLE_FREQ != 16000 || AFE_INPUT_NUMBER_CHANNELS > 2 || AFE_INPUT_NUMBER_CHANNELS == 0
#error "Invalid configuration for AFE middleware. Supported values are:[frame_size:10ms,sample_freq:16K,number_channel:mono/stereo]"
#endif

#ifndef ENABLE_AFE_MW_SUPPORT
#error "macro ENABLE_AFE_MW_SUPPORT to be defined"
#endif
/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/
/* As AFE supports only single instance, global_handle is stored/initialized when
 * cy_afe_create API is called first time, later any call of cy_afe_create will
 * return error.
 */
cy_afe_t global_handle = NULL;

cy_afe_alloc_memory_callback_t afe_alloc_memory = NULL;

cy_afe_free_memory_callback_t  afe_free_memory = NULL;

/******************************************************
 *               Static Functions
 ******************************************************/

/******************************************************
 *               Functions
 ******************************************************/
/**
 * Default internal callback for getting output buffer if application has not registered callback.
 */
cy_rslt_t cy_afe_default_internal_get_output_buf_callback(cy_afe_t afe_handle, uint32_t **output_buf, void* user_args)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    afe_internal_context_t* handle = (afe_internal_context_t*) afe_handle;

    if (NULL == handle || NULL == output_buf)
    {
        result = CY_RSLT_AFE_BAD_ARG;
        cy_afe_log_err(result,
                "Invalid argument passed. handle:[%p], output_buf:[%p]",
                handle, output_buf);
        return result;
    }

    /* Allocate AFE output buffer */
    if (NULL == handle->internal_output_buffer)
    {
        if(afe_alloc_memory)
        {
            afe_alloc_memory(CY_AFE_MEM_ID_AFE_OUTPUT_BUFFER, CY_AFE_MONO_FRAME_SIZE_IN_BYTES,
                    (void **)&handle->internal_output_buffer);
        }
        else
        {
            handle->internal_output_buffer = calloc(1, CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
        }
        if (NULL == handle->internal_output_buffer)
        {
            result = CY_RSLT_AFE_OUT_OF_MEMORY;
            cy_afe_log_err(result,
                    "failed to allocate memory for output buffer");
            return result;
        }
    }

    *output_buf = handle->internal_output_buffer;

    return result;
}

/**
 * Creates AFE instance, AFE processing thread, AFE queue & AFE tuner thread
 * & AFE tuner queue if tuner functionality is enabled
 */
cy_rslt_t cy_afe_create(cy_afe_config_t *config_init, cy_afe_t *handle)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if (NULL != global_handle)
    {
        result = CY_RSLT_AFE_ALREADY_INITIALIZED;
        cy_afe_log_err(result, "AFE context is already initialized");
        return result;
    }

    /* Check for null parameters */
    if (NULL == config_init || NULL == handle)
    {
        result = CY_RSLT_AFE_BAD_ARG;
        cy_afe_log_err(result,
                "Invalid argument passed. config_init:[%p], handle:[%p]",
                config_init, handle);
        return result;
    }

    /* Check for AFE output callback */
    if (NULL == config_init->afe_output_callback)
    {
        result = CY_RSLT_AFE_BAD_ARG;
        cy_afe_log_err(result, "AFE output cb cannot be passed as NULL");
        return result;
    }

    if (NULL == config_init->filter_settings)
    {
        result = CY_RSLT_AFE_BAD_ARG;
        cy_afe_log_err(result, "AFE filter settings cannot be passed as NULL");
        return result;
    }

    afe_alloc_memory = config_init->alloc_memory;
    afe_free_memory = config_init->free_memory;

    /* Debug: Check values immediately after assignment */
    cy_afe_log_err(0, "DEBUG: After assignment - afe_alloc_memory: %p, afe_free_memory: %p", 
                   afe_alloc_memory, afe_free_memory);
    cy_afe_log_err(0, "DEBUG: Original config values - alloc: %p, free: %p", 
                   config_init->alloc_memory, config_init->free_memory);

    if( (NULL == afe_alloc_memory && NULL != afe_free_memory)  ||
        (NULL != afe_alloc_memory && NULL == afe_free_memory))
    {
        result = CY_RSLT_AFE_BAD_ARG;
        cy_afe_log_err(result, "afe_alloc_memory and afe_free_memory both needs to be valid");
        cy_afe_log_err(result, "DETAILED: afe_alloc_memory=%p, afe_free_memory=%p", 
                       afe_alloc_memory, afe_free_memory);
        return result;
    }

    cy_afe_log_dbg(
            "config params:[no_of_channels:%d, frame_size:%d, sample_freq:%d,afe_output_cb:%p,get_buffer_cb:%p]",
            CY_AFE_INPUT_NUM_OF_CHANNELS, CY_AFE_FRAME_SIZE_MS,
            CY_AFE_SAMPLE_FREQ, config_init->afe_output_callback,
            config_init->afe_get_buffer_callback);

    /* Set the AFE context to NULL */
    *handle = NULL;

    /* Allocate memory for the internal AFE context */
    afe_internal_context_t *context = NULL;
    if(afe_alloc_memory)
    {
        afe_alloc_memory(CY_AFE_MEM_ID_AFE_CONTEXT, sizeof(afe_internal_context_t),
                (void **)&context);
    }
    else
    {
        context = calloc(1, sizeof(afe_internal_context_t));
    }
    if (NULL == context)
    {
        result = CY_RSLT_AFE_OUT_OF_MEMORY;
        cy_afe_log_err(result, "Unable to allocate memory for AFE context");
        return result;
    }



    /*
     * Setup the audio processing task and queue
     */
    result = afe_setup_audio_processing_task(context);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "Failed to setup the audio processing task");
        result = CY_RSLT_AFE_GENERIC_ERROR;
        goto CLEAN_RETURN;
    }

#ifdef CY_AFE_ENABLE_TUNING_FEATURE


    context->tuner_callbacks.notify_settings_callback = config_init->tuner_cb.notify_settings_callback;
    context->tuner_callbacks.read_request_callback = config_init->tuner_cb.read_request_callback;
    context->tuner_callbacks.write_response_callback = config_init->tuner_cb.write_response_callback;

    /*
     * Setup the audio tuner task to configure tuner settings
     */
    result = afe_setup_audio_tuner_task(context, config_init);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "Failed to setup the audio tuner task");
        result = CY_RSLT_AFE_GENERIC_ERROR;
        goto CLEAN_RETURN;
    }

    result = afe_allocate_memory_for_dbg_output(context);
    if(CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "Failed to allocate memory for AFE debug output");
        goto CLEAN_RETURN;
    }

#endif

    /* Initialize AFE system components for speech enhancement */
    result = afe_speech_enhancement_init((int32_t *)config_init->filter_settings, config_init->mw_settings,
                                        config_init->mw_settings_length, &context->sp_enh_context);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "Failed to initialize speech enhancement");
        goto CLEAN_RETURN;
    }

    /* Use application provided function for getting AFE output buffer
     * else use the internal function to allocate static memory */
    if(NULL == config_init->afe_get_buffer_callback)
    {
        context->get_output_buffer_cb = cy_afe_default_internal_get_output_buf_callback;
    }
    else
    {
        context->get_output_buffer_cb = config_init->afe_get_buffer_callback;
    }

    context->config_init = *config_init;

    if(afe_alloc_memory)
    {
        afe_alloc_memory(CY_AFE_MEM_ID_AFE_DBG_OUT_BUFFER, CY_AFE_MONITOR_OUT_MAX_SIZE,
                (void *)&context->ifx_internal_output);
    }
    else
    {
        context->ifx_internal_output = (uint16_t *) malloc(CY_AFE_MONITOR_OUT_MAX_SIZE);
    }

    if(NULL == context->ifx_internal_output)
    {
         cy_afe_log_err(result, "Out of mem sz: %d",CY_AFE_MONITOR_OUT_MAX_SIZE);
         goto CLEAN_RETURN;
    }

    *handle = context;
    global_handle = (void*) context;

    cy_afe_log_info("cy_afe_create success");

    return CY_RSLT_SUCCESS;

CLEAN_RETURN:
    cy_afe_delete((cy_afe_t*) &context);
    return result;
}

/**
 * Feed audio input data
 */
cy_rslt_t cy_afe_feed(cy_afe_t handle, CY_AFE_DATA_T *input_buffer, CY_AFE_DATA_T *aec_ref_buf)
{
    afe_internal_context_t *context = (afe_internal_context_t*) handle;
    cy_rslt_t result = CY_RSLT_SUCCESS;

#ifdef ENABLE_AFE_APP_CHECK_POINT
    AFE_APP_CHECK_POINT()
#endif

    /* Check for afe handle & input audio data pointer, aec_ref_buf pointer can be passed NULL when
     * AEC is not enabled or if aec is enabled, but dont want to perform AEC on these frames
     */
    if (NULL == context || NULL == input_buffer)
    {
        result = CY_RSLT_AFE_BAD_ARG;
        cy_afe_log_err_on_no_isr(result,
                "Invalid argument passed. context:[%p], input_buffer:[%p]",
                context, input_buffer);
        return result;
    }

    if (global_handle != context)
    {
        result = CY_RSLT_AFE_BAD_ARG;
        cy_afe_log_err_on_no_isr(result, "Invalid handle passed. handle:[%p]",
                handle);
        return result;
    }

    /* Check for the license expiry of AFE audio library */
    if(cy_afe_lib_is_license_expired())
    {
        result = CY_RSLT_AFE_FUNCTIONALITY_RESTRICTED;
        cy_afe_log_err_on_no_isr(result, "AFE library license is expired.");
        return result;
    }

    result = afe_push_audio_data_to_queue(context, input_buffer, aec_ref_buf);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err_on_no_isr(result,
                "Push to audio processing queue failed");
        return result;
    }

#ifdef ENABLE_AFE_APP_CHECK_POINT
    AFE_APP_CHECK_POINT()
#endif

    return result;
}

/**
 * Cleanup the AFE thread, queue and deletes instance
 */
cy_rslt_t cy_afe_delete(cy_afe_t *handle)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    afe_internal_context_t *context = (afe_internal_context_t*) *handle;

    if (NULL == context)
    {
        result = CY_RSLT_AFE_BAD_ARG;
        cy_afe_log_err(result, "Invalid argument passed. handle : [%p]",
                context);
        return result;
    }

    if (context != global_handle)
    {
        result = CY_RSLT_AFE_BAD_ARG;
        cy_afe_log_err(result, "Invalid handle passed. handle : [%p]", handle);
        return result;
    }

    /*
     * Cleanup the audio processing task and resources
     */
    if (true == context->audio_processing_thread_running)
    {
        result = afe_cleanup_audio_processing_task(context);
        if (CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to cleanup the audio processing task resources");
            return result;
        }
    }

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
    /*
     * Cleanup the audio tuner task and resources
     */
    if (true == context->audio_tuner_thread_running)
    {
        result = afe_cleanup_audio_tuner_task(context);
        if (CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to cleanup the audio tuner task resources");
            return result;
        }
    }

    afe_cleanup_memory_for_dbg_output(context);

#endif

    /* Free if memory allocated internally by AFE output callback */
    if (NULL != context->internal_output_buffer)
    {
        if(afe_free_memory && (context->config_init.afe_get_buffer_callback == NULL))
        {
            afe_free_memory(CY_AFE_MEM_ID_AFE_OUTPUT_BUFFER,context->internal_output_buffer);
        }
        else if (context->config_init.afe_get_buffer_callback == NULL)
        {
            free(context->internal_output_buffer );
        }
    }

    if(NULL != context->ifx_internal_output)
    {
        if(afe_free_memory)
        {
            afe_free_memory(CY_AFE_MEM_ID_AFE_DBG_OUT_BUFFER,context->ifx_internal_output);
        }
        else
        {
            free(context->ifx_internal_output);
        }
    }

    afe_speech_enhancement_deinit(context->sp_enh_context);

    if(afe_free_memory)
    {
        afe_free_memory(CY_AFE_MEM_ID_AFE_CONTEXT,context);
    }
    else
    {
        free(context);
    }

    afe_alloc_memory = NULL;
    afe_free_memory = NULL;
    *handle = NULL;
    global_handle = NULL;

    cy_afe_log_info("afe_delete success");

    return result;
}

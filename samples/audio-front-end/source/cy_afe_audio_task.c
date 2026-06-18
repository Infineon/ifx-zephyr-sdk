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
 * @file cy_afe_audio_task.c
 * @brief Set of routines to setup a audio processing task and queue
 *
 */

#include "cy_audio_front_end.h"
#include "cy_afe_audio_internal.h"
#include "cy_afe_audio_speech_enh.h"
#include "cyabs_rtos.h"
#include "cy_afe_audio_task.h"
#include "cy_afe_audio_process.h"
#include "cyabs_rtos_internal.h"
/******************************************************
 *                     Macros
 ******************************************************/
#define AFE_AUDIO_PROCESSING_TASK_NAME                   "afe-task"
/* TODO: Find out the optimum size of AFE thread stack size */
#ifndef AFE_AUDIO_PROCESSING_TASK_THREAD_STACK_SIZE
#define AFE_AUDIO_PROCESSING_TASK_THREAD_STACK_SIZE      (10*1024)
#endif

#ifndef AFE_AUDIO_PROCESSING_TASK_PRIORITY
#define AFE_AUDIO_PROCESSING_TASK_PRIORITY               (CY_RTOS_PRIORITY_NORMAL)
#endif

#ifndef AFE_AUDIO_PROCESSING_MSG_QUEUE_SIZE
#define AFE_AUDIO_PROCESSING_MSG_QUEUE_SIZE              (30)
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

/******************************************************
 *               Static Functions
 ******************************************************/

static void afe_audio_processing_task(cy_thread_arg_t arg);
static cy_rslt_t afe_pop_audio_data_from_queue(afe_internal_context_t *context);

/******************************************************
 *               Functions
 ******************************************************/
/**
 * Creates AFE processing queue & thread
 */
cy_rslt_t afe_setup_audio_processing_task(afe_internal_context_t *context)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    cy_afe_log_dbg("Initializing audio processing queue");

    /*
     * Initialize AFE audio processing queue
     */
    result = cy_rtos_init_queue(&context->audio_processing_queue,
            AFE_AUDIO_PROCESSING_MSG_QUEUE_SIZE, sizeof(afe_queue_data_item_t));
    if (CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "cy_rtos_init_queue failed");
        goto CLEAN_RETURN;
    }

    cy_afe_log_dbg("Audio processing queue initialized");

    context->audio_processing_thread_running = true;

    cy_afe_log_dbg("Creating audio processing task");

    /*
     * Create AFE audio processing task
     */
    result = cy_rtos_create_thread(&context->audio_processing_thread, afe_audio_processing_task, AFE_AUDIO_PROCESSING_TASK_NAME, NULL,
                                    AFE_AUDIO_PROCESSING_TASK_THREAD_STACK_SIZE, AFE_AUDIO_PROCESSING_TASK_PRIORITY, context);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "cy_rtos_create_thread failed");
        goto CLEAN_RETURN;
    }

    cy_afe_log_dbg("Audio processing task created");

    return CY_RSLT_SUCCESS;

CLEAN_RETURN:
    if (true == context->audio_processing_thread_running)
    {
        cy_rtos_deinit_queue(&context->audio_processing_queue);
        cy_rtos_terminate_thread(&context->audio_processing_thread);
        context->audio_processing_thread_running = false;
    }

    return result;

}

/**
 * Main task for audio processing (speech enhancement)
 */
static void afe_audio_processing_task(cy_thread_arg_t arg)
{
    afe_internal_context_t *context = (afe_internal_context_t*) arg;

    cy_afe_log_dbg("Running audio processing task routine");

    /*
     * Run task routine till audio processing thread is stopped
     */

    while (true == context->audio_processing_thread_running)
    {
        afe_pop_audio_data_from_queue(context);
    }

    cy_afe_log_dbg("Exiting from audio processing task routine");
}

/**
 * Terminate audio processing thread & de-initialize queue
 */
cy_rslt_t afe_cleanup_audio_processing_task(afe_internal_context_t *context)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if (true == context->audio_processing_thread_running)
    {
        result = cy_rtos_terminate_thread(&context->audio_processing_thread);
        if (CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "cy_rtos_terminate_thread failed");
            return result;
        }

        cy_afe_log_dbg("Audio processing thread terminated successfully");

        result = cy_rtos_join_thread(&context->audio_processing_thread);
        if (CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "cy_rtos_join_thread failed");
            return result;
        }

        cy_afe_log_dbg("Audio processing thread join completed");
        context->audio_processing_thread_running = false;
    }

    result = cy_rtos_deinit_queue(&context->audio_processing_queue);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "cy_rtos_deinit_queue failed");
        return result;
    }

    cy_afe_log_dbg("Audio processing queue de-initialized successfully");

    return CY_RSLT_SUCCESS;
}

/**
 * Pop message from queue and process
 */
static cy_rslt_t afe_pop_audio_data_from_queue(afe_internal_context_t *context)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    afe_queue_data_item_t afe_queue_item;

    result = cy_rtos_get_queue(&context->audio_processing_queue, &afe_queue_item, CY_RTOS_NEVER_TIMEOUT, false);
    if(CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "Failed to pop message from queue");
        return result;
    }
    else
    {
        return afe_process_audio_data(context, &afe_queue_item);
    }
}

/**
 * Push input audio data to AFE thread queue for processing
 */
cy_rslt_t afe_push_audio_data_to_queue(afe_internal_context_t *context,
        CY_AFE_DATA_T *input_audio_data_ptr, CY_AFE_DATA_T *aec_ref_ptr)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    afe_queue_data_item_t afe_data_item;

    if (NULL == context || NULL == input_audio_data_ptr) {
        result = CY_RSLT_AFE_BAD_ARG;
        cy_afe_log_err_on_no_isr(result,
                "Invalid argument. context: [%p], item : [%p]", context,
                input_audio_data_ptr);
        return result;
    }

    memset(&afe_data_item, 0, sizeof(afe_queue_data_item_t));

#ifdef CY_AFE_ENABLE_CRC_CHECK
    afe_data_item.crc_value = afe_get_crc_checksum_val(context, input_audio_data_ptr);
#endif

#ifdef CY_AFE_ENABLE_TIMESTAMP
    afe_update_timestamp(context, AFE_FRAME_RECEIVED_TIMESTAMP);
#endif

    afe_data_item.input_data_ptr = input_audio_data_ptr;
    afe_data_item.aec_ref_ptr = aec_ref_ptr;

#ifdef CY_AFE_ENABLE_STATS
    afe_update_stats(context, AFE_FRAME_FEED_COUNT);
#endif

#ifdef ENABLE_AFE_APP_CHECK_POINT
    AFE_APP_CHECK_POINT()
#endif

    result = cy_rtos_put_queue(&context->audio_processing_queue, &afe_data_item,
            CY_RTOS_NEVER_TIMEOUT, is_in_isr());

#ifdef ENABLE_AFE_APP_CHECK_POINT
    AFE_APP_CHECK_POINT()
#endif

    if (CY_RSLT_SUCCESS != result)
    {
        afe_update_stats(context, AFE_FRAME_PUSH_TO_QUEUE_FAIL_COUNT);
        cy_afe_log_err_on_no_isr(result, "cy_rtos_put_queue failed");
        return result;
    }

    return result;
}

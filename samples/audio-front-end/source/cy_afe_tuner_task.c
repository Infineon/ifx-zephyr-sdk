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
 * @file cy_afe_tuner_task.c
 * @brief Set of routines to setup a audio tuner thread & queue
 *
 */

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
#include "cy_audio_front_end.h"
#include "cy_afe_audio_internal.h"
#include "cy_afe_tuner_process.h"
#include "cy_afe_audio_speech_enh.h"
#include "cy_afe_tuner_task.h"
/******************************************************
 *                     Macros
 ******************************************************/
#define AFE_AUDIO_TUNER_TASK_NAME                        "afe-tuner-task"
#ifndef AFE_TUNER_TASK_THREAD_STACK_SIZE
#define AFE_TUNER_TASK_THREAD_STACK_SIZE                 (6*1024)
#endif

#ifndef AFE_TUNER_TASK_PRIORITY
#define AFE_TUNER_TASK_PRIORITY                          (CY_RTOS_PRIORITY_NORMAL)
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
static void afe_tuner_task(cy_thread_arg_t arg);
/******************************************************
 *               Functions
 ******************************************************/

cy_rslt_t afe_cleanup_memory_for_dbg_output(afe_internal_context_t* context)
{
    if(NULL != context->dbg_output1)
    {
        if(afe_free_memory)
        {
            afe_free_memory(CY_AFE_MEM_ID_AFE_DBG_OUT_BUFFER,context->dbg_output1);
        }
        else
        {
            free(context->dbg_output1);
        }
        context->dbg_output1 = NULL;
    }

    if(NULL != context->dbg_output2)
    {
        if(afe_free_memory)
        {
            afe_free_memory(CY_AFE_MEM_ID_AFE_DBG_OUT_BUFFER,context->dbg_output2);
        }
        else
        {
            free(context->dbg_output2);
        }
        context->dbg_output2 = NULL;
    }

    if(NULL != context->dbg_output3)
    {
        if(afe_free_memory)
        {
            afe_free_memory(CY_AFE_MEM_ID_AFE_DBG_OUT_BUFFER,context->dbg_output3);
        }
        else
        {
            free(context->dbg_output3);
        }
        context->dbg_output3 = NULL;
    }

    if(NULL != context->dbg_output4)
    {
        if(afe_free_memory)
        {
            afe_free_memory(CY_AFE_MEM_ID_AFE_DBG_OUT_BUFFER,context->dbg_output4);
        }
        else
        {
            free(context->dbg_output4);
        }
        context->dbg_output4 = NULL;
    }

    return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_allocate_memory_for_dbg_output(afe_internal_context_t* context)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* TODO: Allocate memory only based on configuration of debug output in
     * cy_afe_configurator_settings.h
     */

    if(NULL == context->dbg_output1)
    {
        if(afe_alloc_memory)
        {
            afe_alloc_memory(CY_AFE_MEM_ID_AFE_DBG_OUT_BUFFER,  (uint32_t)CY_AFE_MONO_FRAME_SIZE_IN_BYTES,
                    (void **)&context->dbg_output1);
        }
        else
        {
            context->dbg_output1 = calloc(1, CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
        }
        if(NULL == context->dbg_output1)
        {
            result = CY_RSLT_AFE_OUT_OF_MEMORY;
            return result;
        }
    }

    if(NULL == context->dbg_output2)
    {
        if(afe_alloc_memory)
        {
            afe_alloc_memory(CY_AFE_MEM_ID_AFE_DBG_OUT_BUFFER,  (uint32_t)CY_AFE_MONO_FRAME_SIZE_IN_BYTES,
                    (void **)&context->dbg_output2);
        }
        else
        {
            context->dbg_output2 = calloc(1, CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
        }
        if(NULL == context->dbg_output2)
        {
            result = CY_RSLT_AFE_OUT_OF_MEMORY;
            return result;
        }
    }

    if(NULL == context->dbg_output3)
    {
        if(afe_alloc_memory)
        {
            afe_alloc_memory(CY_AFE_MEM_ID_AFE_DBG_OUT_BUFFER,  (uint32_t)CY_AFE_MONO_FRAME_SIZE_IN_BYTES,
                    (void **)&context->dbg_output3);
        }
        else
        {
            context->dbg_output3 = calloc(1, CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
        }
        if(NULL == context->dbg_output3)
        {
            result = CY_RSLT_AFE_OUT_OF_MEMORY;
            return result;
        }
    }

    if(NULL == context->dbg_output4)
    {
        if(afe_alloc_memory)
        {
            afe_alloc_memory(CY_AFE_MEM_ID_AFE_DBG_OUT_BUFFER,  (uint32_t)CY_AFE_MONO_FRAME_SIZE_IN_BYTES,
                    (void **)&context->dbg_output4);
        }
        else
        {
            context->dbg_output4 = calloc(1, CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
        }
        if(NULL == context->dbg_output4)
        {
            result = CY_RSLT_AFE_OUT_OF_MEMORY;
            return result;
        }
    }

    return result;
}

/*
 * Setup the audio tuner processing task and queue
 */
cy_rslt_t afe_setup_audio_tuner_task(afe_internal_context_t *context, cy_afe_config_t *config)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /*
     * Allocate memory to accumulate tuning request which comes from configurator tool
     *
     */
    if(afe_alloc_memory)
    {
        afe_alloc_memory(CY_AFE_MEM_ID_AFE_TUNER_CMD_BUFFER, (uint32_t)CY_AFE_TUNER_MAX_COMMAND_BUFFER_SIZE,
                (void **)&context->internal_request_cmd_buffer);
    }
    else
    {
        context->internal_request_cmd_buffer = malloc(CY_AFE_TUNER_MAX_COMMAND_BUFFER_SIZE);
        if(NULL == context->internal_request_cmd_buffer)
        {
            cy_afe_log_err(result, "Memory allocation failed for command buffer");
            afe_cleanup_audio_tuner_task(context);
            return result;
        }
        memset(context->internal_request_cmd_buffer, 0, CY_AFE_TUNER_MAX_COMMAND_BUFFER_SIZE);
    }

    context->poll_interval_ms = config->poll_interval_ms;
    context->audio_tuner_thread_running = true;

    /*
     * Create AFE audio tuner task
     */
    result = cy_rtos_create_thread(&context->audio_tuner_thread, afe_tuner_task,
            AFE_AUDIO_TUNER_TASK_NAME, NULL,
            AFE_TUNER_TASK_THREAD_STACK_SIZE, AFE_TUNER_TASK_PRIORITY, context);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "cy_rtos_create_thread failed");
        afe_cleanup_audio_tuner_task(context);
        return result;
    }

    if(afe_alloc_memory)
    {
        afe_alloc_memory(CY_AFE_MEM_ID_AFE_TUNER_CMD_BUFFER,  (uint32_t)(MAX_NUM_PARAMS*sizeof(char*)),
                (void **)&context->tuner_cmd_params);
    }
    else
    {
        context->tuner_cmd_params = (char**) calloc(MAX_NUM_PARAMS, sizeof(char*));
    }
    if ( NULL == context->tuner_cmd_params )
    {
        result = CY_RSLT_AFE_OUT_OF_MEMORY;
        cy_afe_log_err(result, "Failed to allocate memory for tuner command params");
        afe_cleanup_audio_tuner_task(context);
        return result;
    }

    result = cy_rtos_mutex_init(&context->audio_tuner_mutex, false);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "semaphore init failed");
        afe_cleanup_audio_tuner_task(context);
        return result;
    }

    return result;
}


/*
 * Tuner task main routine
 */
static void afe_tuner_task(cy_thread_arg_t arg)
{
    afe_internal_context_t *context = (afe_internal_context_t*) arg;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    cy_afe_log_dbg("Running audio tuner task routine");

    /**
     * Receive commands from configurator tool, process and respond back to configurator tool
     */
    while (true == context->audio_tuner_thread_running)
    {
        result = afe_receive_tuner_command_req(context);

        /** Wait for poll timeout */
        if(CY_RSLT_AFE_TUNER_WAIT_FOR_POLL_TIMEOUT == result)
        {
            cy_rtos_delay_milliseconds(context->poll_interval_ms);
        }
    }

    cy_afe_log_dbg("Exiting from audio tuner task routine");
}

/*
 * Cleanup the allocated resources for audio tuner task
 */
cy_rslt_t afe_cleanup_audio_tuner_task(afe_internal_context_t *context)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if (true == context->audio_tuner_thread_running)
    {

        /* Terminate the audio processing task */
        result = cy_rtos_terminate_thread(&context->audio_tuner_thread);
        if (CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "cy_rtos_terminate_thread failed");
            return result;
        }

        cy_afe_log_dbg("Audio tuner thread terminated successfully");

        /* Join the audio processing thread to cleanup the resources */
        result = cy_rtos_join_thread(&context->audio_tuner_thread);
        if (CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "cy_rtos_join_thread failed");
            return result;
        }

        cy_afe_log_dbg("Audio tuner thread join completed");
        context->audio_tuner_thread_running = false;
    }

    if(NULL != context->internal_request_cmd_buffer)
    {
        if(afe_free_memory)
        {
            afe_free_memory(CY_AFE_MEM_ID_AFE_TUNER_CMD_BUFFER,context->internal_request_cmd_buffer);
        }
        else
        {
            free(context->internal_request_cmd_buffer);
        }
        context->internal_request_cmd_buffer = NULL;
    }

    if(NULL != context->tuner_cmd_params)
    {
        if(afe_free_memory)
        {
            afe_free_memory(CY_AFE_MEM_ID_AFE_TUNER_CMD_BUFFER,context->tuner_cmd_params);
        }
        else
        {
            free(context->tuner_cmd_params);
        }
        context->tuner_cmd_params = NULL;
    }

    result = cy_rtos_mutex_deinit(&context->audio_tuner_mutex);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "semaphore deinit failed");
        return result;
    }

    return result;
}
#endif

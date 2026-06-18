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
 * @file cy_afe_audio_bd_calc.c
 * @brief Set of routines for speech enhancement. These functions are interface between
 *        AFE middleware code and system component APIs.
 *
 */

#ifdef CY_AFE_ENABLE_TUNING_FEATURE

#include "cy_afe_audio_speech_enh.h"
#include "cy_audio_front_end.h"
#include "cy_afe_audio_internal.h"
#include "cy_sp_enh.h"
#include "bulk_delay_measurement.h"
#include "cy_afe_audio_bd_calc.h"
#include <inttypes.h>

bdm_struct_t bdm_struct = {0};
unsigned int ref_frame_count = 0;
unsigned int repeat_count = 0;
unsigned int num_ref_frames = 0;
static cy_semaphore_t *bdCalcSyncSemaphore = NULL;
int bulk_delay_output = BULK_DELAY_ERROR;

#if ENABLE_BDM_PRINT_LOGS
#define BDM_PRINT_LOGS(format,...)  printf ("[BDM] "format" \r\n", ##__VA_ARGS__);
#else
#define BDM_PRINT_LOGS(format,...)
#endif

cy_rslt_t cy_afe_bd_calc_init(cy_afe_t afe_handle, bdm_init_out_params_t *bdm_out)
{
    cy_rslt_t ret_val = CY_RSLT_SUCCESS;
    uint32_t frame_size_msec = 10; //HardCoded. It will not change.
    uint32_t sampling_rate = 16000;  //HardCoded. It will not change.
    uint32_t frame_size = 0;
    param_struct_t param_struct = {0};
    afe_internal_context_t *handle = NULL;
    int32_t bdm_state = 0;
    int16_t* ref_buffer = NULL;
    int32_t ref_length = 0;

    if((NULL == afe_handle) || (NULL == bdm_out))
    {
        ret_val = CY_RSLT_AFE_BAD_ARG;
        goto CLEAN_RETURN;
    }

    frame_size = frame_size_msec * sampling_rate / 1000;
    handle = (afe_internal_context_t*) afe_handle;

    /* Reset any global context */
    memset(&bdm_struct, 0, sizeof(bdm_struct_t));

    param_struct.frame_size = frame_size;
    param_struct.sampling_rate = sampling_rate;
    (void) afe_bdm_init(&param_struct, &bdm_struct);

    // get generated ref signal and its length, number of ref frames, and max number of bdm repeat
    ref_buffer = afe_bdm_get_ref_signal(&bdm_struct);
    ref_length = afe_bdm_get_ref_length(&bdm_struct);
    num_ref_frames = ref_length / frame_size;

    handle->is_bdm_enabled = true;

    ret_val = cy_rtos_init_semaphore( bdCalcSyncSemaphore, 1, 0 );
    if (CY_RSLT_SUCCESS != ret_val)
    {
        cy_afe_log_err(ret_val, "Failed to initialize bd calc sync semaphore ");
        goto CLEAN_RETURN;
    }

    bdm_state = afe_bdm_get_state(&bdm_struct);
    BDM_PRINT_LOGS("BDM initial state:%"PRIi32, bdm_state);
    (void)bdm_state;

    bdm_out->aec_ref_buffer = ref_buffer;
    bdm_out->aec_ref_buffer_len = ref_length*2;

    bdm_out->aec_ref_buffer = malloc(ref_length*2);
    if(NULL == bdm_out->aec_ref_buffer)
    {
        ret_val = CY_RSLT_AFE_OUT_OF_MEMORY;
        cy_afe_log_err(ret_val, "out of memory sz:%"PRIi32,ref_length*2);
        goto CLEAN_RETURN;
    }

    memcpy((void*)bdm_out->aec_ref_buffer, (void*)ref_buffer, ref_length*2);
    handle->bdm_out = *bdm_out;

    BDM_PRINT_LOGS("ref_buffer:%p, ref_length = %"PRIi32,ref_buffer, ref_length);
    cy_afe_log_info("BDM calc init success");

CLEAN_RETURN:
    if((NULL != handle) && (NULL != handle->bdm_out.aec_ref_buffer))
    {
        free(handle->bdm_out.aec_ref_buffer);
        handle->bdm_out.aec_ref_buffer = NULL;
        handle->bdm_out.aec_ref_buffer_len = 0;
    }
    return ret_val;
}


cy_rslt_t cy_afe_bd_calc_deinit(cy_afe_t afe_handle)
{
    cy_rslt_t ret_val = CY_RSLT_SUCCESS;
    afe_internal_context_t* handle = (afe_internal_context_t*) afe_handle;

    if(NULL == handle)
    {
        ret_val = CY_RSLT_AFE_BAD_ARG;
        goto CLEAN_RETURN;
    }

    // free bdm
    afe_bdm_free(&bdm_struct);

    if(NULL != bdCalcSyncSemaphore)
    {
        cy_rtos_deinit_semaphore(bdCalcSyncSemaphore);
        bdCalcSyncSemaphore = NULL;
        cy_afe_log_info("Deinit Sem success");
    }
    else
    {
        cy_afe_log_info("Sem not yet created");
    }

    if(NULL != handle->bdm_out.aec_ref_buffer)
    {
        free(handle->bdm_out.aec_ref_buffer);
        handle->bdm_out.aec_ref_buffer = NULL;
        handle->bdm_out.aec_ref_buffer_len = 0;
    }

    memset(&bdm_struct,0,sizeof(bdm_struct));

    handle->is_bdm_enabled = false;
    bulk_delay_output = 0;
    repeat_count = 0;
    ref_frame_count = 0;
    num_ref_frames = 0;

CLEAN_RETURN:
    return ret_val;
}

cy_rslt_t cy_afe_bd_calc_wait_for_complete(unsigned int ui_timout_ms,
        int *bulk_delay)
{
    cy_rslt_t ret_val = CY_RSLT_SUCCESS;

    if(NULL == bulk_delay)
    {
        ret_val = CY_RSLT_AFE_BAD_ARG;
        goto CLEAN_RETURN;
    }
    *bulk_delay = BULK_DELAY_ERROR;

    if(NULL != bdCalcSyncSemaphore)
    {
        ret_val = cy_rtos_get_semaphore(bdCalcSyncSemaphore,ui_timout_ms, false);
        if(0 != *bulk_delay)
        {
            *bulk_delay = bulk_delay_output;
        }
    }

CLEAN_RETURN:
    return ret_val;
}

cy_rslt_t cy_afe_bd_calc_process(afe_internal_context_t *context,
        afe_sp_enh_input_output_t *sp_enh_input_output)
{
    cy_rslt_t ret_val = CY_RSLT_SUCCESS;
    int32_t bdm_state = 0;

    if((NULL == context || NULL == sp_enh_input_output))
    {
        ret_val = CY_RSLT_AFE_BAD_ARG;
        goto CLEAN_RETURN;
    }

    ret_val = afe_bdm_process(&bdm_struct,
            sp_enh_input_output->aec_reference_input,
            sp_enh_input_output->input1,
            sp_enh_input_output->output);

    bdm_state = afe_bdm_get_state(&bdm_struct);

    ref_frame_count++;
    if (ref_frame_count == num_ref_frames)
    {
        // finished one round of ref signal playback, get ready to repeat
        ref_frame_count = 0;
        if (bdm_state != AFE_BDM_SUCCESS)
        {
            repeat_count++;
        }
    }

    if (bdm_state == AFE_BDM_SUCCESS)
    {
        bulk_delay_output = afe_bdm_get_bulk_delay(&bdm_struct);

        BDM_PRINT_LOGS("%"PRIi32"valid delay estimates were obtained after %d repeats, with average delay of %dmsec",
            bdm_struct.valid_delay_count, repeat_count,	bulk_delay_output);

        cy_afe_log_info("%"PRIi32"valid delay estimates were obtained after %d repeats, with average delay of %dmsec",
            bdm_struct.valid_delay_count, repeat_count,	bulk_delay_output);

        goto BDM_COMPLETED;
    }
    else
    {
        if(repeat_count == BULK_DELAY_IDENTIFY_MAX_REPEAT_COUNT)
        {
            cy_afe_log_info("Err !! BDM Timeout");
            BDM_PRINT_LOGS("Err !! BDM Timeout");
            bulk_delay_output = BULK_DELAY_ERROR;
            goto BDM_COMPLETED;
        }
    }

    if(CY_RSLT_SUCCESS != ret_val)
    {
        cy_afe_log_err(ret_val, "Failed to process speech enhancement");
        ret_val = CY_RSLT_AFE_SYSTEM_MODULE_ERROR;
        return ret_val;
    }

    return CY_RSLT_SUCCESS;


 BDM_COMPLETED:
    repeat_count = 0;
    ref_frame_count = 0;
    if(NULL != context)
        context->is_bdm_enabled = false;
    if(NULL != bdCalcSyncSemaphore)
        (void) cy_rtos_set_semaphore(bdCalcSyncSemaphore, false);

    return CY_RSLT_SUCCESS;

 CLEAN_RETURN:
      return ret_val;
}

#endif

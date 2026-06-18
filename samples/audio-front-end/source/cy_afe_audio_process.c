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
 * @file cy_afe_audio_process.c
 * @brief Set of routines for processing the audio frames with AFE submodules.
 *
 */

#include "cy_afe_audio_process.h"
#include "cy_audio_front_end.h"
#include "cy_afe_audio_internal.h"
#include "cy_afe_audio_speech_enh.h"
#ifdef CY_AFE_ENABLE_TUNING_FEATURE
#include "cy_afe_audio_bd_calc.h"
#endif
#include "cy_afe_configurator_settings.h"
#ifdef COMPONENT_PROFILER
#include "cy_afe_profiler.h"
#endif

//#define FILE_INPUT_FEED 1
#ifdef FILE_INPUT_FEED
#include "ok_infineon_100_talkers_stereo.wav.h"
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
static cy_rslt_t afe_send_output_data(afe_internal_context_t *context, afe_sp_enh_input_output_t* sp_enh_output, bool before_afe_proc);
#ifdef CY_AFE_ENABLE_TUNING_FEATURE
static cy_rslt_t afe_fill_debug_output_buf(int channel_no, afe_sp_enh_input_output_t *input_output_buf_ptr,
                                            int16_t *dbg_output, bool before_afe_proc);
#endif
void afe_free_debug_output_buf(cy_afe_buffer_info_t* buffer);
/******************************************************
 *               Functions
 ******************************************************/

#ifdef FILE_INPUT_FEED
static int offset = 44;
void fill_10ms_data(uint16_t* buffer)
{
    int i=0;
    for(i=0;i<160;i++)
    {
        memcpy(buffer, (uint8_t*) hex_array + offset, 2);
        buffer += 1;
        offset += 2;
        offset += 2;
    }
}
#endif

extern uint32_t ifx_out_locator[];

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
static cy_rslt_t afe_fill_debug_output_buf(int channel_no, afe_sp_enh_input_output_t *input_output_buf_ptr,
                                            int16_t *dbg_output, bool before_afe_proc)
{
    /* Copy required input buffer before AFE process */
    if(true == before_afe_proc)
    {
        switch(channel_no)
        {
            case AFE_USB_SELECT_AEC_REF:
            {
                /*
                * For AEC reference data, Only mono audio data is supported.
                */
                if(NULL == input_output_buf_ptr->aec_reference_input)
                {
                    memset((char*)dbg_output,0,CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
                }
                else
                {
                    memcpy((char*)dbg_output, (char*)input_output_buf_ptr->aec_reference_input, CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
                }
            }
            break;
            case AFE_USB_SELECT_INPUT_0:
            {
                /*
                * Copy first 320 bytes from input buffer
                */
                if(NULL == input_output_buf_ptr->input1)
                {
                    memset((char*)dbg_output,0,CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
                }
                else
                {
                    memcpy((char*)dbg_output, (char*)input_output_buf_ptr->input1, CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
                }
            }
            break;
            case AFE_USB_SELECT_INPUT_1:
            {
                /*
                * Copy next 320 bytes from input buffer for stereo data
                */
                if(NULL ==  input_output_buf_ptr->input2)
                {
                    memset((char*)dbg_output,0,CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
                } else
                {
                    memcpy((char*)dbg_output,(char*) input_output_buf_ptr->input2, CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
                }
            }
            break;
            default:
            break; /* Nothing to do */
        }
    }

    /* Copy required output buffer after AFE process */
    if(false == before_afe_proc)
    {
        switch(channel_no)
        {
            case AFE_USB_SELECT_OUTPUT:
            {
                /*
                * AFE output is always mono.
                */
                memcpy((char*)dbg_output, (char*)input_output_buf_ptr->output, CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
            }
            break;
            case AFE_USB_SELECT_SIG_A_0:
            {
                //AEC Output channel 0
                if(-1 != ifx_out_locator[AFE_USB_SELECT_SIG_A_0])
                {
                    char *ifx_temp_copy = (char*)((char*)input_output_buf_ptr->ifx_internal_output + (ifx_out_locator[AFE_USB_SELECT_SIG_A_0] * CY_AFE_MONO_FRAME_SIZE_IN_BYTES));
                    memcpy((char*)dbg_output, (char*)ifx_temp_copy, CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
                }
                else
                {
                    memset((char*)dbg_output,0,CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
                }
            }
            break;
            case AFE_USB_SELECT_SIG_A_1:
            {
                //AEC output channel 1
                if(-1 != ifx_out_locator[AFE_USB_SELECT_SIG_A_1])
                {
                    char *ifx_temp_copy = (char*)((char*)input_output_buf_ptr->ifx_internal_output + (ifx_out_locator[AFE_USB_SELECT_SIG_A_1] * CY_AFE_MONO_FRAME_SIZE_IN_BYTES));
                    memcpy((char*)dbg_output, (char*)ifx_temp_copy, CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
                }
                else
                {
                    memset((char*)dbg_output,0,CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
                }
            }
            break;
            case AFE_USB_SELECT_SIG_B:
            {
                //Beam forming output
                if(-1 != ifx_out_locator[AFE_USB_SELECT_SIG_B])
                {
                    char *ifx_temp_copy = (char*)((char *)input_output_buf_ptr->ifx_internal_output + (ifx_out_locator[AFE_USB_SELECT_SIG_B] * CY_AFE_MONO_FRAME_SIZE_IN_BYTES));
                    memcpy((char*)dbg_output, (char*)ifx_temp_copy, CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
                }
                else
                {
                    memset((char*)dbg_output,0,CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
                }
            }
            break;
            case AFE_USB_SELECT_SIG_C:
            {
                //De-reverberation output
                if(-1 != ifx_out_locator[AFE_USB_SELECT_SIG_C])
                {
                    char *ifx_temp_copy = (char *)((char *)input_output_buf_ptr->ifx_internal_output + (ifx_out_locator[AFE_USB_SELECT_SIG_C] * CY_AFE_MONO_FRAME_SIZE_IN_BYTES));
                    memcpy((char*)dbg_output, (char*)ifx_temp_copy, CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
                }
                else
                {
                    memset((char*)dbg_output,0,CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
                }
            }
            break;
            default:
            break; /* Nothing to do */
        }
    }

    return CY_RSLT_SUCCESS;
}
#endif

/**
 * Send the AFE output data to application based on configuration
 */
static cy_rslt_t afe_send_output_data(afe_internal_context_t *context, afe_sp_enh_input_output_t* sp_enh_output, bool before_afe_proc)
{
    cy_afe_buffer_info_t afe_output_info;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    memset(&afe_output_info, 0, sizeof(cy_afe_buffer_info_t));

    afe_output_info.input_buf = sp_enh_output->input1;
    afe_output_info.input_aec_ref_buf = sp_enh_output->aec_reference_input;
    afe_output_info.output_buf = sp_enh_output->output;

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
    /* Fill debug output based on configuration for channel1 */
    result = afe_fill_debug_output_buf(MY_AFE_USB_SETTINGS.channel_0, sp_enh_output, context->dbg_output1, before_afe_proc);
    if(result != CY_RSLT_SUCCESS)
    {
        cy_afe_log_err(result, "Failed to fill debug output for channel1");
        return result;
    }

    afe_output_info.dbg_output1 = context->dbg_output1;

    /* Fill debug output based on configuration for channel2 */
    result = afe_fill_debug_output_buf(MY_AFE_USB_SETTINGS.channel_1, sp_enh_output, context->dbg_output2, before_afe_proc);
    if(result != CY_RSLT_SUCCESS)
    {
        cy_afe_log_err(result, "Failed to fill debug output for channel2");
        return result;
    }

    afe_output_info.dbg_output2 = context->dbg_output2;

    /* Fill debug output based on configuration for channel3 */
    result = afe_fill_debug_output_buf(MY_AFE_USB_SETTINGS.channel_2, sp_enh_output, context->dbg_output3, before_afe_proc);
    if(result != CY_RSLT_SUCCESS)
    {
        cy_afe_log_err(result, "Failed to fill debug output for channel3");
        return result;
    }

    afe_output_info.dbg_output3 = context->dbg_output3;

    /* Fill debug output based on configuration for channel4 */
    result = afe_fill_debug_output_buf(MY_AFE_USB_SETTINGS.channel_3, sp_enh_output, context->dbg_output4, before_afe_proc);
    if(result != CY_RSLT_SUCCESS)
    {
        cy_afe_log_err(result, "Failed to fill debug output for channel4");
        return result;
    }

    afe_output_info.dbg_output4 = context->dbg_output4;

#endif

#ifdef ENABLE_AFE_MW_CHECK_POINT
    AFE_MW_CHECK_POINT()
#endif

    /* Send output only after processing */
    if( false == before_afe_proc )
    {
        /*
        * Invoke application registered AFE output callback
        */
        result = context->config_init.afe_output_callback((void*) context, &afe_output_info,
                context->config_init.user_arg_callbacks);

    #ifdef ENABLE_AFE_MW_CHECK_POINT
        AFE_MW_CHECK_POINT()
    #endif

        if(CY_RSLT_SUCCESS != result)
        {
            /* Avoid error from application & continue processing next set of audio frames */
            cy_afe_log_err(result, "AFE output callback returned error");
        }
    }

    return CY_RSLT_SUCCESS;
}

/**
 * Process audio data and apply speech enhancement
 */
cy_rslt_t afe_process_audio_data(afe_internal_context_t *context, afe_queue_data_item_t *queue_item)
{
    cy_afe_buffer_info_t output_info;
    CY_AFE_DATA_T *input_buffer = NULL;
    CY_AFE_DATA_T *output_buffer = NULL;
#ifdef ENABLE_IFX_AEC
    CY_AFE_DATA_T *aec_ref_buffer = NULL;
#endif
    cy_rslt_t result = CY_RSLT_SUCCESS;
    afe_sp_enh_input_output_t sp_enh_in_out;
    size_t no_of_items_in_queue = 0;

    memset(&output_info, 0, sizeof(output_info));
    memset(&sp_enh_in_out, 0, sizeof(sp_enh_in_out));

    /* Audio input data pointer */
    input_buffer = queue_item->input_data_ptr;

#ifdef ENABLE_IFX_AEC
    aec_ref_buffer = queue_item->aec_ref_ptr;
#endif

    if(context->afe_frame_queue_push_fail_cnt > 0)
    {
        no_of_items_in_queue = cy_rtos_queue_count(&context->audio_processing_queue, &no_of_items_in_queue);
        cy_afe_log_info(
                "Failed to push audio data pointer to queue from cy_afe_feed API. no_of_items_queue :[%d], failed_to_push_cnt : [%d]",
                no_of_items_in_queue, context->afe_frame_queue_push_fail_cnt);
    }

#ifdef CY_AFE_ENABLE_HEXDUMP
    afe_dump_hex(context, AFE_INPUT_AUDIO, input_buffer);
    afe_dump_hex(context, AFE_AEC_REF, aec_ref_buffer);
#endif

#ifdef CY_AFE_ENABLE_CRC_CHECK
    /* Validate CRC checksum for input audio data */
    result = afe_validate_crc_checksum(context, input_buffer, queue_item->crc_value);
    if(CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "CRC checksum validation failed");
        return result;
    }
#endif

#ifdef ENABLE_IFX_AEC
    sp_enh_in_out.aec_reference_input = aec_ref_buffer;
#else
    sp_enh_in_out.aec_reference_input = NULL;
#endif

#ifdef ENABLE_AFE_MW_CHECK_POINT
    AFE_MW_CHECK_POINT()
#endif

    /* Invoke get_output_buffer_cb to get the buffer to fill afe output data */
    result = context->get_output_buffer_cb((afe_internal_context_t*) context, (uint32_t**) &output_buffer,
            context->config_init.user_arg_callbacks);

#ifdef ENABLE_AFE_MW_CHECK_POINT
    AFE_MW_CHECK_POINT()
#endif

    if(CY_RSLT_SUCCESS != result || output_buffer == NULL)
    {
        cy_afe_log_err(result, "Failed to get the AFE output buffer from application");
        return CY_RSLT_AFE_GENERIC_ERROR;
    }
    else
    {
        /* replace sample with channel */
        if (CY_AFE_INPUT_NUM_OF_CHANNELS == CY_AFE_AUDIO_MONO_CHANNEL)
        {
            sp_enh_in_out.input1 = (CY_AFE_DATA_T*) input_buffer;
            sp_enh_in_out.input2 = NULL;
        }
        else if (CY_AFE_INPUT_NUM_OF_CHANNELS == CY_AFE_AUDIO_STEREO_CHANNEL)
        {
            sp_enh_in_out.input1 = input_buffer;
            sp_enh_in_out.input2 = (CY_AFE_DATA_T*) ((char*)input_buffer + 320);
        }

        sp_enh_in_out.ifx_internal_output = (CY_AFE_DATA_T *)context->ifx_internal_output;
        sp_enh_in_out.output = output_buffer;

#ifdef CY_AFE_ENABLE_TIMESTAMP
        afe_update_timestamp(context, AFE_FRAME_FEED_SE_TIMESTAMP);
#endif

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
        cy_rtos_mutex_get(&context->audio_tuner_mutex,CY_RTOS_NEVER_TIMEOUT);
        afe_send_output_data(context, &sp_enh_in_out, true);
#endif

#ifdef COMPONENT_PROFILER
        cy_afe_profile(AFE_PROFILE_CMD_START,NULL);
#endif /* COMPONENT_PROFILER */

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
        if(context->is_bdm_enabled == false)
#endif
        {
            /* Apply speech enhancement on audio data */
            result = afe_speech_enhancement_process(context->sp_enh_context, &sp_enh_in_out);
        }
#ifdef CY_AFE_ENABLE_TUNING_FEATURE
        else
        {
            result = cy_afe_bd_calc_process(context, &sp_enh_in_out);
#ifdef DUMP_SAVE_BY_INDEX
            cy_dump_save_by_index(0, sp_enh_in_out.input1, 320, 0);
            cy_dump_save_by_index(1, sp_enh_in_out.aec_reference_input, 320, 0);
            cy_dump_save_by_index(2, sp_enh_in_out.output, 320, 0);
#endif /* DUMP_SAVE_BY_INDEX */
        }
#endif

#ifdef COMPONENT_PROFILER
        cy_afe_profile(AFE_PROFILE_CMD_STOP,NULL);
#endif /* COMPONENT_PROFILER */

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
        cy_rtos_mutex_set(&context->audio_tuner_mutex);
#endif

#ifdef CY_AFE_ENABLE_TIMESTAMP
        afe_update_timestamp(context, AFE_FRAME_PROCESSED_TIMESTAMP);
#endif

#ifdef CY_AFE_ENABLE_STATS
        afe_update_stats(context, AFE_FRAME_PROCESSED_COUNT);
#endif

#ifdef CY_AFE_ENABLE_HEXDUMP
        afe_dump_hex(context, AFE_OUTPUT, output_buffer);
#endif

        /* Send afe output along with other information to application registered callback */
        afe_send_output_data(context, &sp_enh_in_out, false);

        if (CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to perform speech enhancement on input data");
            return CY_RSLT_AFE_SPEECH_ENHANCEMENT_ERROR;
        }

        return CY_RSLT_SUCCESS;
    }
}

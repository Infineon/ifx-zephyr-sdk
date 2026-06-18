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
 * @file cy_afe_audio_debug.c
 * @brief Set of functions for AFE middleware debugging
 *
 */

#include "cy_result.h"
#include "cy_afe_audio_debug.h"
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
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/
afe_mw_check_points afe_mw_check_point = {0};
/******************************************************
 *               Static Functions
 ******************************************************/

/******************************************************
 *               Functions
 ******************************************************/

cy_rslt_t afe_update_stats(void *context, afe_stats_type_t type)
{
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    switch(type)
    {
        case AFE_FRAME_FEED_COUNT:
        {
            handle->afe_feed_counter++;
            break;
        }
        case AFE_FRAME_PROCESSED_COUNT:
        {
            handle->afe_frame_processed++;
            break;
        }
        case AFE_FRAME_PUSH_TO_QUEUE_FAIL_COUNT:
        {
            handle->afe_frame_queue_push_fail_cnt++;
        }
        default:
        {

        }
    }

    return CY_RSLT_SUCCESS;
}

#ifdef CY_AFE_ENABLE_TIMESTAMP
cy_rslt_t afe_update_timestamp(void *context, afe_timestamp_type_t type)
{
    cy_time_t time;
    switch(type)
    {
        case AFE_FRAME_RECEIVED_TIMESTAMP:
        {
            cy_rtos_get_time(&time);
            cy_afe_log_dbg("Frame rcvd time:[%u]", time);
            break;
        }
        case AFE_FRAME_FEED_SE_TIMESTAMP:
        {
            cy_rtos_get_time(&time);
            cy_afe_log_dbg("Frame fed to SE:[%u]", time);
            break;
        }
        case AFE_FRAME_PROCESSED_TIMESTAMP:
        {
            cy_rtos_get_time(&time);
            cy_afe_log_dbg("Frame processed time:[%u]", time);
            break;
        }
        default:
        {
            break;
        }
    }

    return CY_RSLT_SUCCESS;
}
#endif

#ifdef CY_AFE_ENABLE_HEXDUMP
cy_rslt_t afe_dump_hex(void *context, afe_hexdump_type_t type, int16_t *ptr)
{
    int audio_frame_size = 0, i = 0;
    char *audio_data = NULL;

    switch(type)
    {
        case AFE_INPUT_AUDIO:
            printf("AFE Input data : [%p] \n", ptr);

#if CY_AFE_INPUT_NUM_OF_CHANNELS == CY_AFE_AUDIO_MONO_SAMPLE
            audio_frame_size = CY_AFE_MONO_FRAME_SIZE * sizeof(int16_t);
#else
            audio_frame_size = (CY_AFE_STEREO_FRAME_SIZE * sizeof(int16_t));
#endif
            audio_data = (char*) ptr;

            for(i=0; i<audio_frame_size; i++)
            {
                printf("0x%02x ", audio_data[i]);

                if (((i+1) % 16 == 0) && i)
                    printf("\n");
            }
            printf("\n");

            break;
        case AFE_AEC_REF:
            printf("AEC reference data : [%p] \n", ptr);

#if CY_AFE_INPUT_NUM_OF_CHANNELS == CY_AFE_AUDIO_MONO_SAMPLE
            audio_frame_size = CY_AFE_MONO_FRAME_SIZE * sizeof(int16_t);
#else
            audio_frame_size = (CY_AFE_STEREO_FRAME_SIZE * sizeof(int16_t));
#endif
            audio_data = (char*) ptr;

            for(i=0; i<audio_frame_size; i++)
            {
                printf("0x%02x ", audio_data[i]);

                if (((i+1) % 16 == 0) && i)
                    printf("\n");
            }
            printf("\n");
            break;
        case AFE_AEC_OUTPUT:
            printf("AFE AEC Output:[%p]", ptr);

#if CY_AFE_INPUT_NUM_OF_CHANNELS == CY_AFE_AUDIO_MONO_SAMPLE
            audio_frame_size = CY_AFE_MONO_FRAME_SIZE * sizeof(int16_t);
#else
            audio_frame_size = (CY_AFE_STEREO_FRAME_SIZE * sizeof(int16_t));
#endif
            audio_data = (char*) ptr;

            for(i=0; i<audio_frame_size; i++)
            {
                printf("0x%02x ", audio_data[i]);

                if (((i+1) % 16 == 0) && i)
                    printf("\n");
            }

            break;
        case AFE_OUTPUT:
            printf("AFE Output : [%p]\n", ptr);

            audio_frame_size = CY_AFE_MONO_FRAME_SIZE * sizeof(int16_t);
            audio_data = (char*) ptr;

            for(i=0; i<audio_frame_size; i++)
            {
                printf("0x%02x ", audio_data[i]);

                if (((i+1) % 16 == 0) && i)
                    printf("\n");
            }
            printf("\n");
            break;
    }

    return CY_RSLT_SUCCESS;
}
#endif

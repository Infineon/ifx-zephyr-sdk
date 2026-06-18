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
 * @file cy_afe_audio_internal.h
 * @brief Audio front end internal header file
 *
 */

#ifndef AUDIO_FRONT_END_INTERNAL_H__
#define AUDIO_FRONT_END_INTERNAL_H__

#include "cy_audio_front_end.h"
#include "cy_audio_front_end_error.h"
#include "cy_afe_audio_log_msg.h"
#include "cy_afe_audio_defines.h"
#include "cy_afe_audio_debug.h"
#include "stdlib.h"
#include "stdio.h"
#include "cyabs_rtos.h"
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/
#define CY_AFE_TUNER_MAX_REQUEST_BUFFER_SIZE  100
#define CY_AFE_TUNER_MAX_RESPONSE_BUFFER_SIZE 100
/******************************************************
 *                    Constants
 ******************************************************/

extern cy_afe_alloc_memory_callback_t afe_alloc_memory;
extern cy_afe_free_memory_callback_t  afe_free_memory;

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
typedef struct {
    cy_thread_t audio_processing_thread;                      // audio processing thread
    cy_queue_t audio_processing_queue;                        // audio processing queue
    volatile bool audio_processing_thread_running;            // flag to check if thread is running or not

    cy_afe_config_t config_init;
    cy_afe_get_output_buffer_callback_t get_output_buffer_cb; // callback to get the output buffer

    uint16_t input_frame_size;                                // input frame size
    uint16_t output_frame_size;                               // output frame size
    uint32_t *internal_output_buffer;                          // static internal output buffer
    uint16_t *ifx_internal_output;							  // Monitor outputs

#ifdef CY_AFE_ENABLE_TUNING_FEATURE

    uint8_t afe_request_buffer[CY_AFE_TUNER_MAX_REQUEST_BUFFER_SIZE];     // Maximum buffer size to hold tuner request
    uint8_t afe_response_buffer[CY_AFE_TUNER_MAX_RESPONSE_BUFFER_SIZE];   // Maximum buffer size to hold tuner response

    uint8_t *internal_request_cmd_buffer;     // Internal request command buffer to store request
    uint16_t internal_request_cmd_len;        // Internal request command length

    cy_afe_tuner_callbacks_t tuner_callbacks; // Tuner callbacks

    cy_thread_t audio_tuner_thread;           // audio tuner thread
    volatile bool audio_tuner_thread_running; // flag to check if thread is running or not

    uint16_t poll_interval_ms;

    char** tuner_cmd_params;
    CY_AFE_DATA_T* dbg_output1;
    CY_AFE_DATA_T* dbg_output2;
    CY_AFE_DATA_T* dbg_output3;
    CY_AFE_DATA_T* dbg_output4;

    bdm_init_out_params_t bdm_out;

    cy_mutex_t audio_tuner_mutex;
#endif

    int afe_feed_counter;    // AFE feed counter
    int afe_frame_processed; // AFE frame processed counter
    int afe_frame_queue_push_fail_cnt; // AFE frame queue push failed counter

    void *sp_enh_context; // Speech enhancement context

    bool is_bdm_enabled;

} afe_internal_context_t;

typedef struct {
    uint16_t timestamp;
    uint32_t crc_value;
    CY_AFE_DATA_T *input_data_ptr;
    CY_AFE_DATA_T *aec_ref_ptr;
} afe_queue_data_item_t;

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
 * Calculate CRC checksum value
 *
 * @param[in]  context          Audio front end middleware handle
 * @param[in]  input_audio_ptr  Pointer to input audio data
 *
 * @return checksum value
 *
 */
uint32_t afe_get_crc_checksum_val(afe_internal_context_t* context, CY_AFE_DATA_T* input_audio_ptr);

/**
 * Validate CRC checksum
 *
 * @param[in]  context          Audio front end middleware handle
 * @param[in]  input_audio_ptr  Pointer to input audio data
 * @param[in]  checsum_val      checksum value that was calculated
 *
 * @return cy_rslt_success if checksum is matching
 *
 */
cy_rslt_t afe_validate_crc_checksum(afe_internal_context_t* context, CY_AFE_DATA_T* input_audio_ptr, uint32_t checksum_val);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_FRONT_END_INTERNAL_H__ */

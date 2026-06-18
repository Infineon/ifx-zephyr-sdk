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
 * @file cy_afe_audio_utils.c
 * @brief Utility functions for AFE
 *
 */

#ifdef CY_AFE_ENABLE_CRC_CHECK
#include "cy_afe_audio_internal.h"
#include "cy_audio_front_end_error.h"
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
uint32_t afe_get_crc_checksum_val(afe_internal_context_t* context, CY_AFE_DATA_T* input_audio_ptr);
cy_rslt_t afe_validate_crc_checksum(afe_internal_context_t* context, CY_AFE_DATA_T* input_audio_ptr, uint32_t checksum_val);
/******************************************************
 *               Functions
 ******************************************************/
uint32_t afe_get_crc_checksum_val(afe_internal_context_t* context, CY_AFE_DATA_T* input_audio_ptr)
{
    int audio_input_bytes = 0;
    uint32_t *start = NULL;
    uint32_t *end = NULL;

    start = (uint32_t*) input_audio_ptr;

    audio_input_bytes = (CY_AFE_SAMPLE_FREQ * (CY_AFE_SAMPLE_WIDTH_SIZE/8) * CY_AFE_INPUT_NUM_OF_CHANNELS * CY_AFE_FRAME_SIZE_MS);

    end = (uint32_t*) (((char*) input_audio_ptr) + audio_input_bytes) - sizeof(uint32_t);

    /**
     * Simple CRC check, done only for start and end of the buffers.
     * Not required to do for the entire buffer size.
     */
    return ((*start) ^ (*end));
}

cy_rslt_t afe_validate_crc_checksum(afe_internal_context_t* context, CY_AFE_DATA_T* input_audio_ptr, uint32_t checksum_val)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    uint32_t crc_computed = 0;

    crc_computed = afe_get_crc_checksum_val(context, input_audio_ptr);

    if(crc_computed != checksum_val)
    {
        result = CY_RSLT_AFE_CRC_CHECKSUM_ERROR;
        cy_afe_log_err(result, "data corruption: [0x%08x] [0x%08x] [%p]",
                checksum_val, crc_computed, input_audio_ptr);
        return result;
    }

    return CY_RSLT_SUCCESS;
}
#endif

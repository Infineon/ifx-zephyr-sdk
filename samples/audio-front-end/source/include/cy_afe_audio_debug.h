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
 * @file cy_afe_audio_debug.h
 * @brief Set of routines to help for AFE debugging (timestamp, hexdump etc)
 *
 */

#ifndef AUDIO_FRONT_END_DEBUG_H__
#define AUDIO_FRONT_END_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/
#define ENABLE_AFE_MW_CHECK_POINT
#define ENABLE_AFE_APP_CHECK_POINT
#define ENABLE_AFE_MW_TUNER_CHECK_POINT

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/
typedef enum
{
    AFE_FRAME_FEED_COUNT,
    AFE_FRAME_PROCESSED_COUNT,
    AFE_FRAME_PUSH_TO_QUEUE_FAIL_COUNT
} afe_stats_type_t;

#ifdef CY_AFE_ENABLE_TIMESTAMP
typedef enum
{
    AFE_FRAME_RECEIVED_TIMESTAMP,
    AFE_FRAME_FEED_SE_TIMESTAMP,
    AFE_FRAME_PROCESSED_TIMESTAMP
} afe_timestamp_type_t;
#endif

#ifdef CY_AFE_ENABLE_HEXDUMP
typedef enum
{
    AFE_INPUT_AUDIO,
    AFE_AEC_REF,
    AFE_AEC_OUTPUT,
    AFE_OUTPUT
} afe_hexdump_type_t;
#endif

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    unsigned int afe_mw_check_point_line;
    unsigned int afe_app_check_point_line;
    unsigned int afe_mw_tuner_check_point_line;
} afe_mw_check_points;

extern afe_mw_check_points afe_mw_check_point;

#ifdef ENABLE_AFE_MW_CHECK_POINT
#define AFE_MW_CHECK_POINT() { afe_mw_check_point.afe_mw_check_point_line =__LINE__;}
#endif

#ifdef ENABLE_AFE_MW_TUNER_CHECK_POINT
#define AFE_MW_TUNER_CHECK_POINT() { afe_mw_check_point.afe_mw_tuner_check_point_line =__LINE__;}
#endif

#ifdef ENABLE_AFE_APP_CHECK_POINT
#define AFE_APP_CHECK_POINT() { afe_mw_check_point.afe_app_check_point_line =__LINE__;}
#endif

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/
#ifdef CY_AFE_ENABLE_TIMESTAMP
cy_rslt_t afe_update_timestamp(void *context, afe_timestamp_type_t type);
#endif

cy_rslt_t afe_update_stats(void *context, afe_stats_type_t type);

#ifdef CY_AFE_ENABLE_HEXDUMP
cy_rslt_t afe_dump_hex(void *context, afe_hexdump_type_t type, int16_t *ptr);
#endif

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_FRONT_END_DEBUG_H__ */

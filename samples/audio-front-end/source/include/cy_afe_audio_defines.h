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
 * @file cy_afe_audio_defines.h
 * @brief Defines for the audio front end middleware.
 *
 */

#ifndef AUDIO_FRONT_END_DEFINES_H__
#define AUDIO_FRONT_END_DEFINES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cy_afe_configurator_settings.h"
#include "cy_afe_types_compat.h"

/******************************************************
 *                     Macros
 ******************************************************/
/* Mapping of configurator file defines. This is done to avoid any code change
 * if defines in configurator generated file changes */
#define CY_AFE_INPUT_NUM_OF_CHANNELS (AFE_INPUT_NUMBER_CHANNELS)
#define CY_AFE_FRAME_SIZE_MS         (AFE_FRAME_SIZE_MS)
#define CY_AFE_SAMPLE_FREQ           (AFE_FRAME_RATE_SPS)

#ifdef ENABLE_IFX_AEC
#define CY_AFE_AEC_ENABLED
#endif

#ifdef AFE_USE_BEAM_FORMING
#define CY_AFE_BEAM_FORMING_ENABLED
#endif

#ifdef AFE_USE_HIGH_PASS_FILTER
#define CY_AFE_HPF_ENABLED
#endif

#ifdef AFE_USE_DEREVERBERATION
#define CY_AFE_DR_ENABLED
#endif

#ifdef AFE_USE_NOISE_SUPPRESSION
#define CY_AFE_NS_ENABLED
#endif

#ifdef AFE_USE_ECHO_SUPPRESSION
#define CY_AFE_ES_ENABLED
#endif

#ifdef AFE_USE_INTERFERENCE_CANCELLER
#define CY_AFE_IC_ENABLED
#endif

/******************************************************
 *                    Constants
 ******************************************************/
#define CY_AFE_SAMPLE_WIDTH_SIZE     (16)
#define CY_AFE_AUDIO_MONO_CHANNEL    (1)
#define CY_AFE_AUDIO_STEREO_CHANNEL  (2)

#define CY_AFE_MONO_FRAME_SIZE_IN_BYTES     (320)
#define CY_AFE_STEREO_FRAME_SIZE_IN_BYTES   (640)
#define CY_AFE_MONITOR_OUT_MAX_SIZE 		(4 * CY_AFE_MONO_FRAME_SIZE_IN_BYTES)
#define CY_AFE_AUDIO_METER_MAX 	        	(3)

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_FRONT_END_DEFINES_H__ */

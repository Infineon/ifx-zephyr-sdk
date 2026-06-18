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

/** @file cy_audio_front_end_error.h
 *  Defines the Audio front end middleware error codes.
 */

#ifndef INCLUDED_AUDIO_FRONT_END_ERROR_H_
#define INCLUDED_AUDIO_FRONT_END_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cy_result.h"
#include "cy_result_mw.h"


/**
 * \defgroup group_afe_results Audio front end results/error codes
 * @ingroup group_afe_macros
 *
 * AFE middleware APIs return results of type cy_rslt_t and consist of three parts:
 * - module base
 * - type
 * - error code
 *
 * \par Result Format
 *
   \verbatim
              Module base                   Type    Library-specific error code
      +-----------------------------------+------+------------------------------+
      |CY_RSLT_MODULE_AFE_BASE             | 0x2  |           Error Code         |
      +-----------------------------------+------+------------------------------+
                14 bits                    2 bits            16 bits

   See the macro section of this document for library-specific error codes.
   \endverbatim
 *
 * The data structure cy_rslt_t is part of cy_result.h located in <core_lib/include>
 *
 * Module base: This base is derived from CY_RSLT_MODULE_MIDDLEWARE_BASE (defined in cy_result.h) and is an offset of the CY_RSLT_MODULE_MIDDLEWARE_BASE.
 *              The details of the offset and the middleware base are defined in cy_result_mw.h, which is part of the [GitHub connectivity-utilities] (https://github.com/Infineon/connectivity-utilities) repo.
 *              For example, the AFE library uses CY_RSLT_MODULE_AFE_BASE as the module base.
 *
 * Type: This type is defined in cy_result.h and can be one of CY_RSLT_TYPE_FATAL, CY_RSLT_TYPE_ERROR, CY_RSLT_TYPE_WARNING, or CY_RSLT_TYPE_INFO. AWS library error codes are of type CY_RSLT_TYPE_ERROR.
 *
 * Library-specific error code: These error codes are library-specific and defined in the macro section.
 *
 * Helper macros used for creating the library-specific result are provided as part of cy_result.h.
 * \{
 */

/** Audio front end error code base. */
#define CY_RSLT_AFE_ERR_BASE            CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_AFE_BASE, 0)

/** Audio front end out of memory */
#define CY_RSLT_AFE_OUT_OF_MEMORY                ( CY_RSLT_AFE_ERR_BASE + 1 )
/** Audio front end generic module error */
#define CY_RSLT_AFE_GENERIC_ERROR                ( CY_RSLT_AFE_ERR_BASE + 2 )
/** Audio front end bad argument error */
#define CY_RSLT_AFE_BAD_ARG                      ( CY_RSLT_AFE_ERR_BASE + 3 )
/** Audio front end middleware already initialized */
#define CY_RSLT_AFE_ALREADY_INITIALIZED          ( CY_RSLT_AFE_ERR_BASE + 4 )
/** Audio front end CRC checksum error */
#define CY_RSLT_AFE_CRC_CHECKSUM_ERROR           ( CY_RSLT_AFE_ERR_BASE + 5 )
/** Audio front end speech enhancement error */
#define CY_RSLT_AFE_SPEECH_ENHANCEMENT_ERROR     ( CY_RSLT_AFE_ERR_BASE + 6 )
/** Audio front end invalid tuner command */
#define CY_RSLT_AFE_TUNER_INALID_CMD             ( CY_RSLT_AFE_ERR_BASE + 7 )
/** Audio front end tuner generic error */
#define CY_RSLT_AFE_TUNER_GENERIC_ERROR          ( CY_RSLT_AFE_ERR_BASE + 8 )
/** Audio front end tuner need more data from configurator tool/application */
#define CY_RSLT_AFE_TUNER_NEED_MORE_DATA         ( CY_RSLT_AFE_ERR_BASE + 9 )
/** Audio front end tuner internal error */
#define CY_RSLT_AFE_TUNER_INTERNAL_ERROR         ( CY_RSLT_AFE_ERR_BASE + 10 )
/** Audio front end tuner wait for poll timeout before invoking tuner receive callback again */
#define CY_RSLT_AFE_TUNER_WAIT_FOR_POLL_TIMEOUT  ( CY_RSLT_AFE_ERR_BASE + 11 )
/** Audio front end system module error */
#define CY_RSLT_AFE_SYSTEM_MODULE_ERROR          ( CY_RSLT_AFE_ERR_BASE + 12 )
/** Audio front end invalid cmd params */
#define CY_RSLT_AFE_TUNER_INVALID_CMD_PARAMS     ( CY_RSLT_AFE_ERR_BASE + 13 )
/** Audio front end component not enabled */
#define CY_RSLT_AFE_TUNER_COMPONENT_NOT_ENABLED  ( CY_RSLT_AFE_ERR_BASE + 14 )
/** Audio front end component not supported */
#define CY_RSLT_AFE_TUNER_CMD_NOT_SUPPORTED      ( CY_RSLT_AFE_ERR_BASE + 15 )
/** Audio front end functionality restricted */
#define CY_RSLT_AFE_FUNCTIONALITY_RESTRICTED      ( CY_RSLT_AFE_ERR_BASE + 16 )
/** Audio front end hw input gain out of range */
#define CY_RSLT_AFE_TUNER_HW_INPUT_GAIN_OUT_OF_RANGE    ( CY_RSLT_AFE_ERR_BASE + 17 )

/** \} group_afe_macros */
#ifdef __cplusplus
} /*extern "C" */
#endif
#endif /* ifndef INCLUDED_AUDIO_FRONT_END_ERROR_H_ */

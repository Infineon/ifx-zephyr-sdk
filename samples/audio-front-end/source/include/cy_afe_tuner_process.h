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
 * @file cy_afe_tuner_process.h
 * @brief Set of AFE tuner routines to process the tuner config messages
 *
 */

#ifndef AUDIO_FRONT_END_TUNER_PROCESS_H__
#define AUDIO_FRONT_END_TUNER_PROCESS_H__

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
#include "cy_audio_front_end.h"
#include "cy_audio_front_end_error.h"
#include "cy_afe_audio_internal.h"

/******************************************************
 *                     Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
/**
 * Maximum command buffer size as request from configurator tool
 */
#define CY_AFE_TUNER_MAX_COMMAND_BUFFER_SIZE (500)

/** Maximum number of paramaters any tuner commands can have */
#define MAX_NUM_PARAMS                       (10)

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef cy_rslt_t (*command_function_t)(void *context, char** params, int params_cnt);
/******************************************************
 *                    Structures
 ******************************************************/
/**
 * Structure to hold the information for each tuner command.
 */
typedef struct
{
    const char* cmd_name;                   /**< Main command name (set/get). */
    const char* sub_cmd_name;               /**< Actual tuner command. */
    command_function_t command;             /**< Function that runs the command. */
    uint32_t arg_count;                     /**< Minimum number of arguments. */
    const char* delimit;                    /**< Custom string of characters that may delimit the arguments for this command - NULL value will use the default for the console. */
} cy_afe_tuner_cmd_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Static Functions
 ******************************************************/

/******************************************************
 *               Functions
 ******************************************************/
/**
 * Function to send the response back to configurator tool over UART by invoking app callback
 *
 * @param[in]  context      Audio front end middleware handle
 * @param[in]  status       Status string
 * @params[in] cmd_data     command data of specific command to be sent back to configurator tool
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t afe_send_tuner_command_res(afe_internal_context_t *context, const char *status, char *text);

/**
 * Function to read data from configurator tool over UART by invoking app callback
 *
 * @param[in]  context      Audio front end middleware handle
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t afe_receive_tuner_command_req(afe_internal_context_t* context);

/**
 * Get status string to configurator tool based on result
 *
 * @param[in]  result   result code
 *
 * @return    Pointer to constant status string
 */
const char* get_status_string_from_result(cy_rslt_t result);

#endif

#endif /* AUDIO_FRONT_END_TUNER_PROCESS_H__ */

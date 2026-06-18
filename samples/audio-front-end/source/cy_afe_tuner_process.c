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
 * @file cy_afe_tuner_process.c
 * @brief Set of AFE tuner routines to receive the command, process and send response back to configurator tool
 *
 */

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
#include "cy_audio_front_end.h"
#include "include/cy_afe_audio_internal.h"
#include "include/cy_afe_tuner_process.h"

/******************************************************
 *                     Macros
 ******************************************************/
#define NEWLINE_AT_END_OF_CMD (2) /* \r\n at the end of command */

#define AFE_CRLF_LEN                (2)
#define AFE_CRLF                    "\r\n"
#define AFE_RESPONSE_HEADER         "AFERSP,"
#define AFE_RESPONSE_HEADER_LEN     (sizeof(AFE_RESPONSE_HEADER)-1)

#ifndef MIN
#define MIN(a,b) (a < b ? a : b)
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
extern cy_afe_tuner_cmd_t cmds[];

/******************************************************
 *               Static Functions
 ******************************************************/
static cy_rslt_t afe_process_tuner_command_req(afe_internal_context_t* context, uint8_t* req_cmd, uint16_t req_len);

/******************************************************
 *               Functions
 ******************************************************/
static uint8_t error_buf[32];
/**
 * Get corresponding string for the result to send it to configurator tool
 */
const char* get_status_string_from_result(cy_rslt_t result)
{
    char* str = NULL;

    switch(result)
    {
        case CY_RSLT_SUCCESS:
        {
            str = "ok";
        }
        break;
        case CY_RSLT_AFE_TUNER_INALID_CMD:
        {
            str = "invalid_cmd";
        }
        break;
        case CY_RSLT_AFE_TUNER_GENERIC_ERROR:
        {
            str = "error";
        }
        break;
        case CY_RSLT_AFE_TUNER_INTERNAL_ERROR:
        {
            str = "internal_error";
        }
        break;
        case CY_RSLT_AFE_TUNER_INVALID_CMD_PARAMS:
        {
            str = "invalid_cmd_params";
        }
        break;
        case CY_RSLT_AFE_TUNER_COMPONENT_NOT_ENABLED:
        {
            str = "comp_not_enabled";
        }
        break;
        case CY_RSLT_AFE_TUNER_CMD_NOT_SUPPORTED:
        {
            str = "not_supported";
        }
        break;
        case CY_RSLT_AFE_TUNER_HW_INPUT_GAIN_OUT_OF_RANGE:
        {
            int32_t min_range= AFE_MIN_MIC_INPUT_HW_GAIN * 2;
            int32_t max_range=  AFE_MAX_MIC_INPUT_HW_GAIN * 2;
            snprintf((char*)error_buf, sizeof(error_buf), "error,%"PRIi32",%"PRIi32, min_range, max_range);
            str = (char*)error_buf;
        }
        break;
        default:
        {
            str = "internal_error";
        }
        break;
    }

    return str;
}

/**
 * Function to send response back to configurator tool
 */
cy_rslt_t afe_send_tuner_command_res(afe_internal_context_t *context, const char* status, char* text)
{
    cy_afe_tuner_buffer_t response_buffer;
    uint8_t *res = NULL;
    int length = 0;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    memset(&response_buffer, 0, sizeof(cy_afe_tuner_buffer_t));
    memset(context->afe_response_buffer, 0, sizeof(context->afe_response_buffer));

    res = context->afe_response_buffer;
    response_buffer.buffer = res;

    if(strlen(status + AFE_CRLF_LEN + AFE_RESPONSE_HEADER_LEN) > (sizeof(context->afe_response_buffer)-1))
    {
        result = CY_RSLT_AFE_TUNER_INTERNAL_ERROR;
        cy_afe_log_err(result, "Sizeof AFE response buffer is not sufficient");
        return result;
    }

    /* AFE response header */
    memcpy(res, AFE_RESPONSE_HEADER, AFE_RESPONSE_HEADER_LEN);
    length = length + AFE_RESPONSE_HEADER_LEN;
    res = res + AFE_RESPONSE_HEADER_LEN;

    /* Command response */
    memcpy(res, status, strlen(status));
    length = length + strlen(status);
    res = res + strlen(status);

    /* Added response text, if any */
    if(NULL != text)
    {
        if((strlen(status)+ strlen(text) + AFE_CRLF_LEN) > (sizeof(context->afe_response_buffer)-1))
        {
            result = CY_RSLT_AFE_TUNER_INTERNAL_ERROR;
            cy_afe_log_err(result, "Sizeof AFE response buffer is not sufficient");
            return result;
        }

        *res = ',';
        res++;
        length = length + 1;

        memcpy(res, text, strlen(text));
        res = res + strlen(text);
        length = length + strlen(text);
    }

    /* Added <CRLF> ending delimiter  */
    memcpy(res, AFE_CRLF, AFE_CRLF_LEN);
    length = length + AFE_CRLF_LEN;

    response_buffer.length = length;

#ifdef ENABLE_AFE_MW_TUNER_CHECK_POINT
    AFE_MW_TUNER_CHECK_POINT()
#endif

    /**
     * Invoke application registered callback to send response
     */
    context->tuner_callbacks.write_response_callback(context, &response_buffer, context->config_init.user_arg_callbacks);

#ifdef ENABLE_AFE_MW_TUNER_CHECK_POINT
    AFE_MW_TUNER_CHECK_POINT()
#endif

    return CY_RSLT_SUCCESS;
}

/**
 * Process received tuner command request from configurator tool
 */
static cy_rslt_t afe_process_tuner_command_req(afe_internal_context_t* context, uint8_t* req_cmd, uint16_t req_len)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_afe_tuner_cmd_t *cmd = NULL;
    char **params = context->tuner_cmd_params;
    char* saveptr = NULL;
    int params_cnt = 0;
    bool cmd_found = false;

    char *copy = malloc (req_len + 1);
    if ( copy == NULL )
    {
        return CY_RSLT_AFE_OUT_OF_MEMORY;
    }

    cy_afe_log_dbg("Tuner command received:%.*s", req_len, req_cmd);

    memset(copy, 0, req_len+ 1);
    memcpy(copy, req_cmd, req_len);

    params[params_cnt++] = strtok_r(copy,",", &saveptr);
    params[params_cnt++] = strtok_r( NULL, (char *) ",", &saveptr);

    for(cmd = cmds; cmd->cmd_name != NULL; cmd++)
    {
        if(NULL != cmd->sub_cmd_name)
        {
            if((strcmp(cmd->cmd_name, params[0]) == 0) && (strcmp(cmd->sub_cmd_name, params[1]) == 0))
            {
                cmd_found = true;
                break;
            }
        }
        else
        {
            if( strcmp(cmd->cmd_name, params[0]) == 0 )
            {
                cmd_found = true;
                break;
            }
        }
    }

    /** If command is not found, send error to configurator tool */
    if(false == cmd_found)
    {
        result = afe_send_tuner_command_res(context, get_status_string_from_result(CY_RSLT_AFE_TUNER_INALID_CMD), NULL);
        if(CY_RSLT_SUCCESS != result)
        {
            /* Fall through. Continue processing other commands ??*/
            cy_afe_log_err(result, "Failed to send response to configurator tool");
        }
    }
    else
    {
        /* parse arguments */
        while ( saveptr != NULL && saveptr[ 0 ] != '\0' )
        {
            params[params_cnt++] = strtok_r( NULL, (char *) ",", &saveptr );
        }

        cmd->command(context, params, params_cnt);
    }

    free(copy);

    return CY_RSLT_SUCCESS;
}

/**
 * Check for newline(\r\n) in the received command
 */
static bool check_for_newline_in_received_data(uint8_t* data, uint16_t length)
{
    if(length > 2)
    {
        if(data[length-2] == '\r' && data[length-1] == '\n')
        {
            return true;
        }
    }

    return false;
}

/**
 * Receive tuner command request from configurator tool
 */
cy_rslt_t afe_receive_tuner_command_req(afe_internal_context_t* context)
{
    cy_afe_tuner_buffer_t request_buffer;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    memset(&request_buffer, 0, sizeof(cy_afe_tuner_buffer_t));

    request_buffer.buffer = context->afe_request_buffer;
    request_buffer.buffer_max_len = CY_AFE_TUNER_MAX_REQUEST_BUFFER_SIZE;

#ifdef ENABLE_AFE_MW_TUNER_CHECK_POINT
    AFE_MW_TUNER_CHECK_POINT()
#endif

    /** Invoke read callback to get the commands from configurator tool */
    result = context->tuner_callbacks.read_request_callback(context, &request_buffer, context->config_init.user_arg_callbacks);

#ifdef ENABLE_AFE_MW_TUNER_CHECK_POINT
    AFE_MW_TUNER_CHECK_POINT()
#endif

    if(CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "Tuner read request callback failed");
        return result;
    }

    /**
     * If application did not provide any data in previous read call, then call next read after
     * poll timeout configured
     */
    if(request_buffer.length == 0)
    {
        return CY_RSLT_AFE_TUNER_WAIT_FOR_POLL_TIMEOUT;
    }

    /**
     * Store incoming data from application to local command buffer for further processing
     */
    memcpy(context->internal_request_cmd_buffer + context->internal_request_cmd_len, request_buffer.buffer, MIN(request_buffer.length, CY_AFE_TUNER_MAX_COMMAND_BUFFER_SIZE));
    context->internal_request_cmd_len = (context->internal_request_cmd_len + request_buffer.length);

    /**
     * Check if command received ends with \r\n, if yes, then start processing command
     */
    if(true == check_for_newline_in_received_data(context->internal_request_cmd_buffer, context->internal_request_cmd_len))
    {
        result = afe_process_tuner_command_req(context, context->internal_request_cmd_buffer, (context->internal_request_cmd_len - NEWLINE_AT_END_OF_CMD));
        if (CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to process tuner commands");
            return result;
        }

        memset(context->internal_request_cmd_buffer, 0, CY_AFE_TUNER_MAX_COMMAND_BUFFER_SIZE);
        context->internal_request_cmd_len = 0;
    }
    else
    {
        return CY_RSLT_AFE_TUNER_NEED_MORE_DATA;
    }

    return CY_RSLT_SUCCESS;
}
#endif

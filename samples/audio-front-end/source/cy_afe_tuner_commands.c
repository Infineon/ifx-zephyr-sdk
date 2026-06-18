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
 * @file cy_afe_tuner_commands.c
 * @brief Implementation of supported tuner commands
 *
 */

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
#include "cy_audio_front_end.h"
#include "cy_afe_audio_internal.h"
#include "cy_afe_tuner_process.h"
#include "cy_afe_audio_speech_enh.h"
#include "cy_afe_audio_bd_calc.h"

/******************************************************
 *                     Macros
 ******************************************************/
#define SET_CMD "set"
#define GET_CMD "get"

#define SET_START_STREAM "start_stream"
#define SET_STOP_STREAM "stop_stream"
#define AFE_SOUND_METER "sound_meter"

#define COMPONENT_ED_NAME_IC    "comp_bf"
#define COMPONENT_ED_NAME_DR    "comp_dr"
#define COMPONENT_ED_NAME_NS    "comp_ns"
#define COMPONENT_ED_NAME_ES    "comp_es"
#define COMPONENT_ED_NAME_AEC   "comp_aec"
#define COMPONENT_ED_NAME_NS_ES "comp_ns_es"

#define COMPONENT_NAME_HPF_PARAM    "hpf_param"
#define COMPONENT_NAME_AEC_PARAM    "aec_param"
#define COMPONENT_NAME_BF_PARAM     "bf_param"
#define COMPONENT_NAME_ES_PARAM     "es_param"
#define COMPONENT_NAME_NS_PARAM     "ns_param"
#define COMPONENT_NAME_INPUT_PARAM  "input_param"

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


const static struct {
    ifx_sp_enh_ip_component_config_t val1;
    ifx_sp_enh_ip_component_config_t val2;
    const char *str;
} sp_enh_ip_component_id_conversion [] = {
    {IFX_SP_ENH_IP_COMPONENT_BF, IFX_SP_ENH_IP_COMPONENT_INVALID, COMPONENT_ED_NAME_IC},
    {IFX_SP_ENH_IP_COMPONENT_DRVB, IFX_SP_ENH_IP_COMPONENT_INVALID, COMPONENT_ED_NAME_DR},
#ifdef ENABLE_IFX_NS
    {IFX_SP_ENH_IP_COMPONENT_NS, IFX_SP_ENH_IP_COMPONENT_INVALID, COMPONENT_ED_NAME_NS},
#else
    {IFX_SP_ENH_IP_COMPONENT_DSNS, IFX_SP_ENH_IP_COMPONENT_INVALID, COMPONENT_ED_NAME_NS},
#endif
#ifdef ENABLE_IFX_ES
    {IFX_SP_ENH_IP_COMPONENT_ES, IFX_SP_ENH_IP_COMPONENT_INVALID, COMPONENT_ED_NAME_ES},
#else
    {IFX_SP_ENH_IP_COMPONENT_DSES, IFX_SP_ENH_IP_COMPONENT_INVALID, COMPONENT_ED_NAME_ES},
#endif
    {IFX_SP_ENH_IP_COMPONENT_AEC, IFX_SP_ENH_IP_COMPONENT_INVALID, COMPONENT_ED_NAME_AEC},
    // {IFX_SP_ENH_IP_COMPONENT_NS, IFX_SP_ENH_IP_COMPONENT_ES, COMPONENT_ED_NAME_NS_ES},
};

const static struct {
    const char *str;
    uint32_t val;
} afe_level_indicator[] = {
    {"low", 0},
    {"medium",1},
    {"high",2},
    {"NULL",3},
};

const static struct {
    ifx_sp_enh_ip_component_config_t val;
    const char *str;
} sp_enh_ip_get_param_component_id_conversion [] = {
    {IFX_SP_ENH_IP_COMPONENT_HPF, COMPONENT_NAME_HPF_PARAM},
    {IFX_SP_ENH_IP_COMPONENT_AEC, COMPONENT_NAME_AEC_PARAM},
    {IFX_SP_ENH_IP_COMPONENT_BF, COMPONENT_NAME_BF_PARAM},
#ifdef ENABLE_IFX_NS
    {IFX_SP_ENH_IP_COMPONENT_NS, COMPONENT_NAME_NS_PARAM},
#else
    {IFX_SP_ENH_IP_COMPONENT_DSNS, COMPONENT_NAME_NS_PARAM},
#endif
#ifdef ENABLE_IFX_ES
    {IFX_SP_ENH_IP_COMPONENT_ES, COMPONENT_NAME_ES_PARAM},
#else
    {IFX_SP_ENH_IP_COMPONENT_DSES, COMPONENT_NAME_ES_PARAM},
#endif
};



static cy_rslt_t config_input_gain(void *context, char** params, int params_cnt);
static cy_rslt_t config_hpf(void *context, char** params, int params_cnt);
static cy_rslt_t config_bulk_delay(void *context, char** params, int params_cnt);
static cy_rslt_t config_ic(void *context, char** params, int params_cnt);
static cy_rslt_t config_es(void *context, char** params, int params_cnt);
static cy_rslt_t config_ns(void *context, char** params, int params_cnt);
static cy_rslt_t config_enable_or_disable_module(void *context, char** params, int params_cnt);
static cy_rslt_t config_start_stop_stream(void *context, char** params, int params_cnt);
static cy_rslt_t config_set_audio_channels(void *context, char** params, int params_cnt);
static cy_rslt_t config_get_audio_channels(void *context, char** params, int params_cnt);
static cy_rslt_t config_start_aec_calibration(void *context, char** params, int params_cnt);
static cy_rslt_t config_get_sound_meter(void *context, char** params, int params_cnt);
static cy_rslt_t config_get_component_params(void *context, char** params, int params_cnt);
static cy_rslt_t config_get_input_params(void *context, char** params, int params_cnt);

/******************************************************
 *                 Global Variables
 ******************************************************/
cy_afe_tuner_cmd_t cmds[]=
{
    {SET_CMD, "input_gain",                   config_input_gain,                  1, ","},
    {GET_CMD, "input_gain",                   config_input_gain,                  0, ","},
    {SET_CMD, "hpf",                          config_hpf,                         1, ","}, //ToDo:- Do not support freq, need to enable once algo supported
    {GET_CMD, "hpf",                          config_hpf,                         0, ","},
    {SET_CMD, "bulk_delay",                   config_bulk_delay,                  1, ","},
    {GET_CMD, "bulk_delay",                   config_bulk_delay,                  0, ","},
    {GET_CMD, "ic",                           config_ic,                          0, ","},
    {SET_CMD, "ic",                           config_ic,                          1, ","},
    {GET_CMD, "es",                           config_es,                          0, ","},
    {SET_CMD, "es",                           config_es,                          1, ","},
    {GET_CMD, "ns",                           config_ns,                          0, ","},
    {SET_CMD, "ns",                           config_ns,                          1, ","},
    {SET_CMD, COMPONENT_ED_NAME_IC,           config_enable_or_disable_module,    1, ","},
    {GET_CMD, COMPONENT_ED_NAME_IC,           config_enable_or_disable_module,    0, ","},
    {SET_CMD, COMPONENT_ED_NAME_DR,           config_enable_or_disable_module,    1, ","},
    {GET_CMD, COMPONENT_ED_NAME_DR,           config_enable_or_disable_module,    0, ","},
    // {SET_CMD, COMPONENT_ED_NAME_NS_ES,        config_enable_or_disable_module,    1, ","},
    // {GET_CMD, COMPONENT_ED_NAME_NS_ES,        config_enable_or_disable_module,    0, ","},
    {SET_CMD, COMPONENT_ED_NAME_NS,           config_enable_or_disable_module,    1, ","},
    {GET_CMD, COMPONENT_ED_NAME_NS,           config_enable_or_disable_module,    0, ","},
    {SET_CMD, COMPONENT_ED_NAME_ES,           config_enable_or_disable_module,    1, ","},
    {GET_CMD, COMPONENT_ED_NAME_ES,           config_enable_or_disable_module,    0, ","},
    {SET_CMD, COMPONENT_ED_NAME_AEC,          config_enable_or_disable_module,    1, ","},
    {GET_CMD, COMPONENT_ED_NAME_AEC,          config_enable_or_disable_module,    0, ","},
    {SET_START_STREAM, NULL,                  config_start_stop_stream,           0, ","},
    {SET_STOP_STREAM,  NULL,                  config_start_stop_stream,           0, ","},
    {SET_CMD, "audio-channels",               config_set_audio_channels,          4, ","},
    {GET_CMD, "audio-channels",               config_get_audio_channels,          0, ","},
    {"start", "aec-calibration",              config_start_aec_calibration,       0, ","},
    {GET_CMD, AFE_SOUND_METER,                config_get_sound_meter,             0, ","},
    {GET_CMD, COMPONENT_NAME_HPF_PARAM,       config_get_component_params,        0, ","},
    {GET_CMD, COMPONENT_NAME_AEC_PARAM,       config_get_component_params,        0, ","},
    {GET_CMD, COMPONENT_NAME_BF_PARAM,        config_get_component_params,        0, ","},
    {GET_CMD, COMPONENT_NAME_ES_PARAM,        config_get_component_params,        0, ","},
    {GET_CMD, COMPONENT_NAME_NS_PARAM,        config_get_component_params,        0, ","},
    {GET_CMD, COMPONENT_NAME_INPUT_PARAM,     config_get_input_params,            0, ","},
    { NULL, NULL, NULL, 0, NULL}
};


/******************************************************
 *               Static Functions
 ******************************************************/

/******************************************************
 *               Functions
 ******************************************************/

static cy_rslt_t afe_convert_level_string_to_value(const char *str, uint32_t * val)
{
    int j;
    for (j = 0;  j < sizeof (afe_level_indicator) / sizeof (afe_level_indicator[0]);  ++j)
     {
         if (!strcmp (str, afe_level_indicator[j].str))
         {
             *val =  afe_level_indicator[j].val;
             return CY_RSLT_SUCCESS;
         }
     }

     return CY_RSLT_AFE_TUNER_INVALID_CMD_PARAMS;
}

static cy_rslt_t afe_str_to_sp_enh_ip_component_id (const char *str, ifx_sp_enh_ip_component_config_t *comp1, ifx_sp_enh_ip_component_config_t *comp2)
{
     int j;
     *comp1 = *comp2 = IFX_SP_ENH_IP_COMPONENT_INVALID;

     for (j = 0;  j < sizeof (sp_enh_ip_component_id_conversion) / sizeof (sp_enh_ip_component_id_conversion[0]);  ++j)
     {
         if (!strcmp (str, sp_enh_ip_component_id_conversion[j].str))
         {
             *comp1 =  sp_enh_ip_component_id_conversion[j].val1;
             *comp2 =  sp_enh_ip_component_id_conversion[j].val2;
             return CY_RSLT_SUCCESS;
         }
     }

     return CY_RSLT_SUCCESS;
}

static cy_rslt_t config_start_stop_stream(void *context, char** params, int params_cnt)
{
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    cy_afe_config_setting_t config_setting;
    cy_rslt_t result;
    int stream_state; /*  1 - start stream, 0 - stop  stream */
    char data[12];

    memset(data, 0, sizeof(data));
    memset(&config_setting, 0, sizeof(cy_afe_config_setting_t));

    if(strcmp(SET_START_STREAM, params[0]) == 0)
    {
        stream_state = 1; /* Start steam */
    }
    else
    {
        stream_state = 0; /* Stop steam */
    }

    config_setting.action = CY_AFE_UPDATE_CONFIG;
    config_setting.config_name = CY_AFE_CONFIG_STREAM;
    config_setting.value = (int*) &stream_state;

    result = handle->tuner_callbacks.notify_settings_callback(handle, &config_setting,
            handle->config_init.user_arg_callbacks);
    if(CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "Failed to write input gain from application");
        result = CY_RSLT_AFE_TUNER_INTERNAL_ERROR;
        goto send_response;
    }
    else
    {
        afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    }

    return CY_RSLT_SUCCESS;

send_response:
    afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    return CY_RSLT_SUCCESS;
}

static cy_rslt_t config_input_gain(void *context, char** params, int params_cnt)
{
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    cy_afe_config_setting_t config_setting;
    cy_rslt_t result;
    int input_gain;
    char data[12];

    memset(data, 0, sizeof(data));
    memset(&config_setting, 0, sizeof(cy_afe_config_setting_t));

    if(strcmp(SET_CMD, params[0]) == 0)
    {
        config_setting.action = CY_AFE_UPDATE_CONFIG;
        config_setting.config_name = CY_AFE_CONFIG_INPUT_GAIN;
        input_gain = atoi(params[2]);
        config_setting.value = (int*) &input_gain;

#ifdef ENABLE_AFE_MW_TUNER_CHECK_POINT
    AFE_MW_TUNER_CHECK_POINT()
#endif

        result = handle->tuner_callbacks.notify_settings_callback(handle, &config_setting,
                handle->config_init.user_arg_callbacks);

#ifdef ENABLE_AFE_MW_TUNER_CHECK_POINT
    AFE_MW_TUNER_CHECK_POINT()
#endif

        if(CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to write input gain from application");
            goto send_response;
        }
        else
        {
            afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
        }
    }
    else
    {
        config_setting.action = CY_AFE_READ_CONFIG;
        config_setting.config_name = CY_AFE_CONFIG_INPUT_GAIN;
        config_setting.value = (int*) &input_gain;

#ifdef ENABLE_AFE_MW_TUNER_CHECK_POINT
    AFE_MW_TUNER_CHECK_POINT()
#endif

        result = handle->tuner_callbacks.notify_settings_callback(handle, &config_setting,
                handle->config_init.user_arg_callbacks);

#ifdef ENABLE_AFE_MW_TUNER_CHECK_POINT
    AFE_MW_TUNER_CHECK_POINT()
#endif

        if(CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to read input gain from application");
            goto send_response;
        }
        else
        {
            sprintf(data, "%d", input_gain);
            afe_send_tuner_command_res(handle, get_status_string_from_result(result), data);
        }
    }

    return CY_RSLT_SUCCESS;

send_response:
    afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    return CY_RSLT_SUCCESS;
}
static cy_rslt_t config_hpf(void *context, char** params, int params_cnt)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    cy_afe_config_setting_t config_setting;
    int hpf;
    char data[12];

    memset(data, 0, sizeof(data));
    memset(&config_setting, 0, sizeof(cy_afe_config_setting_t));

    if(strcmp(SET_CMD, params[0]) == 0)
    {
        hpf = atoi(params[2]);
#ifdef NOT_SUPPORTED_IN_ALGO
        result = afe_speech_enhancement_update_config_value(context, IFX_SP_ENH_IP_COMPONENT_HPF, hpf);
        if(CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to update HPF value");
            goto send_response;
        }

        config_setting.action = CY_AFE_NOTIFY_CONFIG;
        config_setting.config_name = CY_AFE_CONFIG_HPF;
        config_setting.value = (int*) &hpf;

#ifdef ENABLE_AFE_MW_TUNER_CHECK_POINT
    AFE_MW_TUNER_CHECK_POINT()
#endif

        handle->tuner_callbacks.notify_settings_callback(handle, &config_setting,
                handle->config_init.user_arg_callbacks);

#ifdef ENABLE_AFE_MW_TUNER_CHECK_POINT
    AFE_MW_TUNER_CHECK_POINT()
#endif

        afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
#else
        /* {{{ Temporary setting to not supported as the algo do not support high pass filter freq change */
        result = CY_RSLT_AFE_TUNER_CMD_NOT_SUPPORTED;
        goto send_response;
        /* }}} */
#endif
    }
    else
    {
        result = afe_speech_enhancement_get_config_value(context, IFX_SP_ENH_IP_COMPONENT_HPF, (int32_t*)&hpf);
        if(CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to update HPF value");
            goto send_response;
        }

        snprintf( data, sizeof(data) - 1, "%d", hpf );
        afe_send_tuner_command_res(handle, get_status_string_from_result(result), data);
    }

    return CY_RSLT_SUCCESS;

send_response:
    afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    return CY_RSLT_SUCCESS;
}

static cy_rslt_t config_bulk_delay(void *context, char** params, int params_cnt)
{
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    cy_afe_config_setting_t config_setting;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    int bulk_delay;
    char data[12];

    memset(data, 0, sizeof(data));
    memset(&config_setting, 0, sizeof(cy_afe_config_setting_t));

    if(strcmp(SET_CMD, params[0]) == 0)
    {
        bulk_delay = atoi(params[2]);
        result = afe_speech_enhancement_update_config_value(context, IFX_SP_ENH_IP_COMPONENT_AEC, bulk_delay);
        if(CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to update bulk_delay value");
            goto send_response;
        }

        config_setting.action = CY_AFE_NOTIFY_CONFIG;
        config_setting.config_name = CY_AFE_CONFIG_AEC_BULK_DELAY;
        config_setting.value = (int*) &bulk_delay;

#ifdef ENABLE_AFE_MW_TUNER_CHECK_POINT
    AFE_MW_TUNER_CHECK_POINT()
#endif

        handle->tuner_callbacks.notify_settings_callback(handle, &config_setting,
                handle->config_init.user_arg_callbacks);

#ifdef ENABLE_AFE_MW_TUNER_CHECK_POINT
    AFE_MW_TUNER_CHECK_POINT()
#endif

        afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    }
    else
    {
        result = afe_speech_enhancement_get_config_value(context, IFX_SP_ENH_IP_COMPONENT_AEC, (int32_t*)&bulk_delay);
        if(CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to update AEC value");
            goto send_response;
        }

        snprintf( data, sizeof(data) - 1, "%d", bulk_delay);
        afe_send_tuner_command_res(handle, get_status_string_from_result(result), data);
    }

    return CY_RSLT_SUCCESS;

send_response:
    afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    return CY_RSLT_SUCCESS;
}

static cy_rslt_t config_ic(void *context, char** params, int params_cnt)
{
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    cy_afe_config_setting_t config_setting;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    uint32_t aggressiveness = 0;
    char data[12];

    memset(data, 0, sizeof(data));
    memset(&config_setting, 0, sizeof(cy_afe_config_setting_t));

    if(strcmp(SET_CMD, params[0]) == 0)
    {
        result = afe_convert_level_string_to_value(params[2],&aggressiveness);
        if(result != CY_RSLT_SUCCESS)
        {
            cy_afe_log_err(result, "Invalid param for BF/IC");
            goto send_response;
        }

        result = afe_speech_enhancement_update_config_value(context, IFX_SP_ENH_IP_COMPONENT_BF, aggressiveness);
        if(CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to update suppression_gain_db value");
            goto send_response;
        }
        /* TODO use system API to update the config */
        config_setting.action = CY_AFE_NOTIFY_CONFIG;
        config_setting.config_name = CY_AFE_CONFIG_INFERENCE_CANCELLER;
        config_setting.value = (int*) &aggressiveness;

        handle->tuner_callbacks.notify_settings_callback(handle, &config_setting,
                handle->config_init.user_arg_callbacks);
        afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    }
    else
    {
        result = afe_speech_enhancement_get_config_value(context, IFX_SP_ENH_IP_COMPONENT_BF, (int32_t*)&aggressiveness);
        if(CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to update BF value");
            goto send_response;
        }

        snprintf( data, sizeof(data) - 1, "%s", (aggressiveness==0)?"low":(aggressiveness==1)?"medium":"high");
        afe_send_tuner_command_res(handle, get_status_string_from_result(result), data);
    }

    return CY_RSLT_SUCCESS;
send_response:
    afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    return CY_RSLT_SUCCESS;
}

static cy_rslt_t config_es(void *context, char** params, int params_cnt)
{
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    cy_afe_config_setting_t config_setting;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    uint32_t echo_suppressor = 0;
    char data[12];
#ifdef ENABLE_IFX_ES
    ifx_sp_enh_ip_component_config_t comp = IFX_SP_ENH_IP_COMPONENT_ES;
#else
    ifx_sp_enh_ip_component_config_t comp = IFX_SP_ENH_IP_COMPONENT_DSES;
#endif

    memset(data, 0, sizeof(data));
    memset(&config_setting, 0, sizeof(cy_afe_config_setting_t));

    if(strcmp(SET_CMD, params[0]) == 0)
    {
        result = afe_convert_level_string_to_value(params[2],&echo_suppressor);
        if(result != CY_RSLT_SUCCESS)
        {
            cy_afe_log_err(result, "Invalid param for ES");
            goto send_response;
        }

        result = afe_speech_enhancement_update_config_value(context, comp, echo_suppressor);
        if(CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to update suppression_gain_db value");
            goto send_response;
        }

        config_setting.action = CY_AFE_NOTIFY_CONFIG;
        config_setting.config_name = CY_AFE_CONFIG_ECHO_SUPPRESSOR;
        config_setting.value = (int*)&echo_suppressor;

        handle->tuner_callbacks.notify_settings_callback(handle, &config_setting,
                handle->config_init.user_arg_callbacks);
        afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    }
    else
    {
        result = afe_speech_enhancement_get_config_value(context, comp, (int32_t*)&echo_suppressor);
        if(CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to update ES value");
            goto send_response;
        }

        snprintf( data, sizeof(data) - 1, "%s", (echo_suppressor==0)?"low":(echo_suppressor==1)?"medium":"high");
        afe_send_tuner_command_res(handle, get_status_string_from_result(result), data);
    }

    return CY_RSLT_SUCCESS;

send_response:
    afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    return CY_RSLT_SUCCESS;
}

static cy_rslt_t config_ns(void *context, char** params, int params_cnt)
{
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    cy_afe_config_setting_t config_setting;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    int suppression_gain_db;
    char data[12];
#ifdef ENABLE_IFX_NS
    ifx_sp_enh_ip_component_config_t comp = IFX_SP_ENH_IP_COMPONENT_NS;
#else
    ifx_sp_enh_ip_component_config_t comp = IFX_SP_ENH_IP_COMPONENT_DSNS;
#endif

    memset(data, 0, sizeof(data));
    memset(&config_setting, 0, sizeof(cy_afe_config_setting_t));

    if(strcmp(SET_CMD, params[0]) == 0)
    {
        suppression_gain_db = atoi(params[2]);
        result = afe_speech_enhancement_update_config_value(context, comp, suppression_gain_db);
        if(CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to update suppression_gain_db value");
            goto send_response;
        }

        config_setting.action = CY_AFE_NOTIFY_CONFIG;
        config_setting.config_name = CY_AFE_CONFIG_NOISE_SUPPRESSOR;
        config_setting.value = (int*) &suppression_gain_db;

        handle->tuner_callbacks.notify_settings_callback(handle, &config_setting,
                handle->config_init.user_arg_callbacks);
        afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    }
    else
    {
        result = afe_speech_enhancement_get_config_value(context, comp, (int32_t*)&suppression_gain_db);
        if(CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to update NS value");
            goto send_response;
        }

        snprintf( data, sizeof(data) - 1, "%d", suppression_gain_db);
        afe_send_tuner_command_res(handle, get_status_string_from_result(result), data);
    }

    return CY_RSLT_SUCCESS;

send_response:
    afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    return CY_RSLT_SUCCESS;
}

static cy_rslt_t config_enable_or_disable_module(void *context, char** params, int params_cnt)
{
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    cy_afe_config_setting_t config_setting;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    char data[12];
    ifx_sp_enh_ip_component_config_t comp1, comp2;
    bool enable;

    memset(data, 0, sizeof(data));
    memset(&config_setting, 0, sizeof(cy_afe_config_setting_t));

    afe_str_to_sp_enh_ip_component_id(params[1],&comp1,&comp2);

    if( (comp1 == IFX_SP_ENH_IP_COMPONENT_INVALID) && (comp2 == IFX_SP_ENH_IP_COMPONENT_INVALID) )
    {
        result = CY_RSLT_AFE_TUNER_INVALID_CMD_PARAMS;
        goto send_response;
    }

    if(strcmp(SET_CMD, params[0]) == 0)
    {
        enable = (atoi(params[2]) == 0)?false:true;

        result = afe_speech_enhancement_enable_disable_component(context, comp1, enable);
        if(result != CY_RSLT_SUCCESS)
        {
            goto send_response;
        }

        if(comp2 != IFX_SP_ENH_IP_COMPONENT_INVALID)
        {
            result = afe_speech_enhancement_enable_disable_component(context, comp2, enable);
            if(result != CY_RSLT_SUCCESS)
            {
                goto send_response;
            }
        }
#if 0
        config_setting.action = CY_AFE_NOTIFY_CONFIG;
        config_setting.config_name = CY_AFE_CONFIG_COMPONENT_STATE;
        config_setting.value = (int*) &comp;

        handle->tuner_callbacks.notify_settings_callback(handle, &config_setting,
                handle->config_init.user_arg_callbacks);
#endif
        afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    }
    else
    {
        bool enable2;
        result = CY_RSLT_AFE_TUNER_INTERNAL_ERROR;

        result = afe_speech_enhancement_get_component_status(context, comp1, &enable);
        if(CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to get component status");
            goto send_response;
        }

        if(comp2 != IFX_SP_ENH_IP_COMPONENT_INVALID)
        {
            result = afe_speech_enhancement_get_component_status(context, comp2, &enable2);
            if(CY_RSLT_SUCCESS != result)
            {
                cy_afe_log_err(result, "Failed to get component status");
                goto send_response;
            }

            if(enable && !enable2)
            {
                enable = false;
            }
        }

        snprintf( data, sizeof(data) - 1, "%d", enable);

        afe_send_tuner_command_res(handle, get_status_string_from_result(result), data);
    }

    return CY_RSLT_SUCCESS;

send_response:
    afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    return CY_RSLT_SUCCESS;
}

static cy_rslt_t config_set_audio_channels(void *context, char** params, int params_cnt)
{
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    cy_rslt_t result;

    afe_usb_settings_t AFE_USB_SETTINGS  = {
            .channel_0 = AFE_USB_SELECT_INPUT_0, //AEC Out 0
            .channel_1 = AFE_USB_SELECT_SIG_A_1, //AEC Out 1
            .channel_2 = AFE_USB_SELECT_SIG_B,   //BF dbg
            .channel_3 = AFE_USB_SELECT_SIG_C    //DR dbg
    };

    //printf("\r\nconfig_set_audio_channels\r\n");

    AFE_USB_SETTINGS.channel_0 = atoi(params[2]);
    if(AFE_USB_SELECT_SIG_C < AFE_USB_SETTINGS.channel_0)
    {
        result = CY_RSLT_AFE_TUNER_INALID_CMD;
        goto send_response;
    }
    AFE_USB_SETTINGS.channel_1 = atoi(params[3]);
    if(AFE_USB_SELECT_SIG_C < AFE_USB_SETTINGS.channel_1)
    {
        result = CY_RSLT_AFE_TUNER_INALID_CMD;
        goto send_response;
    }
    AFE_USB_SETTINGS.channel_2 = atoi(params[4]);
    if(AFE_USB_SELECT_SIG_C < AFE_USB_SETTINGS.channel_2)
    {
        result = CY_RSLT_AFE_TUNER_INALID_CMD;
        goto send_response;
    }
    AFE_USB_SETTINGS.channel_3 = atoi(params[5]);
    if(AFE_USB_SELECT_SIG_C < AFE_USB_SETTINGS.channel_3)
    {
        result = CY_RSLT_AFE_TUNER_INALID_CMD;
        goto send_response;
    }

    result = afe_speech_enhancement_update_dbg_out_config(handle->sp_enh_context, &AFE_USB_SETTINGS);
    if(CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "Failed to update debug config");
        result = CY_RSLT_AFE_TUNER_INTERNAL_ERROR;
        goto send_response;
    }
    else
    {
        afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
        MY_AFE_USB_SETTINGS = AFE_USB_SETTINGS;
    }
    return CY_RSLT_SUCCESS;

send_response:
    afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    return CY_RSLT_SUCCESS;
}


static cy_rslt_t config_get_audio_channels(void *context, char** params, int params_cnt)
{
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    cy_rslt_t result;
    char data[16] = {0};

    memset(data, 0, sizeof(data));

    sprintf(data, "%u,%u,%u,%u", MY_AFE_USB_SETTINGS.channel_0, MY_AFE_USB_SETTINGS.channel_1,
            MY_AFE_USB_SETTINGS.channel_2, MY_AFE_USB_SETTINGS.channel_3);
    result = CY_RSLT_SUCCESS;
    afe_send_tuner_command_res(handle, get_status_string_from_result(result), data);

    return result;
}



static cy_rslt_t config_start_aec_calibration(void *context, char** params, int params_cnt)
{
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    cy_rslt_t result;
    char data[12] = {0};
    cy_afe_config_setting_t config_setting = {0};
    int bulk_delay_ms = 0;

    memset(data, 0, sizeof(data));

    cy_afe_log_info("start aec calibration");
#ifdef DUMP_SAVE_BY_INDEX
    cy_dump_save_by_index(0, NULL, 0, 0x1);
    cy_dump_save_by_index(1, NULL, 0, 0x1);
    cy_dump_save_by_index(2, NULL, 0, 0x1);
#endif /* DUMP_SAVE_BY_INDEX */
    config_setting.action = CY_AFE_NOTIFY_CONFIG;
    config_setting.config_name = CY_AFE_CONFIG_BULK_DELAY_CALC_START;
    config_setting.value = NULL;
    handle->tuner_callbacks.notify_settings_callback(handle, &config_setting,
        handle->config_init.user_arg_callbacks);

    cy_afe_bd_calc_wait_for_complete(CY_RTOS_NEVER_TIMEOUT, &bulk_delay_ms);

    config_setting.action = CY_AFE_NOTIFY_CONFIG;
    config_setting.config_name = CY_AFE_CONFIG_BULK_DELAY_CALC_STOPPED;
    config_setting.value = &bulk_delay_ms;
    handle->tuner_callbacks.notify_settings_callback(handle, &config_setting,
            handle->config_init.user_arg_callbacks);

    cy_afe_log_info("aec calibration done:%d",bulk_delay_ms);

    sprintf(data, "%d",bulk_delay_ms);
    if (bulk_delay_ms == -1)
    {
        result = CY_RSLT_AFE_TUNER_INTERNAL_ERROR;
    } else
    {
        result = CY_RSLT_SUCCESS;
    }
    afe_send_tuner_command_res(handle, get_status_string_from_result(result), data);

    return result;
}

static cy_rslt_t config_get_sound_meter(void *context, char** params, int params_cnt)
{
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    cy_rslt_t result;
    int16_t audio_meter_data[CY_AFE_AUDIO_METER_MAX] = {0};
    char data[54] = {0};

    memset(data, 0, sizeof(data));
    memset(audio_meter_data, 0, sizeof(audio_meter_data));
    afe_speech_enhancement_get_sound_meter(context, audio_meter_data);

#if CY_AFE_INPUT_NUM_OF_CHANNELS == 2
    sprintf(data, "%"PRIu16",%"PRIu16",%"PRIu16, audio_meter_data[0], audio_meter_data[1],audio_meter_data[2]);
#else
    sprintf(data, "%"PRIu16",%"PRIu16, audio_meter_data[0], audio_meter_data[1]);
#endif

    result = CY_RSLT_SUCCESS;
    afe_send_tuner_command_res(handle, get_status_string_from_result(result), data);

    return result;
}

static void afe_str_to_get_component_params_component_id (
                        const char *str, ifx_sp_enh_ip_component_config_t *comp1)
{
     int j;
     *comp1 = IFX_SP_ENH_IP_COMPONENT_INVALID;

     for (j = 0;  j < sizeof (sp_enh_ip_get_param_component_id_conversion) / sizeof (sp_enh_ip_get_param_component_id_conversion[0]);  ++j)
     {
         if (!strcmp (str, sp_enh_ip_get_param_component_id_conversion[j].str))
         {
             *comp1 =  sp_enh_ip_get_param_component_id_conversion[j].val;
             break;
         }
     }

     return;
}

static cy_rslt_t config_get_component_params(void *context, char** params, int params_cnt)
{
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    ifx_sp_enh_ip_component_config_t component_name = IFX_SP_ENH_IP_COMPONENT_INVALID;
    cy_rslt_t result = CY_RSLT_AFE_BAD_ARG;
    char data[160];
    int32_t value[6];
    int32_t value_count;

    memset(data, 0, sizeof(data));
    memset(value, 0, sizeof(value));
    value_count = sizeof(value)/sizeof(value[0]);

    if(strcmp(GET_CMD, params[0]) == 0)
    {
        afe_str_to_get_component_params_component_id(params[1],&component_name);
        if( component_name == IFX_SP_ENH_IP_COMPONENT_INVALID )
        {
            result = CY_RSLT_AFE_TUNER_INVALID_CMD_PARAMS;
            goto send_response;
        }

        result =  afe_speech_enhancement_get_component_params(context, component_name, (void*)value, &value_count);
        if(CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to get component %s params", params[1]);
            goto send_response;
        }

        switch (component_name)
        {
            case IFX_SP_ENH_IP_COMPONENT_HPF:
            {
                snprintf( data, sizeof(data) - 1, "%"PRIi32, value[0]);
            }
                break;
            case IFX_SP_ENH_IP_COMPONENT_AEC:
            {
                snprintf( data, sizeof(data) - 1, "%"PRIi32",%"PRIi32, value[0], value[1] );
            }
                break;
            case IFX_SP_ENH_IP_COMPONENT_BF:
            {
                snprintf( data, sizeof(data) - 1, "%s,%"PRIi32",%"PRIi32",%"PRIi32",%"PRIi32
                                                , (value[0]==0)?"low":(value[0]==1)?"medium":"high"
                                                , value[1], value[2], value[3], value[4]);
            }
                break;
            case IFX_SP_ENH_IP_COMPONENT_ES:
            case IFX_SP_ENH_IP_COMPONENT_DSES:
            {
                snprintf( data, sizeof(data) - 1, "%s", (value[0]==0)?"low":(value[0]==1)?"medium":"high");
            }
                break;
            case IFX_SP_ENH_IP_COMPONENT_NS:
            {
                snprintf( data, sizeof(data) - 1, "%"PRIi32, value[0]);
            }
                break;
            case IFX_SP_ENH_IP_COMPONENT_DSNS:
            {
                snprintf( data, sizeof(data) - 1, "%"PRIi32, value[0]);
            }
                break;
            default: /* Other cases do nothing */
            {
                result = CY_RSLT_AFE_TUNER_INALID_CMD;
                cy_afe_log_err(result,"Invalid get config component:%d",component_name);
                goto send_response;
            }
            break;
        }

        afe_send_tuner_command_res(handle, get_status_string_from_result(result), data);
        return CY_RSLT_SUCCESS;
    }
    else
    {
        result = CY_RSLT_AFE_TUNER_INALID_CMD;
        cy_afe_log_err(result,"Only get cmd supported for comp params:%d",component_name);
        goto send_response;
    }

send_response:
    afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    return CY_RSLT_SUCCESS;
}

static cy_rslt_t config_get_input_params(void *context, char** params, int params_cnt)
{
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    cy_rslt_t result = CY_RSLT_AFE_BAD_ARG;
    cy_afe_config_setting_t config_setting;
    char data[160];
    int32_t input_gain, num_channels = AFE_INPUT_NUMBER_CHANNELS, input_source = AFE_INPUT_SOURCE;
    int32_t input_frame_ms = AFE_FRAME_SIZE_MS, input_sr = AFE_FRAME_RATE_SPS, aec_ref = AFE_USE_USB_AEC_REF;
    int32_t uart_br = AFE_UART_BAUDRATE, target_speaker = AFE_USE_TARGET_SPEAKER;

    memset(data, 0, sizeof(data));

    if(strcmp(GET_CMD, params[0]) == 0)
    {
        memset(&config_setting, 0, sizeof(cy_afe_config_setting_t));
        config_setting.action = CY_AFE_READ_CONFIG;
        config_setting.config_name = CY_AFE_CONFIG_INPUT_GAIN;
        config_setting.value = (int*) &input_gain;
        result = handle->tuner_callbacks.notify_settings_callback(handle, &config_setting,
                handle->config_init.user_arg_callbacks);
            if(CY_RSLT_SUCCESS != result)
        {
            cy_afe_log_err(result, "Failed to read input gain from application");
            result = CY_RSLT_AFE_TUNER_INTERNAL_ERROR;
            goto send_response;
        }

        snprintf( data, sizeof(data) - 1,
            "%"PRIi32",%"PRIi32",%"PRIi32",%"PRIi32",%"PRIi32",%"PRIi32",%"PRIi32",%"PRIi32,
            input_gain, num_channels, input_source, input_frame_ms, input_sr, aec_ref,target_speaker, uart_br);

        afe_send_tuner_command_res(handle, get_status_string_from_result(result), data);
        return CY_RSLT_SUCCESS;
    }
    else
    {
        result = CY_RSLT_AFE_TUNER_INALID_CMD;
        goto send_response;
    }

send_response:
    afe_send_tuner_command_res(handle, get_status_string_from_result(result), NULL);
    return CY_RSLT_SUCCESS;
}

#endif

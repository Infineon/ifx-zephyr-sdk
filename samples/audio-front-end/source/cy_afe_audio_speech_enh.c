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
 * @file cy_afe_audio_speech_enh.c
 * @brief Set of routines for speech enhancement. These functions are interface between
 *        AFE middleware code and system component APIs.
 *
 */

#include "cy_afe_audio_speech_enh.h"
#include "cy_audio_front_end.h"
#include "cy_afe_audio_internal.h"
#include "cy_sp_enh.h"

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
#include "cy_afe_configurator_settings.h"
#endif

/******************************************************
 *                     Macros
 ******************************************************/
// #define CY_RSLT_ERROR           (-1)

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
#ifdef CY_AFE_ENABLE_TUNING_FEATURE
int32_t ifx_out_locator[AFE_USB_SELECT_SIG_C+1];
#endif
int16_t AUDIO_METER[CY_AFE_AUDIO_METER_MAX];

/******************************************************
 *               Static Functions
 ******************************************************/

/******************************************************
 *               Functions
 ******************************************************/

cy_rslt_t afe_sp_alloc_memory_callback_t(ifx_sp_mem_id mem_id,
        uint32_t size, void **buffer)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if(NULL == buffer)
    {
        result = CY_RSLT_AFE_BAD_ARG;
        return result;
    }

    *buffer = NULL;

    if(NULL != afe_alloc_memory)
    {
        switch(mem_id)
        {
            case IFX_SP_MEM_ID_HANDLE:
            {
                result = afe_alloc_memory(CY_AFE_MEM_ID_AFE_CONTEXT,size,buffer);
                break;
            }
            case IFX_SP_MEM_ID_SCRATCH_MEM:
            {
                result = afe_alloc_memory(CY_AFE_MEM_ID_ALGORITHM_SCRATCH_MEMORY,size,buffer);
                break;
            }
            case IFX_SP_MEM_ID_PERSISTENT_MEM:
            {
                result = afe_alloc_memory(CY_AFE_MEM_ID_ALGORITHM_PERSISTENT_MEMORY,size,buffer);
                break;
            }
            case IFX_SP_MEM_ID_BF_PERSISTENT_MEM:
            {
                result = afe_alloc_memory(CY_AFE_MEM_ID_ALGORITHM_BF_MEMORY,size,buffer);
                break;
            }
            case IFX_SP_MEM_ID_DSNS_SOCMEM_PERSISTENT_MEM:
            {
                result = afe_alloc_memory(CY_AFE_MEM_ID_ALGORITHM_NS_MEMORY,size,buffer);
                break;
            }
            case IFX_SP_MEM_ID_DSES_SOCMEM_PERSISTENT_MEM:
            {
                result = afe_alloc_memory(CY_AFE_MEM_ID_ALGORITHM_ES_MEMORY,size,buffer);
                break;
            }
            case IFX_SP_MEM_ID_GDE_PERSISTENT_MEM:
            {
                result = afe_alloc_memory(CY_AFE_MEM_ID_GDE_PERSISTENT_MEM,size,buffer);
                break;
            }

            default:
            {
                result = afe_alloc_memory(CY_AFE_MEM_ID_GENERIC_MEMORY,size,buffer);
                break;
            }
        }
    }
    else
    {
        *buffer = (void *)calloc (size,1);
    }

    return result;
}

cy_rslt_t afe_sp_free_memory_callback_t(ifx_sp_mem_id mem_id,
        void *buffer)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_afe_mem_id_t afe_mem_id = CY_AFE_MEM_ID_INVALID;

    if(NULL == buffer)
    {
        result = CY_RSLT_AFE_BAD_ARG;
        return result;
    }

    switch(mem_id)
    {
        case IFX_SP_MEM_ID_HANDLE:
        {
            afe_mem_id = CY_AFE_MEM_ID_AFE_CONTEXT;
            break;
        }
        case IFX_SP_MEM_ID_SCRATCH_MEM:
        {
            afe_mem_id = CY_AFE_MEM_ID_ALGORITHM_SCRATCH_MEMORY;
            break;
        }
        case IFX_SP_MEM_ID_PERSISTENT_MEM:
        {
            afe_mem_id = CY_AFE_MEM_ID_ALGORITHM_PERSISTENT_MEMORY;
            break;
        }
        case IFX_SP_MEM_ID_BF_PERSISTENT_MEM:
        {
            afe_mem_id = CY_AFE_MEM_ID_ALGORITHM_BF_MEMORY;
            break;
        }
        case IFX_SP_MEM_ID_DSNS_SOCMEM_PERSISTENT_MEM:
        {
            afe_mem_id = CY_AFE_MEM_ID_ALGORITHM_NS_MEMORY;
            break;
        }
        case IFX_SP_MEM_ID_DSES_SOCMEM_PERSISTENT_MEM:
        {
            afe_mem_id = CY_AFE_MEM_ID_ALGORITHM_ES_MEMORY;
            break;
        }
        case IFX_SP_MEM_ID_GDE_PERSISTENT_MEM:
        {
            afe_mem_id = CY_AFE_MEM_ID_GDE_PERSISTENT_MEM;
            break;
        }
        default:
        {
            afe_mem_id = CY_AFE_MEM_ID_GENERIC_MEMORY;
            break;
        }
    }

    if(NULL != afe_free_memory)
    {
        result = afe_free_memory(afe_mem_id, buffer);
    }
    else
    {
        free(buffer);
        buffer = NULL;  // This has no effect - buffer is a parameter copy
    }

    return result;
}

/*******************************************************************************
 * Function Name: afe_speech_enhancement_init
 ********************************************************************************
 * Summary:
 *   Parse header file format generated by audio front end configurator tool and store
 *   information in structure to use it later. Allocate required memory for persistent
 *   memory & scratch memory.
 *
 * Parameters:
 *   filter_settings (in) : header file filter setting format generated by audio front end
 *                          configurator tool
 *   context (in)         : audio front end instance
 *******************************************************************************/
cy_rslt_t afe_speech_enhancement_init(int32_t *filter_settings,uint8_t *mw_settings,
                                    uint32_t mw_settings_length, void **context)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_sp_enh_config_params sp_enh_config;
    cy_sp_enh_handle* sp_enh_handle = NULL;

#ifdef ENABLE_AFE_MW_CHECK_POINT
    AFE_MW_CHECK_POINT()
#endif

    memset(&sp_enh_config, 0, sizeof(cy_sp_enh_config_params));
    sp_enh_config.sampling_rate = AFE_FRAME_RATE_SPS;
    sp_enh_config.input_frame_size = (AFE_FRAME_RATE_SPS * AFE_FRAME_SIZE_MS)/1000;
    sp_enh_config.num_mics = 2;

#ifdef ENABLE_IFX_HPF
    sp_enh_config.hpf_enable = 1;
#endif

#ifdef ENABLE_IFX_AEC
    sp_enh_config.aec_enable = 1;
#endif

#ifdef ENABLE_IFX_BF
    sp_enh_config.bf_enable = 1;
#endif

#ifdef ENABLE_IFX_DRVB
    sp_enh_config.drvb_enable = 1;
#endif

#ifdef ENABLE_IFX_NS
    sp_enh_config.ns_enable = 1;
#endif

#ifdef ENABLE_IFX_DSNS
    sp_enh_config.dsns_enable = 1;
#endif

#ifdef ENABLE_IFX_ES
    sp_enh_config.es_enable = 1;
#endif

#ifdef ENABLE_IFX_DSES
    sp_enh_config.dses_enable = 1;
#endif

#ifdef ENABLE_IFX_ANALYSIS
    sp_enh_config.anasyn_enable = 1;
#ifndef ENABLE_IFX_SYNTHESIS
#error "ENABLE_IFX_SYNTHESIS to be enabled in afe configurator"
#endif
#endif

    if(NULL != afe_alloc_memory && NULL != afe_free_memory)
    {
        cy_sp_alloc_memory = afe_sp_alloc_memory_callback_t;
        cy_sp_free_memory = afe_sp_free_memory_callback_t;
    }
    else
    {
        cy_sp_alloc_memory = NULL;
        cy_sp_free_memory = NULL;
    }

    cy_afe_log_info("sample_rate = %"PRIi32, sp_enh_config.sampling_rate);
    cy_afe_log_info("audio_frame_size = %"PRIi32, sp_enh_config.input_frame_size);
    cy_afe_log_info("num_channels = %"PRIi32, sp_enh_config.num_mics);
    cy_afe_log_info("hpf_enable = %"PRIi32, sp_enh_config.hpf_enable);
    cy_afe_log_info("aec_enable = %"PRIi32, sp_enh_config.aec_enable);
    cy_afe_log_info("bf_enable = %"PRIi32, sp_enh_config.bf_enable);
    cy_afe_log_info("drvb_enable = %"PRIi32, sp_enh_config.drvb_enable);
    cy_afe_log_info("ns_enable = %"PRIi32, sp_enh_config.ns_enable|sp_enh_config.dsns_enable);
    cy_afe_log_info("es_enable = %"PRIi32, sp_enh_config.es_enable|sp_enh_config.dses_enable);
    cy_afe_log_info("anasyn_enable = %"PRIi32, sp_enh_config.anasyn_enable);

    result = cy_sp_enh_init(filter_settings, mw_settings,
                            mw_settings_length, &sp_enh_handle);
    if(CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "Failed to initialize speech enhancement");
        result = CY_RSLT_AFE_SYSTEM_MODULE_ERROR;
        goto CLEAN_RETURN;
    }

    *context = sp_enh_handle;

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
    afe_speech_enhancement_update_dbg_out_config(sp_enh_handle,&MY_AFE_USB_SETTINGS);
#else
    cy_sp_enh_configure_dbg_out(sp_enh_handle, IFX_SP_ENH_IP_COMPONENT_INVALID, false);
#endif

    return CY_RSLT_SUCCESS;

CLEAN_RETURN:
#ifdef ENABLE_AFE_MW_CHECK_POINT
    AFE_MW_CHECK_POINT()
#endif
    cy_sp_enh_deinit(sp_enh_handle);
    return result;
}

/*******************************************************************************
 * Function Name: afe_speech_enhancement_process
 ********************************************************************************
 * Summary:
 *   Receive input1 (from mic1), input2 (from mic2) and reference data. Perform speech
 *   enhancement on input by invoking system level APIs and return the output.
 *
 * Parameters:
 *   context (in)             : speech enhancement context
 *   sp_enh_input_output (in) : speech enhancement input/output pointers
 *******************************************************************************/
cy_rslt_t afe_speech_enhancement_process(void *context, afe_sp_enh_input_output_t *sp_enh_input_output)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

#ifdef ENABLE_AFE_MW_CHECK_POINT
    AFE_MW_CHECK_POINT()
#endif

#ifdef ENABLE_AFE_STUB
    memcpy(sp_enh_input_output->output, sp_enh_input_output->input1, CY_AFE_MONO_FRAME_SIZE_IN_BYTES);
#else
    result = cy_sp_enh_process(context, sp_enh_input_output->input1,
            sp_enh_input_output->input2,
            sp_enh_input_output->aec_reference_input,
            sp_enh_input_output->output,
            sp_enh_input_output->ifx_internal_output, AUDIO_METER);

    if(CY_RSLT_SUCCESS != result)
    {
        cy_afe_log_err(result, "Failed to process speech enhancement");
        result = CY_RSLT_AFE_SYSTEM_MODULE_ERROR;
        return result;
    }
#endif

#ifdef ENABLE_AFE_MW_CHECK_POINT
    AFE_MW_CHECK_POINT()
#endif

    return CY_RSLT_SUCCESS;
}

/*******************************************************************************
 * Function Name: afe_speech_enhancement_deinit
 ********************************************************************************
 * Summary:
 *   Free the allocated persistent & scratch memory. Also, free the memory for
 *   speech enhancement handle.
 *
 * Parameters:
 *   context (in)             : speech enhancement context
 *******************************************************************************/
cy_rslt_t afe_speech_enhancement_deinit(void *context)
{
    cy_sp_enh_handle *handle = NULL;

    handle = (cy_sp_enh_handle*) context;

    cy_sp_enh_deinit(handle);

    cy_sp_alloc_memory = NULL;
    cy_sp_free_memory = NULL;

    return CY_RSLT_SUCCESS;
}

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
static cy_rslt_t is_afe_speech_enhancement_component_enabled(ifx_sp_enh_ip_component_config_t component_name)
{
#ifndef ENABLE_IFX_HPF
        if(component_name == IFX_SP_ENH_IP_COMPONENT_HPF)
        {
            return CY_RSLT_AFE_TUNER_COMPONENT_NOT_ENABLED;
        }
#endif

#ifndef ENABLE_IFX_AEC
        if(component_name == IFX_SP_ENH_IP_COMPONENT_AEC)
        {
            return CY_RSLT_AFE_TUNER_COMPONENT_NOT_ENABLED;
        }
#endif

#ifndef ENABLE_IFX_ANALYSIS
        if(component_name == IFX_SP_ENH_IP_COMPONENT_ANALYSIS)
        {
            return CY_RSLT_AFE_TUNER_COMPONENT_NOT_ENABLED;
        }
#endif

#ifndef ENABLE_IFX_BF
        if(component_name == IFX_SP_ENH_IP_COMPONENT_BF)
        {
            return CY_RSLT_AFE_TUNER_COMPONENT_NOT_ENABLED;
        }
#endif

#ifndef ENABLE_IFX_DRVB
        if(component_name == IFX_SP_ENH_IP_COMPONENT_DRVB)
        {
            return CY_RSLT_AFE_TUNER_COMPONENT_NOT_ENABLED;
        }
#endif

#ifndef ENABLE_IFX_ES
        if(component_name == IFX_SP_ENH_IP_COMPONENT_ES)
        {
            return CY_RSLT_AFE_TUNER_COMPONENT_NOT_ENABLED;
        }
#endif

#ifndef ENABLE_IFX_DSES
        if(component_name == IFX_SP_ENH_IP_COMPONENT_DSES)
        {
            return CY_RSLT_AFE_TUNER_COMPONENT_NOT_ENABLED;
        }
#endif

#ifndef ENABLE_IFX_NS
        if(component_name == IFX_SP_ENH_IP_COMPONENT_NS)
        {
            return CY_RSLT_AFE_TUNER_COMPONENT_NOT_ENABLED;
        }
#endif

#ifndef ENABLE_IFX_DSNS
        if(component_name == IFX_SP_ENH_IP_COMPONENT_DSNS)
        {
            return CY_RSLT_AFE_TUNER_COMPONENT_NOT_ENABLED;
        }
#endif

#ifndef ENABLE_IFX_SYNTHESIS
        if(component_name == IFX_SP_ENH_IP_COMPONENT_SYNTHESIS)
        {
            return CY_RSLT_AFE_TUNER_COMPONENT_NOT_ENABLED;
        }
#endif
        return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_speech_enhancement_enable_disable_component(void *context,
        ifx_sp_enh_ip_component_config_t component_name, bool enable)
{
    cy_rslt_t result;
    afe_internal_context_t *handle = (afe_internal_context_t*) context;

    result = is_afe_speech_enhancement_component_enabled(component_name);
    if(result != CY_RSLT_SUCCESS)
    {
        cy_afe_log_err(result, "Cmd received for statically disabled component ID:%d", component_name);
        return result;
    }

    cy_rtos_mutex_get(&handle->audio_tuner_mutex,CY_RTOS_NEVER_TIMEOUT);
    result = cy_sp_enh_enable_disable_component(handle->sp_enh_context, component_name, enable);
    cy_rtos_mutex_set(&handle->audio_tuner_mutex);
    if(result != CY_RSLT_SUCCESS)
    {
        cy_afe_log_err(result, "Failed to enable/disable component");
        if(result == CY_RSLT_INVALID_PARAMS)
        {
            result = CY_RSLT_AFE_TUNER_INVALID_CMD_PARAMS;
        }
        else
        {
            result = CY_RSLT_AFE_TUNER_GENERIC_ERROR;
        }
    }

    return result;
}

cy_rslt_t afe_speech_enhancement_update_config_value(void *context, ifx_sp_enh_ip_component_config_t component_name, int32_t value)
{
    cy_rslt_t result;
    int32_t comp_value[2] = {0};
    afe_internal_context_t *handle = (afe_internal_context_t*) context;

    result = is_afe_speech_enhancement_component_enabled(component_name);
    if(result != CY_RSLT_SUCCESS)
    {
        cy_afe_log_err(result, "Cmd received for statically disabled component ID:%d", component_name);
        return result;
    }

    comp_value[0] = value;
    comp_value[1] = value;

    cy_rtos_mutex_get(&handle->audio_tuner_mutex,CY_RTOS_NEVER_TIMEOUT);
    result =  cy_sp_enh_update_config_value(handle->sp_enh_context,component_name,comp_value);
    cy_rtos_mutex_set(&handle->audio_tuner_mutex);
    if(result != CY_RSLT_SUCCESS)
    {
        cy_afe_log_err(result, "Failed to update config value");
        if(result == CY_RSLT_INVALID_PARAMS)
        {
            result = CY_RSLT_AFE_TUNER_INVALID_CMD_PARAMS;
        }
        else
        {
            result = CY_RSLT_AFE_TUNER_GENERIC_ERROR;
        }
    }

    return result;
}

cy_rslt_t afe_speech_enhancement_get_config_value(void *context, ifx_sp_enh_ip_component_config_t component_name, int32_t* value)
{
    cy_rslt_t result = CY_RSLT_AFE_GENERIC_ERROR;
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    hpf_settings_struct_t hpf_pt;
    aec_settings_struct_t aec_pt;
    bf_settings_struct_t bf_pt;
    es_settings_struct_t es_pt;
    ns_settings_struct_t ns_pt;
    dsns_settings_struct_t dsns_pt;
    void * value_struct_pt = NULL;

    if(NULL == value)
    {
        cy_afe_log_err(result, "Invalid value arg");
        return CY_RSLT_AFE_BAD_ARG;
    }

    result = is_afe_speech_enhancement_component_enabled(component_name);
    if(result != CY_RSLT_SUCCESS)
    {
        cy_afe_log_err(result, "Cmd received for statically disabled component ID:%d", component_name);
        return result;
    }

    switch (component_name)
    {
        case IFX_SP_ENH_IP_COMPONENT_HPF:
        {
            value_struct_pt = &hpf_pt;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_AEC:
        {
            value_struct_pt = &aec_pt;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_BF:
        {
            value_struct_pt = &bf_pt;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_ES:
        case IFX_SP_ENH_IP_COMPONENT_DSES:
        {
            value_struct_pt = &es_pt;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_DSNS:
        {
            value_struct_pt = &dsns_pt;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_NS:
        {
            value_struct_pt = &ns_pt;
        }
        break;
        default: /* Other cases do nothing */
        {
            CY_SP_PRINTF("Invalid get config component:%d\r\n",component_name);
            return CY_RSLT_BAD_ARG;
        }
            break;
    }

    cy_rtos_mutex_get(&handle->audio_tuner_mutex,CY_RTOS_NEVER_TIMEOUT);
    result =  cy_sp_enh_get_config_value(handle->sp_enh_context,component_name,value_struct_pt);
    cy_rtos_mutex_set(&handle->audio_tuner_mutex);
    if(result != CY_RSLT_SUCCESS)
    {
        cy_afe_log_err(result, "Failed to update config value");
        if(result == CY_RSLT_INVALID_PARAMS)
        {
            result = CY_RSLT_AFE_TUNER_INVALID_CMD_PARAMS;
        }
        else
        {
            result = CY_RSLT_AFE_TUNER_GENERIC_ERROR;
        }
    }

    switch (component_name)
    {
        case IFX_SP_ENH_IP_COMPONENT_HPF:
        {
            *value = hpf_pt.cutoff_freq_hz;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_AEC:
        {
            *value = aec_pt.bulk_delay_msec;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_BF:
        {
            *value = bf_pt.aggressiveness;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_ES:
        case IFX_SP_ENH_IP_COMPONENT_DSES:
        {
            *value = es_pt.aggressiveness;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_NS:
        {
            *value = ns_pt.ns_gain_dB;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_DSNS:
        {
            *value = dsns_pt.ns_gain_dB;
        }
        break;
        default: /* Other cases do nothing */
        {
            CY_SP_PRINTF("Invalid get config component:%d\r\n",component_name);
            return CY_RSLT_BAD_ARG;
        }
        break;
    }

    return result;
}

cy_rslt_t afe_speech_enhancement_get_component_status(void *context, ifx_sp_enh_ip_component_config_t component_name, bool* enable)
{
    cy_rslt_t result = CY_RSLT_AFE_GENERIC_ERROR;
    afe_internal_context_t *handle = (afe_internal_context_t*) context;

    if(NULL == enable)
    {
        cy_afe_log_err(result, "Invalid enable arg");
        return CY_RSLT_AFE_BAD_ARG;
    }

    result = is_afe_speech_enhancement_component_enabled(component_name);
    if(result != CY_RSLT_SUCCESS)
    {
        cy_afe_log_err(result, "Cmd received for statically disabled component ID:%d", component_name);
        return result;
    }

    cy_rtos_mutex_get(&handle->audio_tuner_mutex,CY_RTOS_NEVER_TIMEOUT);
    result = cy_sp_enh_get_component_status(handle->sp_enh_context, component_name, enable);
    cy_rtos_mutex_set(&handle->audio_tuner_mutex);
    if(result != CY_RSLT_SUCCESS)
    {
        cy_afe_log_err(result, "Failed to enable/disable component");
        if(result == CY_RSLT_INVALID_PARAMS)
        {
            result = CY_RSLT_AFE_TUNER_INVALID_CMD_PARAMS;
        }
        else
        {
            result = CY_RSLT_AFE_TUNER_GENERIC_ERROR;
        }
    }

    return result;
}

cy_rslt_t afe_speech_enhancement_update_dbg_out_config(void *sp_enh_context, afe_usb_settings_t *pMY_AFE_USB_SETTINGS)
{
    cy_rslt_t result;
    unsigned int ifx_out_locator_index = 0;
    cy_sp_enh_handle* sp_enh_handle = (cy_sp_enh_handle* )sp_enh_context;

    cy_sp_enh_configure_dbg_out(sp_enh_handle, IFX_SP_ENH_IP_COMPONENT_INVALID, false);
    memset(ifx_out_locator,0xFF,sizeof(ifx_out_locator));

    //printf("\r\nMicAddr: %p, %d\r\n",
    //		&sp_enh_handle->sp_enh_info.common.num_mics,
    //		sp_enh_handle->sp_enh_info.common.num_mics);

    /*
     * Order of the below function is inline with the AFE system calls
     * sequence
     * */
    if( (pMY_AFE_USB_SETTINGS->channel_0 == AFE_USB_SELECT_SIG_A_0) ||
        (pMY_AFE_USB_SETTINGS->channel_1 == AFE_USB_SELECT_SIG_A_0) ||
        (pMY_AFE_USB_SETTINGS->channel_2 == AFE_USB_SELECT_SIG_A_0) ||
        (pMY_AFE_USB_SETTINGS->channel_3 == AFE_USB_SELECT_SIG_A_0)  )
    {
        //cy_afe_log_info("DBG Out: Enabling AEC Out 0");
        result = is_afe_speech_enhancement_component_enabled(
                IFX_SP_ENH_IP_COMPONENT_AEC);
        if (result != CY_RSLT_SUCCESS)
        {
            cy_afe_log_err(result, "AEC disabled component ID:%d",
                    IFX_SP_ENH_IP_COMPONENT_AEC);
        }
        else
        {
            cy_sp_enh_configure_dbg_out(sp_enh_handle,
                    IFX_SP_ENH_IP_COMPONENT_AEC, true);
            ifx_out_locator[AFE_USB_SELECT_SIG_A_0] = ifx_out_locator_index++;

            if(2 == sp_enh_handle->sp_enh_info.common.num_mics)
            {
                if( (pMY_AFE_USB_SETTINGS->channel_0 == AFE_USB_SELECT_SIG_A_1) ||
                    (pMY_AFE_USB_SETTINGS->channel_1 == AFE_USB_SELECT_SIG_A_1) ||
                    (pMY_AFE_USB_SETTINGS->channel_2 == AFE_USB_SELECT_SIG_A_1) ||
                    (pMY_AFE_USB_SETTINGS->channel_3 == AFE_USB_SELECT_SIG_A_1)  )
                {
                    cy_sp_enh_configure_dbg_out(sp_enh_handle,IFX_SP_ENH_IP_COMPONENT_AEC, true);
                    ifx_out_locator[AFE_USB_SELECT_SIG_A_1] = ifx_out_locator_index++;
                }
                else
                {
                    ifx_out_locator_index++;
                }
            }
        }
    }

    if(2 == sp_enh_handle->sp_enh_info.common.num_mics) // Stereo
    {
        if( (pMY_AFE_USB_SETTINGS->channel_0 == AFE_USB_SELECT_SIG_B) ||
            (pMY_AFE_USB_SETTINGS->channel_1 == AFE_USB_SELECT_SIG_B) ||
            (pMY_AFE_USB_SETTINGS->channel_2 == AFE_USB_SELECT_SIG_B) ||
            (pMY_AFE_USB_SETTINGS->channel_3 == AFE_USB_SELECT_SIG_B)  )
        {
            //cy_afe_log_info("DBG Out: Enabling BF Out");

            result = is_afe_speech_enhancement_component_enabled(
                    IFX_SP_ENH_IP_COMPONENT_BF);
            if(result != CY_RSLT_SUCCESS)
            {
                cy_afe_log_err(result, "BF disabled component ID:%d",
                        IFX_SP_ENH_IP_COMPONENT_BF);
            }
            else
            {
                cy_sp_enh_configure_dbg_out(sp_enh_handle,IFX_SP_ENH_IP_COMPONENT_BF, true);
                ifx_out_locator[AFE_USB_SELECT_SIG_B] = ifx_out_locator_index++;
            }
        }
    }

    if( (pMY_AFE_USB_SETTINGS->channel_0 == AFE_USB_SELECT_SIG_C) ||
        (pMY_AFE_USB_SETTINGS->channel_1 == AFE_USB_SELECT_SIG_C) ||
        (pMY_AFE_USB_SETTINGS->channel_2 == AFE_USB_SELECT_SIG_C) ||
        (pMY_AFE_USB_SETTINGS->channel_3 == AFE_USB_SELECT_SIG_C)  )
    {
        //cy_afe_log_info("DBG Out: Enabling DR Out");

        result = is_afe_speech_enhancement_component_enabled(
                IFX_SP_ENH_IP_COMPONENT_DRVB);
        if(result != CY_RSLT_SUCCESS)
        {
            cy_afe_log_err(result, "DR disabled component ID:%d",
                    IFX_SP_ENH_IP_COMPONENT_DRVB);
        }
        else
        {
            cy_sp_enh_configure_dbg_out(sp_enh_handle,IFX_SP_ENH_IP_COMPONENT_DRVB, true);
            ifx_out_locator[AFE_USB_SELECT_SIG_C] = ifx_out_locator_index++;
        }
    }

    return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_speech_enhancement_get_sound_meter(void *context, int16_t *audio_meter)
{
    afe_internal_context_t *handle = (afe_internal_context_t*) context;

    cy_rtos_mutex_get(&handle->audio_tuner_mutex,CY_RTOS_NEVER_TIMEOUT);
    memcpy(audio_meter, AUDIO_METER,  sizeof(AUDIO_METER));
    cy_rtos_mutex_set(&handle->audio_tuner_mutex);

    return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_speech_enhancement_get_component_params(void *context, ifx_sp_enh_ip_component_config_t component_name,
                                                        int32_t* value, int32_t* value_count)
{
    cy_rslt_t result = CY_RSLT_AFE_GENERIC_ERROR;
    afe_internal_context_t *handle = (afe_internal_context_t*) context;
    hpf_settings_struct_t hpf_pt;
    aec_settings_struct_t aec_pt;
    bf_settings_struct_t bf_pt;
    es_settings_struct_t es_pt;
    ns_settings_struct_t ns_pt;
    dsns_settings_struct_t dsns_pt;
    void * value_struct_pt = NULL;

    if((NULL == value) || (NULL == value_count))
    {
        result = CY_RSLT_AFE_BAD_ARG;
        cy_afe_log_err(result, "Invalid value arg:%p value_count:%p", value, value_count);
        return result;
    }

    result = is_afe_speech_enhancement_component_enabled(component_name);
    if(result != CY_RSLT_SUCCESS)
    {
        cy_afe_log_err(result, "Cmd received for statically disabled component ID:%d",component_name);
        return result;
    }

    switch (component_name)
    {
        case IFX_SP_ENH_IP_COMPONENT_HPF:
        {
            if(*value_count < 1)
            {
                result = CY_RSLT_AFE_BAD_ARG;
                cy_afe_log_err(result, "Invalid value count:%"PRIu32, *value_count);
                return result;
            }
            value_struct_pt = &hpf_pt;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_AEC:
        {
            if(*value_count < 2)
            {
                result = CY_RSLT_AFE_BAD_ARG;
                cy_afe_log_err(result, "Invalid value count:%"PRIu32, *value_count);
                return result;
            }
            value_struct_pt = &aec_pt;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_BF:
        {
            if(*value_count < 5)
            {
                result = CY_RSLT_AFE_BAD_ARG;
                cy_afe_log_err(result, "Invalid value count:%"PRIu32, *value_count);
                return result;
            }
            value_struct_pt = &bf_pt;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_ES:
        case IFX_SP_ENH_IP_COMPONENT_DSES:
        {
            if(*value_count < 1)
            {
                result = CY_RSLT_AFE_BAD_ARG;
                cy_afe_log_err(result, "Invalid value count:%"PRIu32, *value_count);
                return result;
            }
            value_struct_pt = &es_pt;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_DSNS:
        {
            if(*value_count < 1)
            {
                result = CY_RSLT_AFE_BAD_ARG;
                cy_afe_log_err(result, "Invalid value count:%"PRIu32, *value_count);
                return result;
            }
            value_struct_pt = &dsns_pt;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_NS:
        {
            if(*value_count < 1)
            {
                result = CY_RSLT_AFE_BAD_ARG;
                cy_afe_log_err(result, "Invalid value count:%"PRIu32, *value_count);
                return result;
            }
            value_struct_pt = &ns_pt;
        }
        break;
        default: /* Other cases do nothing */
        {
            CY_SP_PRINTF("Invalid get config component:%d\r\n",component_name);
            return CY_RSLT_BAD_ARG;
        }
            break;
    }

    cy_rtos_mutex_get(&handle->audio_tuner_mutex,CY_RTOS_NEVER_TIMEOUT);
    result =  cy_sp_enh_get_config_value(handle->sp_enh_context,component_name,value_struct_pt);
    cy_rtos_mutex_set(&handle->audio_tuner_mutex);
    if(result != CY_RSLT_SUCCESS)
    {
        cy_afe_log_err(result, "Failed to update config value");
        if(result == CY_RSLT_INVALID_PARAMS)
        {
            result = CY_RSLT_AFE_TUNER_INVALID_CMD_PARAMS;
        }
        else
        {
            result = CY_RSLT_AFE_TUNER_GENERIC_ERROR;
        }
    }

    switch (component_name)
    {
        case IFX_SP_ENH_IP_COMPONENT_HPF:
        {
            value[0] = hpf_pt.cutoff_freq_hz;
            *value_count = 1;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_AEC:
        {
            value[0] = aec_pt.bulk_delay_msec;
            value[1] = aec_pt.tail_len_msec;
            *value_count = 2;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_BF:
        {
            value[0] = bf_pt.aggressiveness;
            value[1] = bf_pt.mic_distance_mm;  /* mic distance in mm unit */
            value[2] = bf_pt.num_beams;        /* number of beams */
            value[3] = bf_pt.angle_range_start;/* start angle range in degree */
            value[4] = bf_pt.angle_range_stop; /* end angle range in degree */
            *value_count = 5;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_ES:
        case IFX_SP_ENH_IP_COMPONENT_DSES:
        {
            value[0] = es_pt.aggressiveness;
            *value_count = 1;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_NS:
        {
            value[0] = ns_pt.ns_gain_dB;
            *value_count = 1;
        }
        break;
        case IFX_SP_ENH_IP_COMPONENT_DSNS:
        {
            value[0] = dsns_pt.ns_gain_dB;
            *value_count = 1;
        }
        break;
        default: /* Other cases do nothing */
        {
            result = CY_RSLT_BAD_ARG;
            cy_afe_log_err(result,"Invalid get config component:%d",component_name);
            return result;
        }
        break;
    }

    return result;
}
#endif

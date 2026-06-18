/*
 * Copyright (c) 2026 Infineon Technologies AG,
 * or an affiliate of Infineon Technologies AG.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file cy_afe_speech_enh_stubs.c
 * @brief Speech enhancement stub implementations for basic AFE mode
 * 
 * This file provides stub implementations for speech enhancement functions
 * when CONFIG_AUDIOFE_SPEECH_ENH is disabled, allowing core AFE functionality
 * to work without the full AVC speech processing library.
 */

#include "cy_result.h"
#include "cy_afe_types_compat.h"
#include "cy_audio_front_end_error.h"  /* For CY_RSLT_AFE_BAD_ARG */
#include <zephyr/sys/printk.h>

/* Global variables required by AFE when speech enhancement is disabled */
void* cy_sp_alloc_memory = NULL;
void* cy_sp_free_memory = NULL;

/**
 * Speech enhancement initialization stub
 * @param filter_settings   Filter configuration (unused in basic mode)
 * @param mw_settings      Middleware settings (unused in basic mode) 
 * @param mw_settings_length Settings length (unused in basic mode)
 * @param context          Speech enhancement context (set to NULL)
 * @return CY_RSLT_SUCCESS always (basic mode)
 */
cy_rslt_t afe_speech_enhancement_init(int32_t* filter_settings, uint8_t* mw_settings, 
                                      uint32_t mw_settings_length, void** context)
{
    printk("AFE: afe_speech_enhancement_init ENTRY\n");
    printk("AFE: Parameters - filter_settings=%p, mw_settings=%p, length=%u, context=%p\n", 
           filter_settings, mw_settings, mw_settings_length, context);
    
    /* Detailed parameter validation with specific error reporting */
    if (!context) {
        printk("AFE: ERROR - context pointer is NULL\n");
        return CY_RSLT_AFE_BAD_ARG;
    }
    
    /* Initialize context to NULL for basic mode - don't access other parameters */
    *context = NULL;
    
    /* Less strict validation for basic mode to prevent usage faults */
    printk("AFE: afe_speech_enhancement_init STUB - basic mode SUCCESS\n");
    printk("AFE: Parameters - filter_settings=%p, mw_settings=%p, length=%u\n", 
           filter_settings, mw_settings, mw_settings_length);
    
    printk("AFE: Speech enhancement initialization SUCCESS (basic mode)\n");
    return CY_RSLT_SUCCESS;
}

/**
 * Speech enhancement deinitialization stub
 * @param context Speech enhancement context (ignored)
 * @return CY_RSLT_SUCCESS always
 */
cy_rslt_t afe_speech_enhancement_deinit(void* context)
{
    (void)context;
    printk("AFE: Speech enhancement cleanup - basic mode\n");
    return CY_RSLT_SUCCESS;
}

/**
 * Speech enhancement processing stub - passthrough mode
 * @param context Speech enhancement context (ignored)
 * @param input_output Audio data (passed through unchanged)
 * @return CY_RSLT_SUCCESS always
 */
cy_rslt_t afe_speech_enhancement_process(void* context, void* input_output)
{
    (void)context;
    (void)input_output;
    
    /* In basic mode, we simply pass audio through without processing */
    return CY_RSLT_SUCCESS;
}

/**
 * Speech enhancement component enable/disable stub
 * @param handle AFE handle (ignored)
 * @param component Component type (ignored)
 * @param enable Enable flag (ignored)
 * @return CY_RSLT_SUCCESS always
 */
cy_rslt_t afe_speech_enhancement_enable_disable_component(void* handle, int component, int enable)
{
    (void)handle;
    (void)component;
    (void)enable;
    return CY_RSLT_SUCCESS;
}

/**
 * Speech enhancement configuration update stub
 * @param handle AFE handle (ignored)
 * @param component Component type (ignored)
 * @param value Configuration value (ignored)
 * @return CY_RSLT_SUCCESS always
 */
cy_rslt_t afe_speech_enhancement_update_config_value(void* handle, int component, void* value)
{
    (void)handle;
    (void)component;
    (void)value;
    return CY_RSLT_SUCCESS;
}

/**
 * Speech enhancement configuration get stub
 * @param handle AFE handle (ignored)
 * @param component Component type (ignored)
 * @param value Configuration value pointer (ignored)
 * @return CY_RSLT_SUCCESS always
 */
cy_rslt_t afe_speech_enhancement_get_config_value(void* handle, int component, void* value)
{
    (void)handle;
    (void)component;
    (void)value;
    return CY_RSLT_SUCCESS;
}

/**
 * Speech enhancement debug output configuration stub
 * @param sp_enh_context Speech enhancement context (ignored)
 * @param usb_settings USB debug settings (ignored)
 * @return CY_RSLT_SUCCESS always
 */
cy_rslt_t afe_speech_enhancement_update_dbg_out_config(void* sp_enh_context, afe_usb_settings_t* usb_settings)
{
    (void)sp_enh_context;
    (void)usb_settings;
    return CY_RSLT_SUCCESS;
}

/**
 * Speech enhancement component status get stub
 * @param handle AFE handle (ignored)
 * @param component Component type (ignored)
 * @param enable Status pointer (set to disabled)
 * @return CY_RSLT_SUCCESS always
 */
cy_rslt_t afe_speech_enhancement_get_component_status(void* handle, int component, int* enable)
{
    (void)handle;
    (void)component;
    
    if (enable) {
        *enable = 0; /* All components disabled in basic mode */
    }
    return CY_RSLT_SUCCESS;
}

/**
 * Speech enhancement get component parameters stub
 * @param context Speech enhancement context (ignored)
 * @param component_name Component name (ignored)
 * @param value Parameter values (set to default)
 * @param value_count Parameter count (set to 0)
 * @return CY_RSLT_SUCCESS always
 */
cy_rslt_t afe_speech_enhancement_get_component_params(void* context, int component_name,
                                                     void* value, uint32_t* value_count)
{
    (void)context;
    (void)component_name;
    (void)value;
    
    if (value_count) {
        *value_count = 0; /* No parameters in basic mode */
    }
    return CY_RSLT_SUCCESS;
}

/**
 * Speech enhancement get sound meter stub
 * @param context Speech enhancement context (ignored)
 * @param sound_meter Sound meter data (ignored)
 * @return CY_RSLT_SUCCESS always
 */
cy_rslt_t afe_speech_enhancement_get_sound_meter(void* context, void* sound_meter)
{
    (void)context;
    (void)sound_meter;
    
    /* Return success but no actual meter data in basic mode */
    return CY_RSLT_SUCCESS;
}

/* Define the ifx_out_locator array required by cy_afe_audio_process.c */
/* Initialize all elements to -1 to indicate no valid output locations in basic mode */
int32_t ifx_out_locator[16] = {-1, -1, -1, -1, -1, -1, -1, -1, 
                               -1, -1, -1, -1, -1, -1, -1, -1};
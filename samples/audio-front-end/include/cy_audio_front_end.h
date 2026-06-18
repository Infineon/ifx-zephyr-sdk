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
 * @file cy_audio_front_end.h
 * @brief Audio front end APIs and data structures
 *
 * Audio front end will be referred as AFE in the header file.
 *
 */

#ifndef AUDIO_FRONT_END_H__
#define AUDIO_FRONT_END_H__

#include "cy_result.h"
#include <stdbool.h>
#include <string.h>
#include "cy_audio_front_end_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup group_afe_macros Macros
 * \defgroup group_afe_enums Enumerated types
 * \defgroup group_afe_typedefs Typedefs
 * \defgroup group_afe_structures Structures
 * \defgroup group_afe_functions Functions
 */

/**
 * \addtogroup group_afe_macros
 * \{
 */

/******************************************************
 *                     Macros
 ******************************************************/

/**
 * Audio Sample data type
 */
#ifndef CY_AFE_DATA_T
/**
 * Audio Sample data bit-width is 16bit.
 */
#define CY_AFE_DATA_T int16_t

/**
 * Minimum mic input hardware gain value.
  * Based on hardware, application can override this value in the Makefile.
 */
#ifndef AFE_MIN_MIC_INPUT_HW_GAIN
    #define AFE_MIN_MIC_INPUT_HW_GAIN -105
#endif

/**
 * Maximum mic input hardware gain value.
  * Based on hardware, application can override this value in the Makefile.
 */
#ifndef AFE_MAX_MIC_INPUT_HW_GAIN
    #define AFE_MAX_MIC_INPUT_HW_GAIN 105
#endif

#endif

/** \} group_afe_macros */
/******************************************************
 *                    Constants
 ******************************************************/

 /**
 * \addtogroup group_afe_typedefs
 * \{
 */

 /**
* Audio front end handle
*/
typedef void* cy_afe_t;

/** \} group_afe_typedefs */

/******************************************************
 *                   Enumerations
 ******************************************************/
 /**
 * \addtogroup group_afe_enums
 * \{
 */
/**
 * AFE configuration name
 */
typedef enum
{
    CY_AFE_CONFIG_AEC_STATE,              /* State of AEC component */
    CY_AFE_CONFIG_AEC_BULK_DELAY,         /* Bulk delay configuration for AEC */
    CY_AFE_CONFIG_BEAM_FORMING_STATE,     /* State of beam forming component */
    CY_AFE_CONFIG_INFERENCE_CANCELLER,    /* Inference canceller configuration for beam forming */
    CY_AFE_CONFIG_DEREVEREBERATION_STATE, /* State of derevereberation component */
    CY_AFE_CONFIG_ESNS_STATE,             /* State of echo suppression/noise suppression component */
    CY_AFE_CONFIG_ECHO_SUPPRESSOR,        /* Echo suppressor configuration (dB) */
    CY_AFE_CONFIG_NOISE_SUPPRESSOR,       /* Noise suppressor configuration */
    CY_AFE_CONFIG_INPUT_GAIN,             /* Input gain configuration */
    CY_AFE_CONFIG_HPF,                    /* High pass filter configuration */
    CY_AFE_CONFIG_BULK_DELAY_CALC_START,   /* Bulk delay calculation started */
    CY_AFE_CONFIG_BULK_DELAY_CALC_STOPPED, /* Bulk delay calculation stopped */
    /* CY_AFE_CONFIG_COMPONENT_STATE,      State change of a component */
    CY_AFE_CONFIG_STREAM,                 /* Start Stop stream */
} cy_afe_config_name_t;

/**
 * Action that need to be performed on the AFE configuration
 */
typedef enum
{
    CY_AFE_READ_CONFIG,    /* AFE middleware asks application to read the configuration value */
    CY_AFE_UPDATE_CONFIG,  /* AFE middleware asks application to update/apply the configuration value */
    CY_AFE_NOTIFY_CONFIG   /* AFE middleware already applied the configuration. This is only to notify application */
} cy_afe_config_action_t;

/**
 * AFE configuration debug output
 */
typedef enum
{
    CY_AFE_CONFIG_DEBUG_NONE,             /* Output none */
    CY_AFE_CONFIG_DEBUG_AUDIO_IN0,        /* Audio input1 output */
    CY_AFE_CONFIG_DEBUG_AUDIO_IN1,        /* Audio input2 output */
    CY_AFE_CONFIG_DEBUG_PROCESSED_OUTPUT, /* AFE processed output */
    CY_AFE_CONFIG_DEBUG_REFERENCE_AEC,    /* AEC reference output */
    CY_AFE_CONFIG_DEBUG_AEC_REF_IN0,      /* AEC channel1 output */
    CY_AFE_CONFIG_DEBUG_AEC_REF_IN1,      /* AEC channel2 output */
    CY_AFE_CONFIG_DEBUG_BEAM_FORMING,     /* Beam forming intermediate output */
    CY_AFE_CONFIG_DEBUG_DEREVEREBERATION  /* Derevereberation intermediate output */
} cy_afe_config_debug_t;

/**
 * Debug Output index configuration
 */
typedef enum
{
    CY_AFE_CONFIG_DEBUG_BUF1,     /* Debug output index0 */
    CY_AFE_CONFIG_DEBUG_BUF2,     /* Debug output index1 */
    CY_AFE_CONFIG_DEBUG_BUF3,     /* Debug output index2 */
    CY_AFE_CONFIG_DEBUG_BUF4      /* Debug output index3 */
} cy_afe_config_debug_index_t;

/**
 * AFE system components
 */
typedef enum {
    CY_AFE_COMPONENT_AEC
} cy_afe_component_t;

/**
 * Memory ID configuration
 */
typedef enum
{
    CY_AFE_MEM_ID_INVALID, /* Memory ID Invalid */
    CY_AFE_MEM_ID_AFE_CONTEXT, /* AFE context memory */
    CY_AFE_MEM_ID_AFE_OUTPUT_BUFFER, /* AFE output buffer */
    CY_AFE_MEM_ID_AFE_DBG_OUT_BUFFER, /* AFE debug output buffer */
    CY_AFE_MEM_ID_AFE_TUNER_CMD_BUFFER, /* AFE internal tuner command buffer */
    CY_AFE_MEM_ID_ALGORITHM_PERSISTENT_MEMORY,  /* AFE algorithm persistent memory */
    CY_AFE_MEM_ID_ALGORITHM_SCRATCH_MEMORY, /* AFE algorithm scratch memory */
    CY_AFE_MEM_ID_ALGORITHM_BF_MEMORY, /* AFE algorithm scratch memory */
    CY_AFE_MEM_ID_ALGORITHM_NS_MEMORY, /* AFE algorithm DSNS memory - Required 16byte aligned buffer address */
    CY_AFE_MEM_ID_ALGORITHM_ES_MEMORY, /* AFE algorithm DSES memory - Required 16byte aligned buffer address */
    CY_AFE_MEM_ID_GDE_PERSISTENT_MEM,  /* GDE persistent memory */
    CY_AFE_MEM_ID_GENERIC_MEMORY, /* Generic memory */
    CY_AFE_MEM_ID_MAX
} cy_afe_mem_id_t;

/** \} group_afe_enums */

 /**
 * \addtogroup group_afe_structures
 * \{
 */
/******************************************************
 *                    Structures
 ******************************************************/

/**
 * Output buffer structure for filtered audio data
 */
 typedef struct cy_afe_buffer_info_s
{
    /** Input buffer pointer */
    CY_AFE_DATA_T *input_buf;
    /** AEC buffer pointer which was passed during \ref cy_afe_feed API */
    CY_AFE_DATA_T *input_aec_ref_buf;
    /** Output buffer pointer */
    CY_AFE_DATA_T *output_buf;
#ifdef CY_AFE_ENABLE_TUNING_FEATURE
    /** Debug output1 based on configuration */
    CY_AFE_DATA_T *dbg_output1;
    /** Debug output2 based on configuration */
    CY_AFE_DATA_T *dbg_output2;
    /** Debug output3 based on configuration */
    CY_AFE_DATA_T *dbg_output3;
    /** Debug output4 based on configuration */
    CY_AFE_DATA_T *dbg_output4;
#endif
 }cy_afe_buffer_info_t;

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
/**
 * AFE tuner buffer for request - response
 */
typedef struct cy_afe_tuner_buffer_s
{
    /**
     * Pointer to memory allocated by middleware.
     */
    uint8_t *buffer;

    /**
     * Size of the buffer allocated
     */
    uint16_t buffer_max_len;

    /**
     * Application should fill the length of the data received
     */
    uint16_t length;

}cy_afe_tuner_buffer_t;

/**
 * Audio front end configuration settings are provided based on settings received over UART dynamically through audio configurator
 * tool.
 */
typedef struct  cy_afe_config_setting_s
{
    /**
     * Action that need to be performed on configuration
     */
    cy_afe_config_action_t action;

    /**
     * Configuration name
     */
    cy_afe_config_name_t config_name;

    /**
     * Configuration value
     */
    void *value;
}cy_afe_config_setting_t;

/**
 * Audio front end Bulk delay module output params struct.
 */
typedef struct {
    /**
     * AEC reference buffer to be played in speaker
     */
   void* aec_ref_buffer;

   /**
    * AEC reference buffer in len in bytes
    */
   unsigned int aec_ref_buffer_len;
} bdm_init_out_params_t;

#endif
/** \} group_afe_structures */

 /**
 * \addtogroup group_afe_typedefs
 * \{
 */
/******************************************************
 *                 Type Definitions
 ******************************************************/

/**
 * Callback to get filtered audio data from audio front end middleware
 *
 * The callback will be invoked in the audio front end middleware thread context, next AFE input data will be only processed
 * once application returns from callback. Application can make use of \ref cy_afe_buffer_info_t buffer pointers to free the allocated memory
 * for output buffer, input buffer and AEC buffer (If AEC submodule is enabled) pointers for which memory allocated by
 * application will be provided. If output buffer memory is allocated by AFE middleware then application doesnt need to
 * free the memory..
 *
 * @param[in] handle                Pointer to AFE middleware instance which was created during \ref cy_afe_create API
 * @param[in] output_buffer         Pointer to \ref cy_afe_buffer_info_t with output buffer, input buffer, AEC buffer pointers
 * @param[in] user_arg              User argument
 *
 * @return    cy_rslt_t
 */
typedef cy_rslt_t (*cy_afe_output_callback_t)(cy_afe_t handle, cy_afe_buffer_info_t *output_buffer, void *user_arg);

/**
 * Callback to get buffer from the application to fill the filtered data (Output data)
 *
 * The callback will be invoked in the audio front end middleware context. Application should allocate memory and provide the
 * pointer to audio front end middleware. Application should not free/reuse the memory of output buffer. Audio front end
 * middleware will invoke \ref cy_afe_output_callback_t with the output data. Application may have choice to free/reuse the output
 * buffer in the callback.
 *
 * If application doesn't register output callback during \ref cy_afe_create API call then default implementation of callback will
 * be provided by audio front end middleware where static buffer will be used to fill the output data and provided to application
 * through \ref cy_afe_output_callback_t. Application should make use of the output buffer in the same callback. As static buffer
 * memory will be reused in subsequent calls of \ref cy_afe_output_callback_t.
 *
 * @param[in] handle                Pointer to AFE middleware instance which was created during \ref cy_afe_create API
 * @param[out] output_buffer        Pointer to output buffer to fill the filtered output data
 * @param[in] user_arg              User argument
 *
 * @return    cy_rslt_t
 */
typedef cy_rslt_t (*cy_afe_get_output_buffer_callback_t)(cy_afe_t handle, uint32_t **output_buffer, void *user_arg);

/**
 * Memory Callback to get memory from the application. Application may choose the location of the memory,
 * the memory can be from any section, sections like, DTCM, SOCMEM, SRAM1, SRAM0, ITCM, RRAM, etc.,
 *
 * Depends the memory choice the performance of algorithm will vary. The application needs to be
 * chose wisely the best available memory for the purpose.
 *
 * @param[in] mem_id               Memory Id, Application may choose the memory location depends on
 *                                 the memory id.
 * @param[in] size        		   Size of the memory
 * @param[out] buffer              Pointer to the memory.
 *
 * @return    cy_rslt_t
 */
typedef cy_rslt_t (*cy_afe_alloc_memory_callback_t)(cy_afe_mem_id_t mem_id,
        uint32_t size, void **buffer);

/**
 * Memory Callback to free memory back to the application. The memory which is allocated
 * using cy_afe_alloc_memory_callback_t will be freed back to the application using this
 * callback.
 *
 * @param[in] mem_id               Memory Id.
 * @param[in] buffer               Pointer to the memory.
 *
 * @return    cy_rslt_t
 */
typedef cy_rslt_t (*cy_afe_free_memory_callback_t)(cy_afe_mem_id_t mem_id,
        void *buffer);

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
/**
 * Callback function to write response data to configurator tool over transport (Ex. UART)
 *
 * AFE middleware frames response which need to be sent to configurator tool over transport (UART) and invokes the callback.
 * Application should make use of respective driver APIs to send the data. memory allocated for \ref cy_afe_tuner_buffer_t
 * will be freed by middleware once context is returned by application.
 *
 * @param[in] handle             Pointer to AFE middleware instance which was created during \ref cy_afe_create API
 * @param[in] response_buffer    Pointer to response buffer to send the data out to configurator tool
 * @param[in] user_arg           User argument
 *
 * @return    cy_rslt_t
 */
typedef cy_rslt_t (*cy_afe_write_response_callback_t)(cy_afe_t handle, cy_afe_tuner_buffer_t *response_buffer, void *user_arg);

/**
 * Callback function to receive request from configurator tool over transport (Ex. UART)
 *
 * Application can choose to block (to wait for data from transport) or return the context back to AFE middleware. Once the
 * context is returned back to AFE middleware, callback will be invoked after poll_timeout configured during AFE initialization.
 *
 * AFE middleware will allocate memory for request_buffer, hence application need to copy the data to \ref cy_afe_tuner_buffer_t.
 * freeing the memory for \ref cy_afe_tuner_buffer_t will be taken care by middleware.
 *
 * @param[in]  handle               Pointer to AFE middleware instance which was passed during \ref cy_afe_create API
 * @param[out] request_buffer       Pointer to request_buffer. memory for the request buffer (data) will be allocated by the middleware.
 * @param[in]  user_arg             User argument
 *
 * @return    cy_rslt_t
 */
typedef cy_rslt_t (*cy_afe_read_request_callback_t)(cy_afe_t handle, cy_afe_tuner_buffer_t *request_buffer, void *user_arg);

/**
 * Callback function to notify about change in configuration.
 *
 * Ex.
 * Case 1: User changes input gain in configuration tool. Since middleware cannot apply input_gain configuration. It asks application
 *         to apply/update the configuration.\ref cy_afe_config_setting_t values are:
 *
 * 	   	=> \ref cy_afe_config_action_t will be set to CY_AFE_UPDATE_CONFIG
 * 	   	=> \ref cy_afe_config_name_t will be set to CY_AFE_CONFIG_INPUT_GAIN
 * 	   	=> config_setting->value will be filled with respective value. application can typecast to (int) and use it.
 *
 * Case 2: User disables beam forming component from configurator tool. Middleware will apply the configuration and notifies application
 *         about the change. \ref cy_afe_config_setting_t values are:
 *
 * 	    => \ref cy_afe_config_action_t will be set to CY_AFE_NOTIFY_CONFIG
 * 	    => \ref cy_afe_config_name_t will be set to CY_AFE_CONFIG_BEAM_FORMING_STATE
 * 	    => config_setting->value will be filled with 0 if component is disabled & 1 if component is enabled. application can typecast
 * 	       to (int) and know about component status
 *
 * Case 3: Configurator tool reads input gain. Since middleware doesn't know about input gain configuration. It asks application to read
 *         configuration value. \ref cy_afe_config_setting_t values are:
 *
 * 	    => \ref cy_afe_config_action_t will be set to CY_AFE_READ_CONFIG
 * 	    => \ref cy_afe_config_name_t will be set to CY_AFE_CONFIG_INPUT_GAIN
 * 	    => config_setting->value can be updated to fill the value of input gain read by application.
 *
 * @param[in] handle             Pointer to AFE middleware instance which was created during \ref cy_afe_create API
 * @param[in] config_setting     Configuration setting structure which holds information of action that need to be performed on configuration,
 *                               configuration name & value.
 * @param[in] user_arg           User argument
 *
 * @return    cy_rslt_t
 */
typedef cy_rslt_t (*cy_afe_notify_tuner_settings_callback_t)(cy_afe_t handle, cy_afe_config_setting_t *config_setting, void *user_arg);
#endif

/** \} group_afe_typedefs */

 /**
 * \addtogroup group_afe_structures
 * \{
 */
/******************************************************
 *                    Structures
 ******************************************************/

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
/**
 * Audio front end tuner related callbacks
 */
typedef struct
{
    /**
     * Callback to notify application settings
     */
    cy_afe_notify_tuner_settings_callback_t notify_settings_callback;

    /**
     * Write callback to send the data to configurator tool
     */
    cy_afe_write_response_callback_t write_response_callback;

    /**
     * Read callback to read the data from the configurator tool
     */
    cy_afe_read_request_callback_t read_request_callback;
} cy_afe_tuner_callbacks_t;

#endif


/**
 * Audio front end middleware configuration
 */
typedef struct
{
    /** Pointer to filter settings. Application can get the filter settings
     *  from configurator tool generated header file or any other storage
     *  where application stored. */
    uint32_t *filter_settings;

    /** Pointer to middleware specific settings. Application can get the coefficients
     *  from configurator tool generated header file or any other storage
     *  where application stored. */
    uint8_t *mw_settings;

    /**
     * Length of the middleware specific settings
     */
    uint32_t mw_settings_length;

    /** User must pass the output callback to get the output data from AFE middleware */
    cy_afe_output_callback_t afe_output_callback;

    /**
     * Register callbacks to get the buffer to fill the output data
     *
     * User may choose to register the callback to provide the memory for the output buffer. If callback is registered,
     * then AFE middleware will invoke the callback to get memory for output buffer. And ownership of freeing the buffer
     * is with the application. If callback is not registered, AFE middleware will have the default implementation of callback
     * through which static buffer memory will be used, output data will be filled and provided to application through
     * \ref cy_afe_output_callback_t callback. Note that, static buffer memory will be used in subsequent output callbacks and
     * user doesn't need to free the memory.
     */
    cy_afe_get_output_buffer_callback_t afe_get_buffer_callback;

    /** Optional user argument which can be passed and provided back in the callbacks by middleware */
    void *user_arg_callbacks;

#ifdef CY_AFE_ENABLE_TUNING_FEATURE

    /**
     * Register tuner callbacks
     */
    cy_afe_tuner_callbacks_t tuner_cb;

    /** Optional user argument */
    void *user_args;

    /**
     * Configure poll_interval in ms. poll_interval will be used by AFE middleware to invoke read_callback periodically.
     */
    int poll_interval_ms;
#endif

    /** Memory Callback to get memory from the application. */
    cy_afe_alloc_memory_callback_t alloc_memory;

    /** Memory Callback to free memory back to the application. */
    cy_afe_free_memory_callback_t  free_memory;

} cy_afe_config_t;


/** \} group_afe_structures */

/******************************************************
 *                 Global Variables
 ******************************************************/

 /**
 * \addtogroup group_afe_functions
 * \{
 */
/******************************************************
 *               Function Declarations
 ******************************************************/

/**
 * Creates an instance of AFE middleware
 *
 * @param[in]   config_init       Configuration for audio front end middleware initialization
 * @param[out]  handle            Audio front end middleware handle which is allocated by middleware
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_afe_create(cy_afe_config_t *config_init, cy_afe_t *handle);

/**
 * Feed the audio data to audio front end middleware (10ms worth of data)
 *
 * Memory for the input buffer & aec reference data (if AEC is enabled) need to be allocated by application. Application shall not free/reuse
 * input buffer & AEC reference buffer till AFE middleware provides \ref cy_afe_output_callback_t callback. Application may have choice to
 * handle (free/reuse) the input & aec reference buffers on \ref cy_afe_output_callback_t callback.
 *
 * If Application needs to feed stereo data, then the data format must be non-interleaved format. example: For stereo data of 10ms
 * worth, first 320bytes must be of channel-1 and then next 320bytes must be channel-2. application must pass enough number of
 * bytes, if it doesn't pass then AFE middleware can go into unexpected state.
 *
 * AEC reference data shall be mono only.
 *
 * @param[in]  handle           Handle to audio front end instance created by the \ref cy_afe_create API
 * @param[in]  input_buffer     Pointer to input buffer to pass the audio data
 * @param[in]  aec_ref          AEC reference mono audio data
 *
 * @return     CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_afe_feed(cy_afe_t handle, CY_AFE_DATA_T *input_buffer, CY_AFE_DATA_T *aec_ref);

/**
 * Destroy created AFE middleware instance
 *
 * @param[out]  handle     Address of the Handle to audio front end instance created by the \ref cy_afe_create API. Once API returns,
 *                         \ref cy_afe_t handle will be reset to NULL.
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_afe_delete(cy_afe_t *handle);


#ifdef CY_AFE_ENABLE_TUNING_FEATURE
/**
 * Creates the bulk delay calculation submodule inside the AFE. This API needs to be
 * called by application on the notification event CY_AFE_CONFIG_BULK_DELAY_CALC_START
 *
 * @param[in]  handle           Handle to audio front end instance created by the \ref cy_afe_create API
 * @param[in]  bdm_out          Pointer to bulk delay module output params
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_afe_bd_calc_init(cy_afe_t handle, bdm_init_out_params_t *bdm_out);

/**
 * Deletes the bulk delay calculation submodule inside the AFE. This API needs to be
 * called by application on the notification event CY_AFE_CONFIG_BULK_DELAY_CALC_STOPPED
 *
 * @param[in]  handle           Handle to audio front end instance created by the \ref cy_afe_create API
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_afe_bd_calc_deinit(cy_afe_t handle);

#endif

/** \} group_afe_functions */

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_FRONT_END_H__ */

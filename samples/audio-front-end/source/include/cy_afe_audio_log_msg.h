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
* @file cy_afe_audio_log_msg.h
* @brief Audio front end log messages
*
*/

#ifndef AUDIO_FRONT_END_LOG_MESSAGE_H__
#define AUDIO_FRONT_END_LOG_MESSAGE_H__

#include "cy_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/
#if ENABLE_AUDIO_FRONT_END_LOGS == 3
/* Zephyr compatibility - use printk for AFE logging */
#define cy_afe_log_info(format,...)                  printk ("[AFE] "format" \n", ##__VA_ARGS__);
#define cy_afe_log_err(ret_val,format,...)           printk ("[AFE] [Err:0x%08x, Line:%d] "format"\n",ret_val,__LINE__, ##__VA_ARGS__);
#define cy_afe_log_err_on_no_isr(ret_val,format,...) printk ("[AFE] [Err:0x%08x, Line:%d] "format"\n",ret_val,__LINE__, ##__VA_ARGS__);
#define cy_afe_log_dbg(format,...)       			 printk ("[AFE] "format" \n", ##__VA_ARGS__);
#elif ENABLE_AUDIO_FRONT_END_LOGS == 2
#define cy_afe_log_info(format,...)                  printf ("[AFE] "format" \r\n", ##__VA_ARGS__);
#define cy_afe_log_err(ret_val,format,...)           printf ("[AFE] [Err:0x%"PRIx32",  Line:%d] "format"\r\n",ret_val,__LINE__, ##__VA_ARGS__);
#define cy_afe_log_err_on_no_isr(ret_val,format,...) if(false == is_in_isr()) printf ("[AFE] [Err:0x%"PRIx32", Line:%d] "format"\r\n",ret_val,__LINE__, ##__VA_ARGS__);
#define cy_afe_log_dbg(format,...)       			 printf ("[AFE] "format" \r\n", ##__VA_ARGS__);
#elif ENABLE_AUDIO_FRONT_END_LOGS
#define cy_afe_log_info(format,...)                  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[AFE] "format"\r\n", ##__VA_ARGS__);
#define cy_afe_log_err(ret_val,format,...)           cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_ERR,"[AFE] [Err:0x%"PRIx32", Line:%d] "format"\r\n",ret_val,__LINE__, ##__VA_ARGS__);
#define cy_afe_log_err_on_no_isr(ret_val,format,...) if(false == is_in_isr()) cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_ERR,"[AFE] [Err:0x%"PRIx32", line:%d] "format"\r\n",ret_val,__LINE__,##__VA_ARGS__);
#define cy_afe_log_dbg(format,...)       		     cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_DEBUG, "[AFE] "format"\r\n", ##__VA_ARGS__);
#else
#define cy_afe_log_info(format,...)
#define cy_afe_log_err(ret_val,format,...)
#define cy_afe_log_err_on_no_isr(ret_val,format,...)
#define cy_afe_log_dbg(format,...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_FRONT_END_LOG_MESSAGE_H__ */

/*
 * Copyright (c) 2026 Infineon Technologies AG,
 * or an affiliate of Infineon Technologies AG.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cy_afe_audio_internal.h"
#include "cy_audio_front_end_error.h"
#include <stdarg.h>
#include <zephyr/sys/printk.h>

afe_mw_check_points afe_mw_check_point = {0};

#ifdef cy_afe_log_err
#undef cy_afe_log_err
#endif
#ifdef cy_afe_log_info
#undef cy_afe_log_info
#endif
#ifdef cy_afe_log_dbg
#undef cy_afe_log_dbg
#endif

void cy_afe_log_err(cy_rslt_t result, const char* format, ...)
{
	(void)result;
	(void)format;
	printk("AFE ERROR [0x%08x]: log message\n", result);
}

void cy_afe_log_info(const char* format, ...)
{
	(void)format;
	printk("AFE INFO: log message\n");
}

void cy_afe_log_dbg(const char* format, ...)
{
	(void)format;
	printk("AFE DEBUG: log message\n");
}

cy_rslt_t afe_update_stats(void *context, afe_stats_type_t type)
{
	(void)context;
	(void)type;
	return CY_RSLT_SUCCESS;
}

#ifndef CONFIG_AVC
bool cy_afe_lib_is_license_expired(void)
{
	return false;
}
#endif

#if !CY_AFE_ENABLE_TUNING_FEATURE
cy_rslt_t afe_setup_audio_tuner_task(void* context)
{
	(void)context;
	return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_cleanup_audio_tuner_task(void* context)
{
	(void)context;
	return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_allocate_memory_for_dbg_output(void* context)
{
	(void)context;
	return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_cleanup_memory_for_dbg_output(void* context)
{
	(void)context;
	return CY_RSLT_SUCCESS;
}
#endif

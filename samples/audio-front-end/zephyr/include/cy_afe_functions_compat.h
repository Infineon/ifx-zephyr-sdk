#ifndef CY_AFE_FUNCTIONS_COMPAT_H_
#define CY_AFE_FUNCTIONS_COMPAT_H_

#include <stdbool.h>

static inline bool cy_afe_lib_is_license_expired(void)
{
	return false;
}

static inline bool cy_avc_lib_is_license_expired(void)
{
	return false;
}

#endif

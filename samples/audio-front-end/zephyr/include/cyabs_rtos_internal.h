#ifndef CYABS_RTOS_INTERNAL_H_
#define CYABS_RTOS_INTERNAL_H_

#include "cyabs_rtos.h"
#include "cyabs_rtos_impl.h"

#define CY_RTOS_THREAD_STACK_SIZE_DEFAULT 2048

static inline cy_rslt_t cy_rtos_init(void)
{
    return CY_RTOS_SUCCESS;
}

static inline cy_rslt_t cy_rtos_deinit(void)
{
    return CY_RTOS_SUCCESS;
}

#endif

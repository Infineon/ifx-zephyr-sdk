#ifndef CYABS_RTOS_IMPL_H_
#define CYABS_RTOS_IMPL_H_

#include <zephyr/kernel.h>
#include "cyabs_rtos.h"

#define CY_RTOS_MIN_STACK_SIZE 1024

static inline cy_time_t convert_ms_to_ticks(cy_time_t timeout_ms)
{
    return timeout_ms;
}

#endif

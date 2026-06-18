#ifndef CYABS_RTOS_H_
#define CYABS_RTOS_H_

#include <zephyr/kernel.h>
#include "cy_result.h"

typedef enum cy_thread_priority {
    CY_RTOS_PRIORITY_MIN         = 0,
    CY_RTOS_PRIORITY_LOW         = 2,
    CY_RTOS_PRIORITY_BELOWNORMAL = 3,
    CY_RTOS_PRIORITY_NORMAL      = 5,
    CY_RTOS_PRIORITY_ABOVENORMAL = 7,
    CY_RTOS_PRIORITY_HIGH        = 10,
    CY_RTOS_PRIORITY_REALTIME    = 12,
    CY_RTOS_PRIORITY_MAX         = 15
} cy_thread_priority_t;

typedef struct k_thread cy_thread_t;
typedef struct k_mutex cy_mutex_t;
typedef struct k_msgq cy_queue_t;
typedef struct k_timer cy_timer_t;
typedef struct k_sem cy_semaphore_t;
typedef void* cy_thread_arg_t;
typedef uint32_t cy_time_t;
typedef void (*cy_thread_entry_fn_t)(cy_thread_arg_t arg);

#define CY_RTOS_SUCCESS       (CY_RSLT_SUCCESS)
#define CY_RTOS_GENERAL_ERROR (CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, 1))
#define CY_RTOS_BAD_PARAM     (CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, 2))
#define CY_RTOS_NEVER_TIMEOUT (0xFFFFFFFFUL)

static inline bool is_in_isr(void)
{
    return k_is_in_isr();
}

static inline cy_rslt_t cy_rtos_create_thread(cy_thread_t* thread, cy_thread_entry_fn_t entry_function,
                                              const char* name, void* stack, uint32_t stack_size,
                                              cy_thread_priority_t priority, cy_thread_arg_t arg)
{
    (void)thread; (void)entry_function; (void)name; (void)stack; (void)stack_size; (void)priority; (void)arg;
    return CY_RTOS_SUCCESS;
}

static inline cy_rslt_t cy_rtos_thread_terminate(cy_thread_t* thread) { (void)thread; return CY_RTOS_SUCCESS; }
static inline cy_rslt_t cy_rtos_join_thread(cy_thread_t* thread) { (void)thread; return CY_RTOS_SUCCESS; }
static inline cy_rslt_t cy_rtos_thread_join(cy_thread_t* thread) { return cy_rtos_join_thread(thread); }
static inline cy_rslt_t cy_rtos_terminate_thread(cy_thread_t* thread) { (void)thread; return CY_RTOS_SUCCESS; }
static inline cy_rslt_t cy_rtos_delay_milliseconds(uint32_t ms) { k_msleep(ms); return CY_RTOS_SUCCESS; }

static inline cy_rslt_t cy_rtos_init_mutex(cy_mutex_t* mutex) { return k_mutex_init(mutex) == 0 ? CY_RTOS_SUCCESS : CY_RTOS_GENERAL_ERROR; }
static inline cy_rslt_t cy_rtos_get_mutex(cy_mutex_t* mutex, uint32_t timeout_ms)
{
    k_timeout_t timeout = (timeout_ms == CY_RTOS_NEVER_TIMEOUT) ? K_FOREVER : K_MSEC(timeout_ms);
    return k_mutex_lock(mutex, timeout) == 0 ? CY_RTOS_SUCCESS : CY_RTOS_GENERAL_ERROR;
}
static inline cy_rslt_t cy_rtos_set_mutex(cy_mutex_t* mutex) { return k_mutex_unlock(mutex) == 0 ? CY_RTOS_SUCCESS : CY_RTOS_GENERAL_ERROR; }
static inline cy_rslt_t cy_rtos_deinit_mutex(cy_mutex_t* mutex) { (void)mutex; return CY_RTOS_SUCCESS; }

static inline cy_rslt_t cy_rtos_init_queue(cy_queue_t* queue, size_t length, size_t item_size) { (void)queue; (void)length; (void)item_size; return CY_RTOS_SUCCESS; }
static inline cy_rslt_t cy_rtos_put_queue(cy_queue_t* queue, const void* item_ptr, uint32_t timeout_ms, bool in_isr)
{ (void)queue; (void)item_ptr; (void)timeout_ms; (void)in_isr; return CY_RTOS_SUCCESS; }
static inline cy_rslt_t cy_rtos_get_queue(cy_queue_t* queue, void* item_ptr, uint32_t timeout_ms, bool in_isr)
{ (void)queue; (void)item_ptr; (void)timeout_ms; (void)in_isr; return CY_RTOS_SUCCESS; }
static inline cy_rslt_t cy_rtos_deinit_queue(cy_queue_t* queue) { (void)queue; return CY_RTOS_SUCCESS; }

static inline cy_rslt_t cy_rtos_init_semaphore(cy_semaphore_t** semaphore, uint32_t max_count, uint32_t initial_count)
{
    static cy_semaphore_t static_semaphore;
    *semaphore = &static_semaphore;
    return k_sem_init(*semaphore, initial_count, max_count) == 0 ? CY_RTOS_SUCCESS : CY_RTOS_GENERAL_ERROR;
}
static inline cy_rslt_t cy_rtos_get_semaphore(cy_semaphore_t** semaphore, uint32_t timeout_ms, bool in_isr)
{
    (void)in_isr;
    if (!semaphore || !*semaphore) {
        return CY_RTOS_BAD_PARAM;
    }
    k_timeout_t timeout = (timeout_ms == CY_RTOS_NEVER_TIMEOUT) ? K_FOREVER : K_MSEC(timeout_ms);
    return k_sem_take(*semaphore, timeout) == 0 ? CY_RTOS_SUCCESS : CY_RTOS_GENERAL_ERROR;
}
static inline cy_rslt_t cy_rtos_set_semaphore(cy_semaphore_t* semaphore, bool in_isr)
{
    (void)in_isr;
    if (!semaphore) {
        return CY_RTOS_BAD_PARAM;
    }
    k_sem_give(semaphore);
    return CY_RTOS_SUCCESS;
}
static inline cy_rslt_t cy_rtos_deinit_semaphore(cy_semaphore_t** semaphore) { (void)semaphore; return CY_RTOS_SUCCESS; }

#define CY_SP_PRINTF(...) printk(__VA_ARGS__)

#endif

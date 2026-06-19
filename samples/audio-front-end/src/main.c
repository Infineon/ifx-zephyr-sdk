/*
 * Copyright (c) 2026 Infineon Technologies AG,
 * or an affiliate of Infineon Technologies AG.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

/* Include generated settings first to prevent redefinition warnings */
#include "generated/cy_afe_configurator_settings.h"

#include "cy_audio_front_end.h"
#include "cy_audio_front_end_error.h"
#include "cyabs_rtos.h"

/* AFE Test constants */
#define AFE_SEPARATOR "=====================================\n"

/* Forward declarations for memory callbacks */
cy_rslt_t afe_zephyr_alloc_memory(cy_afe_mem_id_t mem_id, uint32_t size, void **buffer);
cy_rslt_t afe_zephyr_free_memory(cy_afe_mem_id_t mem_id, void *buffer);

/* Test data for AFE API testing */
#define AFE_TEST_FRAME_SIZE 160  /* 10ms at 16kHz */
static int16_t test_audio_buffer[AFE_TEST_FRAME_SIZE];
static int16_t test_aec_buffer[AFE_TEST_FRAME_SIZE];

/* Test result tracking */
typedef struct {
    const char *test_name;
    cy_rslt_t result;
    bool executed;
} afe_test_result_t;

static afe_test_result_t test_results[] = {
    {"AFE Create", CY_RSLT_SUCCESS, false},
    {"AFE Feed", CY_RSLT_SUCCESS, false},
    {"AFE Delete", CY_RSLT_SUCCESS, false},
    {"Multi-Frame", CY_RSLT_SUCCESS, false},
    {"Memory Stress", CY_RSLT_SUCCESS, false},
    {"Config Test", CY_RSLT_SUCCESS, false},
    {"Error Handle", CY_RSLT_SUCCESS, false},
    {"Performance", CY_RSLT_SUCCESS, false},
};

#define TEST_CREATE_IDX     0
#define TEST_FEED_IDX       1
#define TEST_DELETE_IDX     2
#define TEST_MULTIFRAME_IDX 3
#define TEST_MEMSTRESS_IDX  4
#define TEST_CONFIG_IDX     5
#define TEST_ERRORHDL_IDX   6
#define TEST_PERF_IDX       7
#define NUM_TESTS       (sizeof(test_results)/sizeof(test_results[0]))

/* Test configuration with different parameters - for advanced testing */
static uint32_t test_filter_settings[] = {
    0x00000002,  /* Test configuration marker */
    0x00000001,  /* Enable some processing */
    0x00000000,  /* AEC still disabled */
    0x00000000   /* End marker */
};

/* Performance measurement */
static uint32_t perf_start_time, perf_end_time;

/* AFE Global Handle - matches ModusToolbox pattern */
cy_afe_t afe_handle = NULL;

/* Configure AFE with exact ModusToolbox initialization order */
cy_afe_config_t afe_config = {0};

/* Callback synchronization */
static volatile uint32_t output_callback_count = 0;
static volatile uint32_t expected_callback_count = 0;
static K_SEM_DEFINE(afe_processing_complete, 0, 1);


/* AFE output buffer info - global like ModusToolbox */
typedef struct {
    int16_t *output_buf;
    int16_t *input_buf; 
    int16_t *input_aec_ref_buf;
#if CY_AFE_ENABLE_TUNING_FEATURE
    int16_t *dbg_output1;
    int16_t *dbg_output2;
    int16_t *dbg_output3;
    int16_t *dbg_output4;
#endif
} afe_buffer_info_t;

static afe_buffer_info_t afe_output_buffer_info = {0};

/* Tuning callback stubs - only available when tuning feature is enabled */
#if CY_AFE_ENABLE_TUNING_FEATURE
static cy_rslt_t stub_notify_settings_callback(cy_afe_t handle, cy_afe_config_setting_t *config_setting, void *user_arg)
{
    /* Enhanced safety checks */
    if (handle == NULL || config_setting == NULL) {
        printk("AFE Warning: stub_notify_settings_callback called with NULL parameters\n");
        return CY_RSLT_SUCCESS;
    }
    
    printk("AFE Test: Tuning notify callback invoked (safely stubbed) - handle:%p\n", handle);
    return CY_RSLT_SUCCESS;
}

static cy_rslt_t stub_write_response_callback(cy_afe_t handle, cy_afe_tuner_buffer_t *response_buffer, void *user_arg)
{
    /* Enhanced safety checks */
    if (handle == NULL || response_buffer == NULL) {
        printk("AFE Warning: stub_write_response_callback called with NULL parameters\n");
        return CY_RSLT_SUCCESS;
    }
    
    printk("AFE Test: Tuning write response callback invoked (safely stubbed) - handle:%p\n", handle);
    return CY_RSLT_SUCCESS;
}

static cy_rslt_t stub_read_request_callback(cy_afe_t handle, cy_afe_tuner_buffer_t *request_buffer, void *user_arg)
{
    /* Enhanced safety checks */
    if (handle == NULL || request_buffer == NULL) {
        printk("AFE Warning: stub_read_request_callback called with NULL parameters\n");
        return CY_RSLT_SUCCESS;
    }
    
    printk("AFE Test: Tuning read request callback invoked (safely stubbed) - handle:%p\n", handle);
    return CY_RSLT_SUCCESS;
}
#endif

/* Zephyr-compatible AFE memory management - sizes based on ModusToolbox implementation */
#define AFE_CONTEXT_POOL_SIZE     8192    /* AFE context memory - increased for MW settings */
#define AFE_OUTPUT_POOL_SIZE      4096    /* AFE output buffers - increased */  
#define AFE_ALGORITHM_POOL_SIZE   80000   /* Algorithm persistent memory - reduced for RAM constraints */
#define AFE_SCRATCH_POOL_SIZE     40000   /* Algorithm scratch memory - matches ModusToolbox */
#define AFE_GENERIC_POOL_SIZE     20480   /* Generic allocations - increased for MW settings */

/* Static memory pools for AFE - Zephyr way */
static uint8_t afe_context_pool[AFE_CONTEXT_POOL_SIZE];
static uint8_t afe_output_pool[AFE_OUTPUT_POOL_SIZE]; 
static uint8_t afe_algorithm_pool[AFE_ALGORITHM_POOL_SIZE];
static uint8_t afe_scratch_pool[AFE_SCRATCH_POOL_SIZE];
static uint8_t afe_generic_pool[AFE_GENERIC_POOL_SIZE];

/* Memory pool usage tracking */
static struct {
    uint8_t *pool;
    uint32_t size;
    uint32_t used;
    const char *name;
} afe_memory_pools[] = {
    {afe_context_pool,   AFE_CONTEXT_POOL_SIZE,   0, "CONTEXT"},
    {afe_output_pool,    AFE_OUTPUT_POOL_SIZE,    0, "OUTPUT"},
    {afe_algorithm_pool, AFE_ALGORITHM_POOL_SIZE, 0, "ALGORITHM"},
    {afe_scratch_pool,   AFE_SCRATCH_POOL_SIZE,   0, "SCRATCH"},
    {afe_generic_pool,   AFE_GENERIC_POOL_SIZE,   0, "GENERIC"},
};

/* Simple pool allocator - Zephyr compatible */
static void* afe_pool_alloc(int pool_index, uint32_t size)
{
    if (pool_index < 0 || pool_index >= ARRAY_SIZE(afe_memory_pools)) {
        return NULL;
    }
    
    /* Fix pointer type compatibility - direct access to avoid struct copying */
    if (afe_memory_pools[pool_index].used + ((size + 3) & ~3) > afe_memory_pools[pool_index].size) {
        printk("AFE Memory: %s pool exhausted (need %u, have %u)\n", 
               afe_memory_pools[pool_index].name, (size + 3) & ~3, 
               afe_memory_pools[pool_index].size - afe_memory_pools[pool_index].used);
        return NULL;
    }
    
    void *ptr = afe_memory_pools[pool_index].pool + afe_memory_pools[pool_index].used;
    afe_memory_pools[pool_index].used += (size + 3) & ~3;
    
    printk("AFE Memory: Allocated %u bytes from %s pool (used: %u/%u)\n",
           (size + 3) & ~3, afe_memory_pools[pool_index].name, 
           afe_memory_pools[pool_index].used, afe_memory_pools[pool_index].size);
    
    return ptr;
}

/* AFE Memory allocation callback - Zephyr compatible */
cy_rslt_t afe_zephyr_alloc_memory(cy_afe_mem_id_t mem_id, uint32_t size, void **buffer)
{
    cy_rslt_t ret_val = CY_RSLT_SUCCESS;
    
    /* Match ModusToolbox parameter validation */
    if(NULL == buffer)
    {
        return ret_val;
    }
    *buffer = NULL;
    
    printk("AFE Memory: Allocation request - ID: %d, Size: %u\n", mem_id, size);
    
    /* Match ModusToolbox zero-size handling */
    if (size == 0)
    {
        printk("AFE Memory: No allocation as size is zero\n");
        *buffer = &afe_memory_pools[4].pool[0]; /* Use GENERIC pool base for temp memory */
        return ret_val;
    }

    const char* mem_id_names[] = {
        "AFE_CONTEXT", "AFE_OUTPUT_BUFFER", "AFE_DBG_OUT_BUFFER", 
        "ALGORITHM_PERSISTENT", "ALGORITHM_BF", "ALGORITHM_NS", "ALGORITHM_ES",
        "ALGORITHM_SCRATCH", "AFE_TUNER_CMD_BUFFER", "GENERIC_MEMORY", "GDE_PERSISTENT"
    };
    const char* mem_name = (mem_id < 11) ? mem_id_names[mem_id] : "UNKNOWN";
    
    /* Map AFE memory IDs to our static pools */
    switch (mem_id) {
        case CY_AFE_MEM_ID_AFE_CONTEXT:
            *buffer = afe_pool_alloc(0, size); /* CONTEXT pool */
            break;
            
        case CY_AFE_MEM_ID_AFE_OUTPUT_BUFFER:
        case CY_AFE_MEM_ID_AFE_DBG_OUT_BUFFER:
            *buffer = afe_pool_alloc(1, size); /* OUTPUT pool */
            break;
            
        case CY_AFE_MEM_ID_ALGORITHM_PERSISTENT_MEMORY:
        case CY_AFE_MEM_ID_ALGORITHM_BF_MEMORY:
        case CY_AFE_MEM_ID_ALGORITHM_NS_MEMORY:
        case CY_AFE_MEM_ID_ALGORITHM_ES_MEMORY:
        case CY_AFE_MEM_ID_GDE_PERSISTENT_MEM:
            *buffer = afe_pool_alloc(2, size); /* ALGORITHM pool */
            break;
            
        case CY_AFE_MEM_ID_ALGORITHM_SCRATCH_MEMORY:
            *buffer = afe_pool_alloc(3, size); /* SCRATCH pool */
            break;
            
        case CY_AFE_MEM_ID_AFE_TUNER_CMD_BUFFER:
        case CY_AFE_MEM_ID_GENERIC_MEMORY:
        default:
            *buffer = afe_pool_alloc(4, size); /* GENERIC pool */
            break;
    }
    
    if (*buffer == NULL) {
        printk("AFE Memory: Allocation failed for ID %d (%s), size %u\n", mem_id, mem_name, size);
        ret_val = -1;  /* Match ModusToolbox error return value */
    }
    else {
        printk("AFE Memory: Allocated %u bytes at %p for ID %d (%s)\n", size, *buffer, mem_id, mem_name);
    }
    
    return ret_val;
}

/* AFE Memory free callback - Zephyr compatible (match ModusToolbox pattern) */
cy_rslt_t afe_zephyr_free_memory(cy_afe_mem_id_t mem_id, void *buffer)
{
    /* Add safety validation to prevent bus faults */
    if (buffer == NULL) {
        printk("AFE Memory: Free request - NULL buffer (ID: %d)\n", mem_id);
        return CY_RSLT_SUCCESS;
    }

    /* Additional safety check: ensure address is reasonable */
    if ((uintptr_t)buffer < 0x20000000 || (uintptr_t)buffer > 0x30000000) {
        printk("AFE Memory: WARNING - Buffer %p outside expected RAM range (ID: %d)\n", buffer, mem_id);
        printk("AFE Memory: Skipping potentially dangerous free operation\n");
        return CY_RSLT_SUCCESS;
    }

    printk("AFE Memory: Free request - ID: %d, Buffer: %p\n", mem_id, buffer);
    
    /* Validate buffer is within our static pool ranges */
    bool valid_buffer = false;
    for (int i = 0; i < ARRAY_SIZE(afe_memory_pools); i++) {
        uint8_t *pool_start = afe_memory_pools[i].pool;
        uint8_t *pool_end = pool_start + afe_memory_pools[i].size;
        if ((uint8_t*)buffer >= pool_start && (uint8_t*)buffer < pool_end) {
            valid_buffer = true;
            printk("AFE Memory: Buffer %p is valid in %s pool\n", buffer, afe_memory_pools[i].name);
            break;
        }
    }
    
    if (!valid_buffer) {
        printk("AFE Memory: INFO - Buffer %p not in static pools (may be other allocation)\n", buffer);
        /* Don't return error as this might be expected for some memory types */
    }
    
    /* For static pools, we don't actually free - just mark as available for reuse */
    /* This prevents bus faults from accessing freed memory */
    
    return CY_RSLT_SUCCESS;
}

/* Reset callback tracking for new test run */
static void afe_reset_callback_tracking(uint32_t expected_callbacks)
{
    output_callback_count = 0;
    expected_callback_count = expected_callbacks;
    k_sem_reset(&afe_processing_complete);
    printk("AFE Sync: Expecting %u callbacks for this test\n", expected_callbacks);
}

/* Wait for AFE processing to complete */
static void afe_wait_for_processing_complete(void)
{
    printk("AFE Sync: Waiting for AFE processing to complete...\n");
    if (k_sem_take(&afe_processing_complete, K_SECONDS(5)) == 0) {
        printk("AFE Sync: Processing completed - %u callbacks received\n", output_callback_count);
    } else {
        printk("AFE Sync: Timeout waiting for processing - %u of %u callbacks received\n", 
               output_callback_count, expected_callback_count);
    }
}

/* Reset memory pools for clean AFE restart */
static void afe_reset_memory_pools(void)
{
    printk("AFE Memory: Resetting all memory pools\n");
    
    for (int i = 0; i < ARRAY_SIZE(afe_memory_pools); i++) {
        /* Clear usage tracking */
        afe_memory_pools[i].used = 0;
        
        /* Zero out pool memory to prevent stale data issues */
        memset(afe_memory_pools[i].pool, 0, afe_memory_pools[i].size);
        
        printk("AFE Memory: Reset %s pool (%u bytes available)\n", 
               afe_memory_pools[i].name, afe_memory_pools[i].size);
    }
    
    printk("AFE Memory: All pools reset successfully\n");
}

/* AFE API Test Functions */
static cy_rslt_t test_output_callback(cy_afe_t handle, cy_afe_buffer_info_t *output_buffer, void *user_arg)
{
    (void)handle;
    (void)user_arg;
    
    /* Track callback invocations */
    output_callback_count++;
    
    /* Update global buffer info like ModusToolbox pattern */
    afe_output_buffer_info.output_buf = (int16_t *) output_buffer->output_buf;
    afe_output_buffer_info.input_buf = (int16_t *) output_buffer->input_buf;
    afe_output_buffer_info.input_aec_ref_buf = (int16_t *) output_buffer->input_aec_ref_buf;
#if CY_AFE_ENABLE_TUNING_FEATURE
    afe_output_buffer_info.dbg_output1 = (int16_t *) output_buffer->dbg_output1;
    afe_output_buffer_info.dbg_output2 = (int16_t *) output_buffer->dbg_output2;
    afe_output_buffer_info.dbg_output3 = (int16_t *) output_buffer->dbg_output3;
    afe_output_buffer_info.dbg_output4 = (int16_t *) output_buffer->dbg_output4;
#endif
    
    printk("AFE Test: Output callback %u/%u invoked with buffer info: %p\n", 
           output_callback_count, expected_callback_count, output_buffer);
    if (output_buffer && output_buffer->output_buf) {
        printk("AFE Test: Output buffer available at %p\n", output_buffer->output_buf);
    }
    
    /* Signal when expected number of callbacks received */
    if (output_callback_count >= expected_callback_count) {
        printk("AFE Test: Expected callbacks received, signaling completion\n");
        k_sem_give(&afe_processing_complete);
    }
    
    return CY_RSLT_SUCCESS;
}

/* AFE Get Buffer callback - required for AFE operation */
static uint8_t afe_output_buffer[AFE_TEST_FRAME_SIZE * 2 * sizeof(int16_t)]; /* Global like ModusToolbox */

static cy_rslt_t afe_get_output_buffer_callback(cy_afe_t context, uint32_t **output_buffer, void *user_args)
{
    /* Use global output buffer like ModusToolbox pattern */
    (void)context;
    (void)user_args;
    
    *output_buffer = (uint32_t*)afe_output_buffer;
    printk("AFE Memory: Get buffer callback - provided buffer at %p\n", *output_buffer);
    return CY_RSLT_SUCCESS;
}

/* USB settings - disabled for basic AFE testing */
#ifdef CONFIG_USB_AFE_SUPPORT
/* USB settings would be defined here if USB support was enabled */
// afe_usb_settings_t MY_AFE_USB_SETTINGS = { ... };
#endif

/* Speech processing global variables - when disabled */
#ifndef CONFIG_AUDIOFE_SPEECH_ENH
void* cy_sp_alloc_memory = NULL;
void* cy_sp_free_memory = NULL;
#endif

/* Legacy memory functions - marked unused to avoid warnings */
__attribute__((unused)) static void* test_alloc_memory(uint32_t size)
{
    /* This should not be called - we provide proper AFE callbacks */
    printk("AFE Test: Unexpected legacy alloc call: %u bytes\n", size);
    return NULL;
}

__attribute__((unused)) static void test_free_memory(void* ptr)
{
    /* This should not be called - we provide proper AFE callbacks */
    printk("AFE Test: Unexpected legacy free call\n");
}

/* AFE error code decoder */
static const char* afe_decode_error(cy_rslt_t result)
{
    switch (result) {
        case CY_RSLT_SUCCESS: return "SUCCESS";
        case 0x085a0001: return "CY_RSLT_AFE_OUT_OF_MEMORY";
        case 0x085a0002: return "CY_RSLT_AFE_GENERIC_ERROR";
        case 0x085a0003: return "CY_RSLT_AFE_BAD_ARG";
        case 0x085a0004: return "CY_RSLT_AFE_ALREADY_INITIALIZED";
        case 0x085a0005: return "CY_RSLT_AFE_CRC_CHECKSUM_ERROR";
        case 0x085a0006: return "CY_RSLT_AFE_SPEECH_ENHANCEMENT_ERROR";
        case 0x085a0007: return "CY_RSLT_AFE_TUNER_INVALID_CMD";
        case 0x085a0008: return "CY_RSLT_AFE_TUNER_GENERIC_ERROR";
        default: return "UNKNOWN_ERROR";
    }
}

/* Test result reporting */
static void report_test_results(void)
{
    int passed = 0, failed = 0;
    
    printk("\n");
    printk(AFE_SEPARATOR);
    printk("AFE Test Results Summary:\n");
    printk(AFE_SEPARATOR);
    
    for (int i = 0; i < NUM_TESTS; i++) {
        if (test_results[i].executed) {
            if (test_results[i].result == CY_RSLT_SUCCESS) {
                printk("AFE Test: %-12s - PASS\n", test_results[i].test_name);
                passed++;
            } else {
                printk("AFE Test: %-12s - FAIL (0x%08x: %s)\n", 
                       test_results[i].test_name, test_results[i].result,
                       afe_decode_error(test_results[i].result));
                failed++;
            }
        } else {
            printk("AFE Test: %-12s - SKIPPED\n", test_results[i].test_name);
        }
    }
    
    printk(AFE_SEPARATOR);
    printk("AFE Test: Overall Results: %d PASSED, %d FAILED, %d SKIPPED\n", 
           passed, failed, NUM_TESTS - passed - failed);
    
    /* Production readiness assessment */
    if (failed == 0 && passed >= 5) {
        printk("AFE Test: *** ALL EXECUTED TESTS PASSED ***\n");
        printk("AFE Assessment: READY for basic application use\n");
        printk("AFE Note: For production, add real filter coefficients\n");
        printk("AFE Note: Test with actual audio hardware integration\n");
        printk("AFE Note: Validate real-time performance constraints\n");
    } else if (failed > 0) {
        printk("AFE Test: *** %d TEST(S) FAILED ***\n", failed);
        printk("AFE Assessment: NOT READY - Fix failures before use\n");
    } else {
        printk("AFE Assessment: MINIMAL TESTING - More tests recommended\n");
    }
    printk(AFE_SEPARATOR);
    printk("\n");
}

void afe_api_basic_test(void)
{
    printk(AFE_SEPARATOR);
    printk("AFE Test: Starting basic AFE API tests\n");
    printk(AFE_SEPARATOR);
    
    /* Reset memory pools for clean test */
    afe_reset_memory_pools();
    
    /* Calculate expected callbacks: 
     * 1 initial + 5 additional + 10 multi-frame = 16 frames total 
     * AFE processing can generate callbacks for processed data */
    afe_reset_callback_tracking(16);
    
    /* Initialize test data */
    for (int i = 0; i < AFE_TEST_FRAME_SIZE; i++) {
        test_audio_buffer[i] = (int16_t)(i % 1000);  /* Simple test pattern */
        test_aec_buffer[i] = 0;  /* No AEC for now */
    }
        
    /* Step 1: Filter settings first - exactly like ModusToolbox */
    afe_config.filter_settings = AFE_FILTER_SETTINGS;
    
    /* Step 2: MW settings to NULL initially - ModusToolbox pattern */
    afe_config.mw_settings = NULL;
    
    /* Step 3: Callbacks - exact ModusToolbox order */
    afe_config.afe_get_buffer_callback = afe_get_output_buffer_callback;
    afe_config.afe_output_callback = test_output_callback;
    afe_config.user_arg_callbacks = NULL;
    
    /* Step 4: Conditional MW settings - exact ModusToolbox pattern */
#if AFE_MW_SETTINGS_SIZE
    afe_config.mw_settings = AFE_MW_SETTINGS;
    afe_config.mw_settings_length = AFE_MW_SETTINGS_SIZE;
#else
    afe_config.mw_settings = NULL;
    afe_config.mw_settings_length = 0;
#endif /* AFE_MW_SETTINGS_SIZE */
    
    /* Step 5: Tuning configuration - only when tuning feature is available */
#if CY_AFE_ENABLE_TUNING_FEATURE
    /* Tuning feature is available - provide safe stubs with disabled background task */
    cy_afe_tuner_callbacks_t tuner_cb = {0}; /* Initialize to zero */
    tuner_cb.notify_settings_callback = stub_notify_settings_callback;
    tuner_cb.read_request_callback = stub_read_request_callback;
    tuner_cb.write_response_callback = stub_write_response_callback;
    afe_config.tuner_cb = tuner_cb;
    
    /* Disable background tuner task completely */
    afe_config.poll_interval_ms = 0;  /* 0 = no background task */
    printk("AFE Test: Tuning feature available but disabled (poll_interval_ms=0)\n");
#else
    /* Tuning feature completely disabled - no tuning types or fields exist */
    printk("AFE Test: Tuning feature completely disabled at compilation level\n");
    printk("AFE Test: AFE library compiled without tuning support\n");
    /* Note: tuner_cb field and poll_interval_ms don't exist in cy_afe_config_t when tuning disabled */
#endif
    
    /* Step 6: Memory functions LAST - exact ModusToolbox pattern */
    afe_config.alloc_memory = (cy_afe_alloc_memory_callback_t)afe_zephyr_alloc_memory;
    afe_config.free_memory = (cy_afe_free_memory_callback_t)afe_zephyr_free_memory;
    
    // /* Immediate validation of function pointer assignments */
    // printk("AFE Debug: Function pointer validation:\n");
    // printk("AFE Debug: afe_zephyr_alloc_memory raw addr: %p\n", (void*)afe_zephyr_alloc_memory);
    // printk("AFE Debug: afe_zephyr_free_memory raw addr: %p\n", (void*)afe_zephyr_free_memory);
    // printk("AFE Debug: afe_config.alloc_memory: %p\n", (void*)afe_config.alloc_memory);
    // printk("AFE Debug: afe_config.free_memory: %p\n", (void*)afe_config.free_memory);
    
    if (afe_config.alloc_memory == NULL) {
        printk("CRITICAL: afe_config.alloc_memory is NULL after assignment!\n");
        return;
    }
    if (afe_config.free_memory == NULL) {
        printk("CRITICAL: afe_config.free_memory is NULL after assignment!\n"); 
        return;
    }
    printk("AFE Debug: Memory functions successfully assigned - alloc:%p, free:%p\n", 
           (void*)afe_config.alloc_memory, (void*)afe_config.free_memory);
    
    /* Remove validation - ModusToolbox doesn't validate before create */
    /* Let AFE library do its own validation */
    
    cy_rslt_t result;
    
    /* Global AFE handle reset for clean test */
    afe_handle = NULL;
    printk("AFE Test: Using global AFE handle (like ModusToolbox) at %p\n", &afe_handle);
    
    /* Comprehensive debug configuration - check all possible fields */
    printk("AFE Test: Comprehensive configuration debug:\n");
    printk("  filter_settings: %p (size: %u, first: 0x%08x)\n", 
           afe_config.filter_settings, AFE_FILTER_SETTING_SIZE, 
           afe_config.filter_settings ? afe_config.filter_settings[0] : 0);
    printk("  mw_settings: %p (length: %u, first: 0x%02x)\n", 
           afe_config.mw_settings, afe_config.mw_settings_length,
           afe_config.mw_settings ? afe_config.mw_settings[0] : 0);
    printk("  afe_output_callback: %p\n", afe_config.afe_output_callback);
    printk("  afe_get_buffer_callback: %p\n", afe_config.afe_get_buffer_callback);
    printk("  user_arg_callbacks: %p\n", afe_config.user_arg_callbacks);
    printk("  alloc_memory: %p\n", afe_config.alloc_memory);
    printk("  free_memory: %p\n", afe_config.free_memory);
    
    /* Check for additional fields that might be required */
    printk("AFE Debug: Structure size check: %u bytes\n", (uint32_t)sizeof(afe_config));
    
#if CY_AFE_ENABLE_TUNING_FEATURE  
    printk("  tuner_cb.notify: %p\n", afe_config.tuner_cb.notify_settings_callback);
    printk("  tuner_cb.read: %p\n", afe_config.tuner_cb.read_request_callback);  
    printk("  tuner_cb.write: %p\n", afe_config.tuner_cb.write_response_callback);
    printk("  poll_interval_ms: %u\n", afe_config.poll_interval_ms);
#else
    printk("  tuning: DISABLED (no tuning fields in structure)\n");
#endif

    /* Try to detect if there are uninitialized fields by dumping structure */
    printk("AFE Debug: Raw config dump (first 64 bytes):\n");
    uint8_t *config_bytes = (uint8_t*)&afe_config;
    for (int i = 0; i < 64 && i < sizeof(afe_config); i += 16) {
        printk("AFE Debug[%02d]: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", 
               i,
               config_bytes[i], config_bytes[i+1], config_bytes[i+2], config_bytes[i+3],
               config_bytes[i+4], config_bytes[i+5], config_bytes[i+6], config_bytes[i+7], 
               config_bytes[i+8], config_bytes[i+9], config_bytes[i+10], config_bytes[i+11],
               config_bytes[i+12], config_bytes[i+13], config_bytes[i+14], config_bytes[i+15]);
    }
    
    /* TEST: Call AFE create with exact ModusToolbox pattern */
    printk("AFE Test: About to call cy_afe_create() with ModusToolbox-pattern config\n");
    printk("AFE Test: Config address: %p, Handle address: %p\n", &afe_config, &afe_handle);
    
    /* 
     * NOTE: If ModusToolbox build was available, we could add identical debug output
     * there to compare:
     * - Structure field offsets and values
     * - Raw memory dump of working afe_config
     * - Callback function addresses and signatures  
     * - Any additional required fields we might be missing
     */
    
    /* Pre-validation debug */
    printk("AFE Debug: Pre-create validation:\n");
    printk("  config ptr: %p\n", &afe_config);
    printk("  handle ptr: %p\n", &afe_handle);
    printk("  afe_output_callback: %p\n", afe_config.afe_output_callback);
    printk("  afe_get_buffer_callback: %p\n", afe_config.afe_get_buffer_callback);
    printk("  filter_settings ptr: %p (AFE_FILTER_SETTINGS)\n", afe_config.filter_settings);
    printk("  alloc_memory ptr: %p (afe_zephyr_alloc_memory)\n", afe_config.alloc_memory);
    printk("  free_memory ptr: %p (afe_zephyr_free_memory)\n", afe_config.free_memory);
    
    /* Verify function pointer validity by testing addresses */
    printk("AFE Debug: Function address verification:\n");
    printk("  afe_zephyr_alloc_memory addr: %p\n", (void*)afe_zephyr_alloc_memory);
    printk("  afe_zephyr_free_memory addr: %p\n", (void*)afe_zephyr_free_memory);
    printk("  config.alloc_memory: %p\n", (void*)afe_config.alloc_memory);
    printk("  config.free_memory: %p\n", (void*)afe_config.free_memory);
    
    printk("AFE Debug: global afe_handle before create: %p\n", afe_handle);
    
    test_results[TEST_CREATE_IDX].executed = true;
    printk("AFE Debug: Calling cy_afe_create() now...\n");
    
    /* Final validation right before cy_afe_create call */
    printk("AFE Debug: PRE-CREATE function pointer check:\n");
    printk("AFE Debug: afe_config.alloc_memory = %p\n", (void*)afe_config.alloc_memory);
    printk("AFE Debug: afe_config.free_memory = %p\n", (void*)afe_config.free_memory);
    
    /* Validate function pointers were assigned correctly */
    if (afe_config.alloc_memory == NULL || afe_config.free_memory == NULL) {
        printk("AFE FATAL: Memory function pointers are NULL before create\n");
        printk("AFE FATAL: alloc=%p, free=%p\n", 
               (void*)afe_config.alloc_memory, (void*)afe_config.free_memory);
        return;
    }
    
    result = cy_afe_create(&afe_config, &afe_handle);
    
    printk("AFE Debug: cy_afe_create() call completed\n");
    test_results[TEST_CREATE_IDX].result = result;
    
    printk("AFE Test: cy_afe_create() returned 0x%08x\n", result);
    
    if (result == CY_RSLT_SUCCESS) {
        printk("AFE Test: cy_afe_create() - SUCCESS (global handle: %p)\n", afe_handle);
    } else {
        printk("AFE Test: cy_afe_create() - FAILED (0x%08x: %s)\n", 
               result, afe_decode_error(result));
        printk("AFE Test: Detailed failure analysis:\n");
        printk("         - Filter settings: %p (first value: 0x%08x)\n", 
               afe_config.filter_settings, afe_config.filter_settings[0]);
        printk("         - MW settings: %p (first byte: 0x%02x)\n", 
               afe_config.mw_settings, afe_config.mw_settings[0]);
        printk("         - MW length: %u (expected: %u)\n", afe_config.mw_settings_length, AFE_MW_SETTINGS_SIZE);
        printk("         - Output callback: %p\n", afe_config.afe_output_callback);
        printk("         - Get buffer callback: %p\n", afe_config.afe_get_buffer_callback);
        printk("         - Alloc memory: %p\n", afe_config.alloc_memory);
        printk("         - Free memory: %p\n", afe_config.free_memory);
        goto test_cleanup; /* Continue to report results even if create fails */
    }
    
    /* Test 2: AFE Feed - only if create succeeded */
    if (afe_handle != NULL && test_results[TEST_CREATE_IDX].result == CY_RSLT_SUCCESS) {
        printk("AFE Test: Testing cy_afe_feed()...\n");
        test_results[TEST_FEED_IDX].executed = true;
        result = cy_afe_feed(afe_handle, test_audio_buffer, test_aec_buffer);
        test_results[TEST_FEED_IDX].result = result;
        if (result == CY_RSLT_SUCCESS) {
            printk("AFE Test: cy_afe_feed() - SUCCESS\n");
        } else {
            printk("AFE Test: cy_afe_feed() - FAILED (0x%08x: %s)\n", 
                   result, afe_decode_error(result));
        }
        
        /* Test 3: Additional processing with main AFE instance */
        printk("AFE Test: Testing additional processing cycles...\n");
        test_results[TEST_DELETE_IDX].executed = false; /* Will be done at end */
        
        /* Process several more frames to test stability */
        for (int extra_frame = 0; extra_frame < 5; extra_frame++) {
            /* Generate varying test pattern */
            for (int i = 0; i < AFE_TEST_FRAME_SIZE; i++) {
                test_audio_buffer[i] = (int16_t)((extra_frame * 200 + i) % 1500 - 750);
            }
            
            result = cy_afe_feed(afe_handle, test_audio_buffer, test_aec_buffer);
            if (result != CY_RSLT_SUCCESS) {
                printk("AFE Test: Additional processing failed at frame %d\n", extra_frame);
                break;
            }
        }
        
        if (result == CY_RSLT_SUCCESS) {
            printk("AFE Test: Additional processing cycles - SUCCESS\n");
        } else {
            printk("AFE Test: Additional processing cycles - FAILED (0x%08x)\n", result);
        }
    } else {
        printk("AFE Test: Skipping AFE Feed tests due to create failure\n");
    }
    
    /* Test 4: Multi-frame processing using main AFE instance (if basic tests passed) */
    if (test_results[TEST_CREATE_IDX].result == CY_RSLT_SUCCESS && afe_handle != NULL) {
        printk("AFE Test: Testing continuous multi-frame processing with main AFE...\n");
        test_results[TEST_MULTIFRAME_IDX].executed = true;
        
        /* Use existing main AFE instance instead of creating new one */
        bool multi_success = true;
        result = CY_RSLT_SUCCESS;
        /* Process 10 frames continuously with main AFE instance */
        for (int frame = 0; frame < 10; frame++) {
            /* Generate different test pattern for each frame */
            for (int i = 0; i < AFE_TEST_FRAME_SIZE; i++) {
                test_audio_buffer[i] = (int16_t)((frame * 100 + i) % 2000 - 1000);
            }
            
            result = cy_afe_feed(afe_handle, test_audio_buffer, test_aec_buffer);
            if (result != CY_RSLT_SUCCESS) {
                printk("AFE Test: Multi-frame failed at frame %d (0x%08x)\n", frame, result);
                multi_success = false;
                break;
            }
        }
        test_results[TEST_MULTIFRAME_IDX].result = multi_success ? CY_RSLT_SUCCESS : result;
        printk("AFE Test: Multi-frame processing %s\n", 
               multi_success ? "SUCCESS" : "FAILED");
        /* Keep main AFE instance alive for other tests */
    } else {
        printk("AFE Test: Skipping multi-frame test - no valid AFE instance\n");
        test_results[TEST_MULTIFRAME_IDX].result = CY_RSLT_TYPE_ERROR;
    }

test_cleanup:
    /* Wait for all AFE output callbacks to complete before cleanup */
    if (afe_handle != NULL && expected_callback_count > 0) {
        printk("AFE Test: Waiting for AFE processing pipeline to complete...\n");
        afe_wait_for_processing_complete();
    }
    
    /* Final cleanup: Try AFE deletion with tuning completely disabled */
    // if (afe_handle != NULL) {
    //     printk("AFE Test: Deleting main AFE instance (tuning disabled, should be safer)...\n");
    //     printk("AFE Debug: Deleting AFE handle: %p\n", afe_handle);
        
    //     /* Mark delete test as executed */
    //     test_results[TEST_DELETE_IDX].executed = true;
        
    //     /* Try deletion - should be safe with no tuner task */
    //     cy_rslt_t result = cy_afe_delete(&afe_handle);
    //     test_results[TEST_DELETE_IDX].result = result;
        
    //     if (result == CY_RSLT_SUCCESS) {
    //         printk("AFE Test: cy_afe_delete() - SUCCESS (handle now: %p)\n", afe_handle);
    //     } else {
    //         printk("AFE Test: cy_afe_delete() - FAILED (0x%08x: %s)\n", 
    //                result, afe_decode_error(result));
    //         printk("AFE Note: Deletion failed - keeping handle active for debugging\n");
    //     }
    // } else {
    //     printk("AFE Test: No main AFE instance to clean up\n");
    // }
    
    /* Always report test results */
    report_test_results();
}

#ifdef CONFIG_AVC
#include "cy_audio_license.h"
/* License function declarations */
bool cy_afe_lib_is_license_expired(void);
bool cy_avc_lib_is_license_expired(void);
#endif

/* 1000 msec = 1 sec */
#define LED_SLEEP_TIME_MS   1000

/* Thread stack size and priority */
#define STACKSIZE 1024
#define PRIORITY 7

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* LED blink thread function */
void led_blink_thread(void)
{
	int ret;
	bool led_state = false;
	int blink_count = 0;

	printk("AFE LED Blink Thread: Starting LED blink thread\n");

	if (!gpio_is_ready_dt(&led)) {
		return;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}


	while (1) {
		ret = gpio_pin_toggle_dt(&led);

		led_state = !led_state;
		blink_count++;
		k_msleep(LED_SLEEP_TIME_MS);
	}
}

/* Define the LED blink thread */
K_THREAD_DEFINE(led_blink_thread_id, STACKSIZE, led_blink_thread, NULL, NULL, NULL,
		PRIORITY, 0, 0);

int main(void)
{
	printk("AFE Sample Application Starting\n");
	printk("===================================\n");
	printk("AFE Board: %s\n", CONFIG_BOARD);
	printk("AFE Main: LED blink thread started in background\n");
	printk("AFE Main: Application running...\n");

#ifdef CONFIG_AUDIOFE
	printk("AFE Main: Audio Front-End module is enabled\n");
	printk("AFE Main: Testing basic AFE functionality (no speech enhancement)...\n");
	
#ifdef CONFIG_AVC
	/* Test license functions from audio-voice-core */
	bool afe_license_expired = cy_afe_lib_is_license_expired();
	bool avc_license_expired = cy_avc_lib_is_license_expired();
	printk("AFE Main: AFE License check: %s\n", afe_license_expired ? "EXPIRED" : "VALID");
	printk("AFE Main: AVC License check: %s\n", avc_license_expired ? "EXPIRED" : "VALID");
#else
	printk("AFE Main: AVC module disabled - basic AFE testing only\n");
#endif
	
	/* Run basic AFE API tests */
	printk("AFE Main: Running AFE API tests with Zephyr static memory pools...\n");
	afe_api_basic_test();
	
	/* Simple test to verify module integration */
	printk("AFE Main: Audio Front-End basic integration test completed\n");
#else
	printk("AFE Main: Audio Front-End module is not enabled\n");
#endif

	/* Counter for main thread activity */
	int main_count = 0;

	while (1) {
		main_count++;
		// printk("AFE Main: Main thread heartbeat: %d\n", main_count);
		
		/* Sleep for 5 seconds to show main thread is active but not interfering */
		k_msleep(5000);
	}

	return 0;
}
#ifndef BULK_DELAY_MEASUREMENT_H
#define BULK_DELAY_MEASUREMENT_H

#include "cy_result.h"
#include <stdint.h>

#define AFE_BDM_SUCCESS 0

typedef struct {
    int32_t valid_delay_count;
} bdm_struct_t;

typedef struct {
    uint32_t frame_size;
    uint32_t sampling_rate;
} param_struct_t;

static inline cy_rslt_t afe_bdm_init(param_struct_t *param, bdm_struct_t *bdm)
{
    (void)param;
    if (bdm) {
        bdm->valid_delay_count = 0;
    }
    return CY_RSLT_SUCCESS;
}

static inline int16_t* afe_bdm_get_ref_signal(bdm_struct_t *bdm) { (void)bdm; return NULL; }
static inline int32_t afe_bdm_get_ref_length(bdm_struct_t *bdm) { (void)bdm; return 0; }
static inline int afe_bdm_get_bulk_delay(bdm_struct_t *bdm) { (void)bdm; return 0; }
static inline int afe_bdm_get_state(bdm_struct_t *bdm) { (void)bdm; return AFE_BDM_SUCCESS; }
static inline cy_rslt_t afe_bdm_free(bdm_struct_t *bdm) { (void)bdm; return CY_RSLT_SUCCESS; }
static inline cy_rslt_t afe_bdm_process(bdm_struct_t *bdm_struct, void *aec_ref_input, void *input, void *output)
{ (void)bdm_struct; (void)aec_ref_input; (void)input; (void)output; return CY_RSLT_SUCCESS; }

#endif

#ifndef CY_AFE_TYPES_COMPAT_H_
#define CY_AFE_TYPES_COMPAT_H_

#include <stdbool.h>
#include "cy_result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct afe_usb_settings_s {
    int channel_0;
    int channel_1;
    int channel_2;
    int channel_3;
    int unused;
} afe_usb_settings_t;

#define AFE_USB_SELECT_SIG_A_0  0
#define AFE_USB_SELECT_SIG_A_1  1
#define AFE_USB_SELECT_SIG_B    2
#define AFE_USB_SELECT_SIG_C    3

#ifndef AFE_USB_SELECT_INPUT_0
#define AFE_USB_SELECT_INPUT_0 0
#define AFE_USB_SELECT_INPUT_1 1
#define AFE_USB_SELECT_AEC_REF 2
#define AFE_USB_SELECT_OUTPUT  4
#define AFE_INPUT_SOURCE       0
#define AFE_USE_USB_AEC_REF    0
#define AFE_UART_BAUDRATE      115200
#define AFE_USE_TARGET_SPEAKER 0
#endif

extern afe_usb_settings_t MY_AFE_USB_SETTINGS;

#ifndef CONFIG_AUDIOFE_SPEECH_ENH
typedef int ifx_sp_mem_id;
typedef struct cy_sp_enh_config_params_s {
    int sampling_rate;
    int input_frame_size;
    int num_mics;
} cy_sp_enh_config_params;
typedef void* cy_sp_enh_handle;

static inline int cy_sp_enh_init(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
static inline int cy_sp_enh_process(void *a, void *b, void *c, void *d, void *e) { (void)a; (void)b; (void)c; (void)d; (void)e; return 0; }
static inline int cy_sp_enh_deinit(void *a) { (void)a; return 0; }
static inline int cy_sp_enh_enable_disable_component(void *a, int b, int c) { (void)a; (void)b; (void)c; return 0; }
static inline int cy_sp_enh_update_config_value(void *a, int b, void *c) { (void)a; (void)b; (void)c; return 0; }
static inline int cy_sp_enh_get_config_value(void *a, int b, void *c) { (void)a; (void)b; (void)c; return 0; }
static inline int cy_sp_enh_get_component_status(void *a, int b, void *c) { (void)a; (void)b; (void)c; return 0; }
static inline int cy_sp_enh_configure_dbg_out(void *a, int b, int c) { (void)a; (void)b; (void)c; return 0; }

extern void* cy_sp_alloc_memory;
extern void* cy_sp_free_memory;

cy_rslt_t afe_speech_enhancement_init(int32_t* filter_settings, uint8_t* mw_settings,
                                      uint32_t mw_settings_length, void** context);
cy_rslt_t afe_speech_enhancement_deinit(void* context);
cy_rslt_t afe_speech_enhancement_process(void* context, void* input_output);
cy_rslt_t afe_speech_enhancement_enable_disable_component(void* handle, int component, int enable);
cy_rslt_t afe_speech_enhancement_update_config_value(void* handle, int component, void* value);
cy_rslt_t afe_speech_enhancement_get_config_value(void* handle, int component, void* value);
cy_rslt_t afe_speech_enhancement_update_dbg_out_config(void* sp_enh_context, afe_usb_settings_t* usb_settings);
cy_rslt_t afe_speech_enhancement_get_component_status(void* handle, int component, int* enable);
#endif

#ifdef __cplusplus
}
#endif

#endif

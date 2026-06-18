#ifndef CY_AFE_CONFIGURATOR_SETTINGS_H__
#define CY_AFE_CONFIGURATOR_SETTINGS_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AFE_FRAME_SIZE_MS
#define AFE_FRAME_SIZE_MS         (10)
#endif

#ifndef AFE_FRAME_RATE_SPS
#define AFE_FRAME_RATE_SPS        (16000)
#endif

#ifndef AFE_INPUT_NUMBER_CHANNELS
#define AFE_INPUT_NUMBER_CHANNELS (2)
#endif

#ifndef CY_AFE_ENABLE_TUNING_FEATURE
#ifdef CONFIG_AUDIOFE_TUNING
#define CY_AFE_ENABLE_TUNING_FEATURE (1u)
#else
#define CY_AFE_ENABLE_TUNING_FEATURE (0u)
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif

#ifndef CY_AUDIO_LICENSE_H_
#define CY_AUDIO_LICENSE_H_

#define CY_AUDIO_LICENSE_VALID 1

static inline int cy_audio_license_check(void)
{
    return CY_AUDIO_LICENSE_VALID;
}

#endif

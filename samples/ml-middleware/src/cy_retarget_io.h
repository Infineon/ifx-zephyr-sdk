/*
 * Stub for cy_retarget_io.h — not needed on Zephyr.
 * In ModusToolbox, cy_retarget_io routes printf to UART.
 * On Zephyr, printf is handled via picolibc -> Zephyr console.
 * DEEPCRAFT-generated model.c includes this header but does not
 * call any cy_retarget_io functions at runtime.
 */
#ifndef CY_RETARGET_IO_H
#define CY_RETARGET_IO_H

/* intentionally empty */

#endif /* CY_RETARGET_IO_H */

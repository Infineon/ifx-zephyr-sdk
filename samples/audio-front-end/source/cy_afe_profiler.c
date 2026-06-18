/******************************************************************************
* File Name: cy_afe_profiler.c
*
* Description: This file contains functions for Speech onset detection.
*
*
*
*******************************************************************************
* (c) 2021, Infineon Technologies Company. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Infineon Technologies Company (Infineon) or one of its
* subsidiaries and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Infineon hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Infineon's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Infineon.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Infineon
* reserves the right to make changes to the Software without notice. Infineon
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Infineon does
* not authorize its products for use in any products where a malfunction or
* failure of the Infineon product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Infineon's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Infineon against all liability.
*******************************************************************************/

/*******************************************************************************
* Include header file
******************************************************************************/

#ifdef COMPONENT_PROFILER
#include "cy_afe_profiler.h"
#include "cy_profiler.h"
#include "cy_afe_audio_log_msg.h"
#include <inttypes.h>
#include <stdio.h>
#define CY_CYCLE_MAX(x, y) (((x) > (y)) ? (x) : (y))

/******************************************************************************
* Defines
*****************************************************************************/
#define AFE_1SEC_FRAME_COUNT    (100)
#define AFE_MCPS_FOR_1SEC       (1000000)
/******************************************************************************
* Constants
*****************************************************************************/

/******************************************************************************
* Variables
*****************************************************************************/
uint32_t afe_cycle_1s=0;
uint32_t afe_cycle_count=0;
/******************************************************************************
* Functions
*****************************************************************************/

bool afe_profile_on = false;
afe_profile_data_t afe_profile = {0};

cy_rslt_t cy_afe_profile(afe_profile_command cmd,
        afe_profile_data_t *data)
{
    switch(cmd)
    {
        case AFE_PROFILE_CMD_ENABLE:
        {
            afe_profile.cycles_taken = 0;
            afe_profile.profile_frame_counter = 0;
            afe_profile.max_cycles = 0;
            afe_profile.tot_cycles = 0;
            afe_profile_on = true;
            break;
        }
        case AFE_PROFILE_CMD_DISABLE:
        {
            afe_profile_on = false;
            break;
        }

        case AFE_PROFILE_CMD_START:
        {
            if(true == afe_profile_on)
            {
                afe_profile.profile_frame_counter++;
                cy_profiler_start();
            }
            break;
        }
        case AFE_PROFILE_CMD_STOP:
        {
            if(true == afe_profile_on)
            {
                uint32_t cycles = 0;
                cy_profiler_stop();
                cycles = cy_profiler_get_cycles();
                afe_profile.cycles_taken += cycles;
                afe_profile.tot_cycles += cycles;
                afe_profile.max_cycles = CY_CYCLE_MAX(afe_profile.max_cycles,cycles);
//                printf("AFE-Profile:Cntr:%u, T.Cycles:%u T.Cycles64:%f MaxCyc:%u, Cycles:%u\n",
//                                     afe_profile.profile_frame_counter,
//                                     afe_profile.cycles_taken,
//                                     (double)afe_profile.tot_cycles,
//                					 afe_profile.max_cycles,
//									 cycles);
            }
            break;
        }
        case AFE_PROFILE_CMD_GET_DATA:
        {
            if(true == afe_profile_on)
            {
                if(NULL != data)
                {
                    data->cycles_taken = afe_profile.cycles_taken;
                    data->tot_cycles = afe_profile.tot_cycles;
                    data->max_cycles = afe_profile.max_cycles;
                    data->profile_frame_counter = afe_profile.profile_frame_counter;
                }
            }
            break;
        }
        case AFE_PROFILE_CMD_RESET:
        {
            if(true == afe_profile_on)
            {
                afe_profile.cycles_taken = 0;
                afe_profile.profile_frame_counter = 0;
                afe_profile.max_cycles = 0;
                afe_profile.tot_cycles = 0;
            }
            break;
        }
        case AFE_PROFILE_CMD_PRINT_STATS:
        {
             cy_afe_log_info("AFEProfile:Cntr:%u, Cycles:%u Cycles64:%f PeakCyc:%u",
                     afe_profile.profile_frame_counter,
                     afe_profile.cycles_taken,
                     (double)afe_profile.tot_cycles,
                     afe_profile.max_cycles );
             printf("AFE-Profile:Cntr:%u, Cycles:%u Cycles64:%f PeakCyc:%u\n",
                     afe_profile.profile_frame_counter,
                     afe_profile.cycles_taken,
                     (double)afe_profile.tot_cycles,
                     afe_profile.max_cycles );

             break;
        }
        case AFE_PROFILE_CMD_PRINT_STATS_1SEC:
        {
            if (afe_cycle_count<=AFE_1SEC_FRAME_COUNT)
            {
                afe_cycle_1s+=afe_profile.cycles_taken;
                afe_cycle_count++;
            }
            else
            {
                afe_cycle_count=0;
                printf("Average MCPS is %u \n",afe_cycle_1s/AFE_MCPS_FOR_1SEC);
                afe_cycle_1s=0;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return CY_RSLT_SUCCESS;
}

#endif

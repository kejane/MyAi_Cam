/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-02     RT-Thread    first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include "tim.h"


#define LED_PIN GET_PIN(I, 8)//LED PIN
#define FM_PIN  GET_PIN(B,12)//FM PIN
extern void camera_on(void);
void wlan_autoconnect_init(void);
extern void wlan_autoconnect_init(void);
rt_thread_t tid_cam = RT_NULL;
rt_thread_t tid_IR=RT_NULL;
rt_uint8_t person;



int main(void)
{
    rt_thread_mdelay(5000);
    wlan_autoconnect_init();
    rt_wlan_config_autoreconnect(RT_TRUE);

    rt_wlan_connect("shwjsn", "3455633727Tc");
    rt_wlan_connect("shwjsn", "3455633727Tc");
    rt_wlan_connect("shwjsn", "3455633727Tc");
    rt_wlan_connect("shwjsn", "3455633727Tc");

    if(rt_wlan_connect("shwjsn", "3455633727Tc") == RT_EOK){
        rt_thread_mdelay(1000);
        for(int cnt = 0;cnt<4;cnt++){
            rt_pin_write(LED_PIN, PIN_HIGH);
            rt_thread_mdelay(250);
            rt_pin_write(LED_PIN, PIN_LOW);
            rt_thread_mdelay(250);
        }
    }


    MX_TIM4_Init();
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(FM_PIN,PIN_MODE_OUTPUT);
    rt_pin_write(FM_PIN, PIN_HIGH);

    camera_on();


    return RT_EOK;
}


#include "stm32h7xx.h"
static int vtor_config(void)
{
    /* Vector Table Relocation in Internal QSPI_FLASH */
    SCB->VTOR = QSPI_BASE;
    return 0;
}
INIT_BOARD_EXPORT(vtor_config);



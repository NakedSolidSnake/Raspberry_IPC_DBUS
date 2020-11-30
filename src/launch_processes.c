/**
 * @file led_process.c
 * @author Cristiano Silva de Souza (cristianosstec@gmail.com)
 * @brief Realiza a leitura da Shared Memory em modo
 * polling e aplica o estado ao led,
 * sendo seu estado alterado pelo processo de 
 * button_process
 * @version 1.0
 * @date 2020-02-09
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <led.h>
#include <shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

LED_t led =
    {
        .gpio.pin = 0,
        .gpio.eMode = eModeOutput
    };

static SHMemory_t shm =
    {
    .path = "/shm_gpio",
    .projId = 65,
    .size = 1024,
    .flags = 0666
    };

int main(int argc, char const *argv[])
{
    int state_cur;
    int state_old;

    if(LED_init(&led))
        return EXIT_FAILURE;

    if(SHMemory_init(&shm))
        return EXIT_FAILURE;

    while(1)
    {
        sscanf(shm.shm, "state = %d", &state_cur);
        if(state_cur !=  state_old)
        {

            state_old = state_cur;
            LED_set(&led, (eState_t)state_cur);            
        }
        usleep(1);
    }

    SHMemory_detach(&shm);    

    return EXIT_SUCCESS;
}

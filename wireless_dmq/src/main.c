
#ifdef CONF_BT_PERIPHERAL

// INCLUDES ------------------------------------------------------------------------------------------------------------------------------------------------------

#include <zephyr/kernel.h>                                            // Proporciona APIs para los thread, timers, sincronización y otras funciones del sistema operativo Zephyr
#include <zephyr/logging/log.h>                                       // Incluye las funciones y macros del modulo LOG

#include "BLE_peripheral.h"                                                 

// DEFINES ------------------------------------------------------------------------------------------------------------------------------------------------------

#define LOG_MAIN_LEVEL 3                                              // Nilvel del LOG 
LOG_MODULE_REGISTER(LOG_MAIN,LOG_MAIN_LEVEL);                         // Registro del modulo LOG y configuracion del nivel

// FUNCTIONS ------------------------------------------------------------------------------------------------------------------------------------------------------

int main(void)
{
     LOG_INF("\n\n\n\n\n\n\n\n\n\n\n\n---------- APP DMQ WIRELESS STARTING ----------\n");

     for(;;)
     {
          BLE_manager();
          k_sleep(K_MSEC(1));                                         
     }
     return 0;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------

#endif//CONF_BT_PERIPHERAL

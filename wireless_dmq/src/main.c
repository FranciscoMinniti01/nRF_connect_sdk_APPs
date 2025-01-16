
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
     
     uint8_t sensor_id[3] = {0x12,0x34,0x56};
     update_adv_manufacturer_data(sensor_id, sizeof(sensor_id));

     for(;;)
     {
          BLE_manager();
          
          static bool flag = false;
          static uint8_t sensor_id[3] = {0x12,0x34,0x56};
          if(!flag) flag = update_adv_manufacturer_data(sensor_id, sizeof(sensor_id));

          k_sleep(K_MSEC(1));                                         
     }
     return 0;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------

#endif//CONF_BT_PERIPHERAL


#ifdef CONF_BT_CENTRAL

// INCLUDES ------------------------------------------------------------------------------------------------------------------------------------------------------

#include <zephyr/kernel.h>                                            // Proporciona APIs para los thread, timers, sincronización y otras funciones del sistema operativo Zephyr
#include <zephyr/logging/log.h>                                       // Incluye las funciones y macros del modulo LOG

#include "BLE_central.h"                                                 

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

          static bool flag = false;
          static uint8_t sensor_id[3] = {0x12,0x34,0x56};
          if(!flag) flag = update_scan_manufacturer_data(sensor_id, sizeof(sensor_id));

          k_sleep(K_MSEC(1));                                         
     }
     return 0;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#endif//CONF_BT_CENTRAL


#ifdef CONF_BT_CENTRAL_PERIPHERAL

// INCLUDES ------------------------------------------------------------------------------------------------------------------------------------------------------

#include <zephyr/kernel.h>                                            // Proporciona APIs para los thread, timers, sincronización y otras funciones del sistema operativo Zephyr
#include <zephyr/logging/log.h>                                       // Incluye las funciones y macros del modulo LOG

#include "BLE_central_peripheral.h"                                                 

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

#endif//CONF_BT_CENTRAL_PERIPHERAL
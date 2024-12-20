#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

// ROLE CONFIGURATION ------------------------------------------------------------------------------------------------------------------------------------------------------

//#define BLE_CONF_ROLE_CENTRAL                                         // Habilita el funcionamiento del dispositivo como central
#define BLE_CONF_ROLE_PERIPHERAL                                      // Habilita el funcionamiento del dispositivo como periferico


// INCLUDES ------------------------------------------------------------------------------------------------------------------------------------------------------

#include <stdint.h>                                                   // Define los tipos de enteros de tamaño fijo uint8_t, uint16_t, ...    
#include <zephyr/logging/log.h>                                       // Proporciona las funciones y macros del modulo LOG

#include <zephyr/bluetooth/bluetooth.h>                               // Proporciona las APIs principales para administrar BLE  
#include <zephyr/bluetooth/uuid.h>                                    // Permite administrar y manipular los UUID // FRAN: Creo que esto es TEMPORAL 
#include <zephyr/bluetooth/addr.h>                                    // Permite administrar las direcciones BLE

#ifdef BLE_CONF_ROLE_CENTRAL
#include <bluetooth/scan.h>                                           // Proporciona las APIs para administrar el escaneo BLE como central
#endif//BLE_CONF_ROLE_CENTRAL

#ifdef BLE_CONF_ROLE_PERIPHERAL
#include <zephyr/bluetooth/gap.h>                                     // Proporciona las APIs y estructuras relacionadas con GAP, como publicidad y conexiones  
#endif//BLE_CONF_ROLE_PERIPHERAL


// CONFIGURATION ------------------------------------------------------------------------------------------------------------------------------------------------------

#define BLE_CONF_LOG_LEVEL 4                                                    // Nilvel del LOG 

#define BT_UUID_NUS_VAL 		BT_UUID_128_ENCODE(0x6e400001, 0xb5a3, 0xf393, 0xe0a9, 0xe50e24dcca9e)     // FRAN: TEMPORAL
#define BT_UUID_NUS_SERVICE   BT_UUID_DECLARE_128(BT_UUID_NUS_VAL)                                       // FRAN: TEMPORAL

// ADVERTISING --------------------------------------------------
#ifdef BLE_CONF_ROLE_PERIPHERAL

#define BLE_CONF_ADV_OPTIONS           BT_LE_ADV_OPT_CONNECTABLE                // Configuracion de la publicidad
#define BLE_CONF_ADV_MIN_INTERVAL      BT_GAP_ADV_FAST_INT_MIN_2                // Configuracion del minimo intervalo de publicidad
#define BLE_CONF_ADV_MAX_INTERVAL      BT_GAP_ADV_FAST_INT_MAX_2                // Configuracion del maximo intervalo de publicidad
#define BLE_CONF_ADV_ADDR_DIREC        NULL                                     // Configuracion la publicidad dirigida (NULL para desactivar)
#define BLE_CONF_ADV_FLAGS             BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR     // Configuracion de las banderas de la publicidad
#define BLE_CONF_DEVICE_NAME           CONFIG_BT_DEVICE_NAME                    // Configuracion del nombre del dispositivo en la publicidad
#define BLE_CONF_ADV_UUID_SERVICE      BT_UUID_NUS_VAL                          // Configuracion del UUID para notificar el servicio utilizado

#endif//BLE_CONF_ROLE_PERIPHERAL

// SCAN --------------------------------------------------
#ifdef BLE_CONF_ROLE_CENTRAL

#define BLE_CONF_SCAN_OPTIONS           BT_LE_SCAN_OPT_FILTER_DUPLICATE         // Configuracion del escaneo
#define BLE_CONF_SCAN_INTERVAL          0x0060                                  // Configuacion del intervalo de escaneo
#define BLE_CONF_SCAN_WINDOWS           0x0050                                  // Configuracion de la ventana de escaneo
#define BLE_CONF_SCAN_UUID_FILTER       BT_UUID_NUS_SERVICE                     // Configuracion del UUID utilizado en el filtro de escaneo

#endif//BLE_CONF_ROLE_CENTRAL


// MACROS - DEFINES - ENUM ------------------------------------------------------------------------------------------------------------------------------------------------------

#define GET_BT_ADDR_STR(origin, addr)                                      \
          char addr[BT_ADDR_LE_STR_LEN];                                   \
          bt_addr_le_to_str(origin, addr, sizeof(addr));

#define IF_BLE_ERROR(err, text, action1, action2, action3)                 \
          if (err) {                                                       \
               LOG_ERR(text, err);                                         \
               action1;                                                    \
               action2;                                                    \
               action3;                                                    \
          }

enum BLE_manager_state
{
     BLE_INIT,
     BLE_WAITING_CONNECTION
};


// FUNCTIONS DECLARATION ------------------------------------------------------------------------------------------------------------------------------------------------------

void BLE_manager();


// ------------------------------------------------------------------------------------------------------------------------------------------------------

#endif//BLE_MANAGER_H
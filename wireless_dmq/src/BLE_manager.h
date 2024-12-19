#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

/*

*/

// ROLE CONFIGURATION ------------------------------------------------------------------------------------------------------------------------------------------------------

#define ROLE_CENTRAL
#define ROLE_PERIPHERAL

// INCLUDES ------------------------------------------------------------------------------------------------------------------------------------------------------

#include <stdint.h>
#include <zephyr/logging/log.h>                                       // Incluye las funciones y macros del modulo LOG

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>                                    // FRAN BORRAR

#ifdef ROLE_CENTRAL

#endif

#ifdef ROLE_PERIPHERAL
#include <zephyr/bluetooth/gap.h>
#endif

// CONFIGURATION ------------------------------------------------------------------------------------------------------------------------------------------------------

#define LOG_BLE_LEVEL 3

// TEMPORAL
#define BT_UUID_NUS_VAL 		BT_UUID_128_ENCODE(0x6e400001, 0xb5a3, 0xf393, 0xe0a9, 0xe50e24dcca9e)	// UUID del servicio NUS
#define BT_UUID_NUS_SERVICE   BT_UUID_DECLARE_128(BT_UUID_NUS_VAL)

// ADVERTISING ------------------------------
#ifdef ROLE_PERIPHERAL
#define ADV_OPTIONS           BT_LE_ADV_OPT_CONNECTABLE      // Configuramos la publicidad como conectable
#define ADV_MIN_INTERVAL      BT_GAP_ADV_FAST_INT_MIN_2      // Configuramos el minimo intervalo de publicidad
#define ADV_MAX_INTERVAL      BT_GAP_ADV_FAST_INT_MAX_2      // Configuramos el maximo intervalo de publicidad
#define ADV_ADDR_DIREC        NULL                            // address of peer for directed advertising
#define ADV_FLAGS             BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR      // Publicidad conectable disponible durante un período de tiempo prolongado y no compatible con bluetooth clasico
#define DEVICE_NAME           CONFIG_BT_DEVICE_NAME  
#define ADV_UUID_SERVICE      BT_UUID_NUS_VAL
#endif

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#endif//BLE_MANAGER_H
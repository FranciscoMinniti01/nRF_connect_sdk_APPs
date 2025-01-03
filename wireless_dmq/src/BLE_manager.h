#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

// ROLE CONFIGURATION ------------------------------------------------------------------------------------------------------------------------------------------------------

#define BLE_CONF_ROLE_CENTRAL                                         // Habilita el funcionamiento del dispositivo como central
#define BLE_CONF_ROLE_PERIPHERAL                                      // Habilita el funcionamiento del dispositivo como periferico


// INCLUDES ------------------------------------------------------------------------------------------------------------------------------------------------------

#include <stdint.h>                                                   // Define los tipos de enteros de tamaño fijo uint8_t, uint16_t, ...    
#include <zephyr/logging/log.h>                                       // Proporciona las funciones y macros del modulo LOG

#include <zephyr/bluetooth/bluetooth.h>                               // Proporciona las APIs principales para administrar BLE
#include <zephyr/bluetooth/uuid.h>                                    // Permite manipular los UUID // FRAN: Creo que esto es TEMPORAL 
#include <zephyr/bluetooth/addr.h>                                    // Permite manipular las direcciones BLE
#include <zephyr/bluetooth/conn.h>                                    // Proporciona las APIs para administrar las conecciones
#include <zephyr/bluetooth/gatt.h>                                    // Proporciona las APIs para administrar servicios y características GATT y sus parametros

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


// CONNECTION --------------------------------------------------
#define SECURITY_LEVEL        BT_SECURITY_L0


// MACROS - STRUCTURES - ENUM ------------------------------------------------------------------------------------------------------------------------------------------------------

/*
 * @brief Convierte una dirección Bluetooth a una representación de cadena legible.
 *
 * Esta macro toma una dirección Bluetooth (estructura `bt_addr_le_t`) y la convierte en un string.
 * La cadena generada tiene el formato estándar Bluetooth, como `XX:XX:XX:XX:XX:XX (type)`
 * y se almacena en un buffer local definido automaticamente.
 *
 * @param origin Dirección Bluetooth de tipo `bt_addr_le_t` que será convertida.
 * @param addr Nombre del buffer donde se almacenará la cadena resultante. La macro lo define internamente.
 */
#define GET_BT_ADDR_STR(origin, addr)                                 \
     char addr[BT_ADDR_LE_STR_LEN];                                   \
     bt_addr_le_to_str(origin, addr, sizeof(addr));

/*
 * Maneja errores BLE con mensajes de log y acciones específicas.
 *
 * Esta macro verifica si se produjo un error (err ≠ 0). Si ocurre un error:
 * - Registra un mensaje de error en el log especificado por `text`.
 * - Ejecuta una o más acciones definidas en `__VA_ARGS__`.
 *
 * @param err Código de error a evaluar.
 * @param text Mensaje de error que se registrará en el log (debe incluir un marcador %d para mostrar el valor de `err`).
 * @param ... Acciones opcionales a ejecutar en caso de error. Puede ser una única acción (por ejemplo, `return err`) 
 *            o varias separadas por punto y coma (`action1; action2`).
 */
#define IF_BLE_ERROR(err, text, ...)                                  \
     do {                                                             \
          if (err) {                                                  \
               LOG_ERR(text, err);                                    \
               do{__VA_ARGS__;} while (0);                            \
          }                                                           \
     } while (0);

enum BLE_manager_state
{
     BLE_INIT,
     BLE_WAITING_CONNECTION
};


// FUNCTIONS DECLARATION ------------------------------------------------------------------------------------------------------------------------------------------------------

void BLE_manager();


// ------------------------------------------------------------------------------------------------------------------------------------------------------

#endif//BLE_MANAGER_H
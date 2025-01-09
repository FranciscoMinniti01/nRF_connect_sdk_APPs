#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

// BUILT CONFIGURATION ------------------------------------------------------------------------------------------------------------------------------------------------------

//#define BLE_CONF_ROLE_CENTRAL                                         // Habilita el funcionamiento del dispositivo como central
#define BLE_CONF_ROLE_PERIPHERAL                                      // Habilita el funcionamiento del dispositivo como periferico

#define BLE_CONF_ENABLE_CONN_PARAM                                    // Habilita la configuracion de los parametros de coneccion por parte del periferico


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

#define BLE_CONF_LOG_LEVEL    4                                                 // Nivel del LOG

#define BT_UUID_NUS_VAL 		BT_UUID_128_ENCODE(0x6e400001, 0xb5a3, 0xf393, 0xe0a9, 0xe50e24dcca9e)     // FRAN: TEMPORAL
#define BT_UUID_NUS_SERVICE   BT_UUID_DECLARE_128(BT_UUID_NUS_VAL)                                       // FRAN: TEMPORAL

// ADVERTISING --------------------------------------------------
#ifdef BLE_CONF_ROLE_PERIPHERAL

#define BLE_CONF_ADV_OPTIONS           BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_ONE_TIME      //Configuracion de la publicidad
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
#define BLE_CONF_SCAN_CONN_AT_MATCH     true                                    // Determina si inicia una coneccion al encontrar un dispositivo o no
#define BLE_CONF_SCAN_ACTIVE_FILTER     BT_SCAN_ALL_FILTER                      // Configuracion para activar ciertos filtros o todos a la vez
#define BLE_CONF_SCAN_TYPE_MATCH        true                                    // Determina si tienen que coinicidir todos los filtros activos o si basta con uno

#endif//BLE_CONF_ROLE_CENTRAL


// CONNECTION --------------------------------------------------
#define BLE_CONF_SECURITY_LEVEL         BT_SECURITY_L3                          // Determina el nivel de seguridad que se va a solicitar como central a un periferico que se conecte

#ifdef BLE_CONF_ENABLE_CONN_PARAM
#define BLE_CONF_CONN_MIN_INTERVAL      30                                      // Configuracion de intervalo minimo de coneccion que como periferico se solicita a una central
#define BLE_CONF_CONN_MAX_INTERVAL      50                                      // Configuracion de intervalo maximo de coneccion que como periferico se solicita a una central
#define BLE_CONF_CONN_LATENCY           5                                       // Configuracion de la latencia de coneccion que como periferico se solicita a una central
#define BLE_CONF_CONN_TIMEOUT           1000                                    // Configuracion del timeout de coneccion que como periferico se solicita a una central (Tiene un valor minimo)
#define CONN_MIN_TIMEOUT                (((1+BLE_CONF_CONN_LATENCY)*BLE_CONF_CONN_MAX_INTERVAL)/4)+1       // Valor minimo que puede tomar el timeout de una coneccion
#if BLE_CONF_CONN_TIMEOUT < CONN_MIN_TIMEOUT
#define FINAL_TIMEOUT CONN_MIN_TIMEOUT
#else
#define FINAL_TIMEOUT BLE_CONF_CONN_TIMEOUT
#endif
#endif//BLE_CONF_ENABLE_CONN_PARAM

// SECURITY --------------------------------------------------
#define BLE_CONF_PASSKEY      123456


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
#define IF_BLE_ERROR(err, text, ...)                             \
     if (err) {                                                  \
          LOG_ERR(text, err);                                    \
          do{__VA_ARGS__;} while (0);                            \
     }

/*
 * Maneja acciones exitosas BLE con mensajes de log y acciones específicas.
 *
 * Esta macro verifica si no se produjo un error (err = 0). Si no ocurre un error:
 * - Registra un mensaje de informacion en el log, especificado por `text`.
 * - Ejecuta una o más acciones definidas en `__VA_ARGS__`.
 *
 * @param err Código de error a evaluar.
 * @param text Mensaje que se registrará en el log.
 * @param ... Acciones opcionales a ejecutar en caso de error. Puede ser una única acción (por ejemplo, `return err`) 
 *            o varias separadas por punto y coma (`action1; action2`).
 */
#define IF_BLE_NO_ERROR(err, text, ...)                          \
     if (!err){                                                  \
          LOG_INF(text);                                         \
          do{__VA_ARGS__;} while (0);                            \
     }

#define IF_BLE_IS_ERROR(err, errcomp, text, ...)                 \
     if (err = errcomp) {                                        \
          LOG_INF(text);                                         \
          do{__VA_ARGS__;} while (0);                            \
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
// INCLUDES ------------------------------------------------------------------------------------------------------------------------------------------------------

#include "BLE_manager.h"

// VARIABLES ------------------------------------------------------------------------------------------------------------------------------------------------------

LOG_MODULE_REGISTER(LOG_BLE,LOG_BLE_LEVEL);			               // Registro del modulo LOG y configuracion del nivel

// TEMPORAL
uint16_t adv_sensor_id = 6666;

// ADVERTISING ------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef ROLE_PERIPHERAL

static const struct bt_data ad[] = {                                            // Estructura del Advertising package con la informacion que contiene
     BT_DATA_BYTES( BT_DATA_FLAGS, ADV_FLAGS),                                  // Configuracion de las banderas del paquete
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, sizeof(DEVICE_NAME) - 1),      // Incluye en el paquete el nombre del dispositivo que vera el usuario
};

static const struct bt_data sd[] = {                                            // Estructura del scan packet con la informacion que contiene
     BT_DATA_BYTES(BT_DATA_UUID128_ALL, ADV_UUID_SERVICE),                      // Incluye el UUID del servicio NUS
     BT_DATA(BT_DATA_MANUFACTURER_DATA, (unsigned char *)&adv_sensor_id, sizeof(adv_sensor_id))
};

void advertising_start()
{
     int err;
	struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM( ADV_OPTIONS,          // Configuracion de la publicidad
                                                          ADV_MIN_INTERVAL,     // Configuracion del minimo intervalo de publicidad
                                                          ADV_MAX_INTERVAL,     // Configuracion del maximo intervalo de publicidad
                                                          ADV_ADDR_DIREC);      // Configuracion de la direcion BLE para publicidad dirigida

	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));	// Inicio de la publicidad
	if (err) {
		if (err == -EALREADY) LOG_INF("Advertising continued\n");             // FRAN Esto nose si es necesario, lo dejo para ver cuando pasa, Tengo que probar con BT_LE_ADV_OPT_ONE_TIME en eñ adv_param.
		else LOG_ERR("Advertising failed to start. Error: %d\n", err);
		return;
	}
	LOG_INF("Advertising successfully started\n");
}

#endif

// ------------------------------------------------------------------------------------------------------------------------------------------------------
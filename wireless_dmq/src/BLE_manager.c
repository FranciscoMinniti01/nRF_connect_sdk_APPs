// INCLUDES ------------------------------------------------------------------------------------------------------------------------------------------------------

#include "BLE_manager.h"

// VARIABLES ------------------------------------------------------------------------------------------------------------------------------------------------------

LOG_MODULE_REGISTER(LOG_BLE,LOG_BLE_LEVEL);			               // Registro del modulo LOG y configuracion del nivel
static uint8_t manager_state = BLE_INIT;

// TEMPORAL
const uint8_t adv_sensor_id[3] = {0x12,0x34,0x56};
#ifdef ROLE_CENTRAL
struct bt_scan_manufacturer_data manufacturer_data; 
#endif

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

// SCANING ------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef ROLE_CENTRAL

static void scan_filter_match( struct bt_scan_device_info *device_info,
			                struct bt_scan_filter_match *filter_match,
			                bool connectable )
{
     GET_BT_ADDR_STR(device_info->recv_info->addr, addr);
	LOG_INF("Scan filters matched. Address: %s - Connectable: %d", addr, connectable);
}

static void scan_connecting_error(struct bt_scan_device_info *device_info)
{
	GET_BT_ADDR_STR(device_info->recv_info->addr, addr);
	LOG_WRN("Scan connecting failed. Address: %s", addr);
}

static void scan_connecting(struct bt_scan_device_info *device_info, struct bt_conn *conn)
{
	//default_conn = bt_conn_ref(conn);
	GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);
	LOG_INF("Scan successful, connection is started. Address: %s", addr);
}

struct cb_data scan_cb_data = {
	.filter_match 		= scan_filter_match,
	.connecting_error 	= scan_connecting_error,
	.connecting 		= scan_connecting,
};
static struct bt_scan_cb scan_cb = {
	.cb_addr = &scan_cb_data,	
};

static int scan_init()
{
	int err;

	struct bt_le_scan_param scan_param_init = {
		.options 	= BT_LE_SCAN_OPT_FILTER_DUPLICATE,								// Evita el procesamiento repetido de paquetes de publicidad provenientes de un mismo periferico. 			
		.interval = 0x0060,
		.window 	= 0x0050,
		.timeout 	= 0,
	};

	struct bt_scan_init_param scan_init = {
		.scan_param         = &scan_param_init,
          .connect_if_match   = 0,													// Configuracion que perimite iniciar una coneccion con los dispositivos que cumplen con el filtro
	};

	bt_scan_init(&scan_init);														// Inicializamos el modulo scan pero no el scanning
	
	bt_scan_cb_register(&scan_cb);													// Registramos las callbacks de scan

	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_UUID, BT_UUID_NUS_SERVICE);		// Creamos un filtro para el UUID del servicio NUS
	if (err) {
		LOG_ERR("Scanning filters UUID cannot be set (err %d)", err);
		return err;
	}

     manufacturer_data.data = adv_sensor_id;
     manufacturer_data.data_len = sizeof(adv_sensor_id);
     LOG_DBG("manufacturer_data_len = %d", manufacturer_data.data_len);
	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_MANUFACTURER_DATA, (void*)&manufacturer_data );		// Creamos un filtro para el 
	if (err) {
		LOG_ERR("Scanning filters DATA cannot be set (err %d)", err);
		return err;
	}

	err = bt_scan_filter_enable(BT_SCAN_ALL_FILTER, true);							// Habilitamos los filtros, BT_SCAN_ALL_FILTER: indicamos que habilitamos todos los filtros. true: Indicamos que deben coincidir todos a la vez 
	if (err) {
		LOG_ERR("Filters cannot be turned on (err %d)", err);
		return err;
	}

	LOG_INF("Scan module initialized");
	return err;
}

#endif

// BLE MACHINE ------------------------------------------------------------------------------------------------------------------------------------------------------

void BLE_manager()
{
     uint8_t err;
     switch (manager_state)
     {
          case BLE_INIT:
               LOG_DBG("BLE manager state init");

               err = bt_enable(NULL);
               if (err) LOG_ERR("Bluetooth init failed (err %d)", err);
               
               #ifdef ROLE_CENTRAL
               err = scan_init();
               if (err) LOG_ERR("scan_init failed (err %d)", err);

               err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
               if (err) LOG_ERR("Scanning failed to start (err %d)", err);
               #endif

               #ifdef ROLE_PERIPHERAL
               advertising_start();
               #endif

               manager_state = BLE_WAITING_CONNECTION;

               break;
          
          case BLE_WAITING_CONNECTION:
               break;

          default:
               break;
     }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------
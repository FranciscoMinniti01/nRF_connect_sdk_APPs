// INCLUDES ------------------------------------------------------------------------------------------------------------------------------------------------------

#include "BLE_manager.h"


// VARIABLES ------------------------------------------------------------------------------------------------------------------------------------------------------

LOG_MODULE_REGISTER(LOG_BLE,BLE_CONF_LOG_LEVEL);                                // Registro del modulo LOG y configuracion del nivel
static uint8_t manager_state = BLE_INIT;                                        // Estado del administrador del BLE

// FRAN: TEMPORAL
const uint8_t adv_sensor_id[3] = {0x12,0x34,0x56};


// ADVERTISING ------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef BLE_CONF_ROLE_PERIPHERAL

static const struct bt_data ad[] = {                                                           // Estructura del paquete ADV
     BT_DATA_BYTES( BT_DATA_FLAGS, BLE_CONF_ADV_FLAGS),
	BT_DATA(BT_DATA_NAME_COMPLETE, BLE_CONF_DEVICE_NAME, sizeof(BLE_CONF_DEVICE_NAME) - 1),
};

static const struct bt_data sd[] = {                                                           // Estructura del paquete de respusta de escaneop
     BT_DATA_BYTES(BT_DATA_UUID128_ALL, BLE_CONF_ADV_UUID_SERVICE),
     BT_DATA( BT_DATA_MANUFACTURER_DATA, (unsigned char *)&adv_sensor_id, sizeof(adv_sensor_id)),
};

void advertising_start()
{
     LOG_DBG("advertising_start()");

     int err;
	struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM( BLE_CONF_ADV_OPTIONS,
                                                          BLE_CONF_ADV_MIN_INTERVAL,
                                                          BLE_CONF_ADV_MAX_INTERVAL,
                                                          BLE_CONF_ADV_ADDR_DIREC);

	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));	               // Inicio de la publicidad
	if (err) {
		if (err == -EALREADY) LOG_ERR("ADV CONTINUA FRAN !!\n");                             // FRAN Esto nose si es necesario, lo dejo para ver cuando pasa, Tengo que probar con BT_LE_ADV_OPT_ONE_TIME en el adv_param.
		else LOG_ERR("Advertising failed start. Error: %d\n", err);
		return;
	}

	LOG_INF("Advertising successfully started\n");
}

#endif


// SCANING ------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef BLE_CONF_ROLE_CENTRAL

static void scan_filter_match( struct bt_scan_device_info *device_info, struct bt_scan_filter_match *filter_match, bool connectable )
{
     GET_BT_ADDR_STR(device_info->recv_info->addr, addr);
	LOG_INF("Scan filters matched. Address: %s - Connectable: %s", addr, (connectable? "SI":"NO") );
}

static void scan_connecting_error(struct bt_scan_device_info *device_info)
{
	GET_BT_ADDR_STR(device_info->recv_info->addr, addr);
	LOG_WRN("Scan connecting failed. Address: %s", addr);
}

static void scan_connecting(struct bt_scan_device_info *device_info, struct bt_conn *conn)
{
	//default_conn = bt_conn_ref(conn); // FRAN esto si va en la vercion completa
	GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);
	LOG_INF("Scan successful, connection is started. Address: %s", addr);
}

BT_SCAN_CB_INIT(scan_cb, scan_filter_match,
                         NULL,
                         scan_connecting_error,
                         scan_connecting);

static int scan_init()
{
	int err;

     LOG_DBG("scan_init()");

	struct bt_le_scan_param scan_param_init = {                                     // Estructura de configuracion del escaneo
		.options 	= BLE_CONF_SCAN_OPTIONS, 			
		.interval = BLE_CONF_SCAN_INTERVAL,
		.window 	= BLE_CONF_SCAN_WINDOWS,
		.timeout 	= 0,                                                             // Configuracion del timeout (Cero para desactivarlo)
	};

	struct bt_scan_init_param scan_init = {
		.scan_param         = &scan_param_init,
          .connect_if_match   = 0,											// Configuracion que perimite iniciar una coneccion con los dispositivos que cumplen con el filtro
	};

	bt_scan_init(&scan_init);											// Inicializamos el modulo scan (No inicia el escaneo)
	
	bt_scan_cb_register(&scan_cb);										// Registramos las callbacks de scan_cb

	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_UUID, BLE_CONF_SCAN_UUID_FILTER);	// Creacion del filtro para el UUID del servicio NUS
	if (err) {
		LOG_ERR("Scanning filters UUID cannot be set (err %d)", err);
		return err;
	}

     struct bt_scan_manufacturer_data manufacturer_data;                             // Estructura para almacenar la informacion para el filtro
     manufacturer_data.data = adv_sensor_id;                                         
     manufacturer_data.data_len = sizeof(adv_sensor_id);
	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_MANUFACTURER_DATA, (void*)&manufacturer_data ); // Creacion del filtro para manufacture data
	if (err) {
		LOG_ERR("Scanning filters DATA cannot be set (err %d)", err);
		return err;
	}

	err = bt_scan_filter_enable(BT_SCAN_ALL_FILTER, true);						// Habilitamos los filtros, BT_SCAN_ALL_FILTER: indicamos que habilitamos todos los filtros. true: Indicamos que deben coincidir todos a la vez 
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
               IF_BLE_ERROR(err,"Bluetooth init failed (err %d)",NULL,NULL,NULL);
               
               #ifdef BLE_CONF_ROLE_CENTRAL
               err = scan_init();
               if (err) LOG_ERR("scan_init failed (err %d)", err);

               err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
               if (err) LOG_ERR("Scanning failed to start (err %d)", err);
               #endif

               #ifdef BLE_CONF_ROLE_PERIPHERAL
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
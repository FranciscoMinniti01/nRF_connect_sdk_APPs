// INCLUDES ------------------------------------------------------------------------------------------------------------------------------------------------------

#include "BLE_manager.h"


// VARIABLES ------------------------------------------------------------------------------------------------------------------------------------------------------

LOG_MODULE_REGISTER(LOG_BLE,BLE_CONF_LOG_LEVEL);                                // Registro del modulo LOG y configuracion del nivel
static uint8_t manager_state  = BLE_INIT;                                       // Estado del administrador del BLE
static bool flag_ble_error    = false;                                          // Bandera global para indicar la precencia de error, en true se reinicia BLE

#ifdef BLE_CONF_ROLE_CENTRAL
static struct bt_conn *peripheral_conn;                                         // Instancia de la conneccion como central a un periferico
#endif//BLE_CONF_ROLE_CENTRAL

#ifdef BLE_CONF_ROLE_PERIPHERAL
static struct bt_conn *central_conn;                                            // Instancia de la conneccion como periferico a un central 
#endif//BLE_CONF_ROLE_PERIPHERAL

// FRAN: TEMPORAL
static uint8_t adv_sensor_id[3] = {0x12,0x34,0x56};                              // FRAN: TEMPORAL


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

static int advertising_start()
{
     int err;

     LOG_DBG("advertising_start()");

	struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM( BLE_CONF_ADV_OPTIONS,
                                                          BLE_CONF_ADV_MIN_INTERVAL,
                                                          BLE_CONF_ADV_MAX_INTERVAL,
                                                          BLE_CONF_ADV_ADDR_DIREC);

	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));	               // Inicio de la publicidad
	if (err == -EALREADY) LOG_WRN("Advertising continuous");
	IF_BLE_ERROR(err, "Advertising failed start. Error: %d", return err );

	LOG_INF("Advertising successfully started");

     return 0;
}

#endif//BLE_CONF_ROLE_PERIPHERAL


// SCANING ------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef BLE_CONF_ROLE_CENTRAL

static void scan_filter_match( struct bt_scan_device_info *device_info, struct bt_scan_filter_match *filter_match, bool connectable )
{
     GET_BT_ADDR_STR(device_info->recv_info->addr, addr);
	LOG_INF("Scan filters matched. Address: %s - Connectable: %s", addr, (connectable? "YES":"NO") );
}

static void scan_connecting_error(struct bt_scan_device_info *device_info)
{
	GET_BT_ADDR_STR(device_info->recv_info->addr, addr);
	LOG_WRN("Scan connecting failed. Address: %s", addr);
}

static void scan_connecting(struct bt_scan_device_info *device_info, struct bt_conn *conn)
{
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
		.timeout 	= 0,                                                             // Configuracion del timeout, lo dejo en cero para desactivarlo ya que no tengo acceso a la callback timeout
	};

	struct bt_scan_init_param scan_init = {
		.scan_param         = &scan_param_init,
          .connect_if_match   = BLE_CONF_SCAN_CONN_AT_MATCH,					// Configuracion que perimite iniciar una coneccion con los dispositivos que cumplen con el filtro
	};

	bt_scan_init(&scan_init);											// Inicializamos el modulo scan (No inicia el escaneo)
	
	bt_scan_cb_register(&scan_cb);										// Registramos las callbacks de scan_cb

	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_UUID, BLE_CONF_SCAN_UUID_FILTER);	// Creacion del filtro para el UUID del servicio NUS
	IF_BLE_ERROR(err, "Scanning filters UUID cannot be set (err %d)", return err);

     static struct bt_scan_manufacturer_data manufacturer_data;                      // Estructura para almacenar la informacion para el filtro de manufacture data
     manufacturer_data.data = adv_sensor_id;                                         
     manufacturer_data.data_len = sizeof(adv_sensor_id);
	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_MANUFACTURER_DATA, (void*)&manufacturer_data );   // Creacion del filtro para manufacture data
	IF_BLE_ERROR(err, "Scanning filters DATA cannot be set (err %d)", return err);

	err = bt_scan_filter_enable(BLE_CONF_SCAN_ACTIVE_FILTER, BLE_CONF_SCAN_TYPE_MATCH);			// Habilitamos los filtros
	IF_BLE_ERROR(err, "Filters cannot be turned on (err %d)", return err);

	LOG_INF("Scan module initialized");

	return 0;
}

#endif//BLE_CONF_ROLE_CENTRAL


// CONNECTION ------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef BLE_CONF_ROLE_CENTRAL
static void mtu_exchange_cb(struct bt_conn *conn, uint8_t err, struct bt_gatt_exchange_params *params)
{
	if (!err) LOG_INF("MTU exchange done");
	else LOG_WRN("MTU exchange failed (Error: %" PRIu8 ")", err);
}
#endif//BLE_CONF_ROLE_CENTRAL

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
     int err;
     struct bt_conn_info info;

     GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);

     LOG_DBG("connected()");

	if(conn_err)
	{
		LOG_ERR("Failed to connect to %s (Error: %d)", addr, conn_err);
          
          #ifdef BLE_CONF_ROLE_CENTRAL
          if(peripheral_conn == NULL)
          {
               err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
               IF_BLE_ERROR(err, "Scanning failed to start (Error: %d)", flag_ble_error=true)
               if(!err) LOG_INF("Scanning start");
          }
          #endif//BLE_CONF_ROLE_CENTRAL
          
          #ifdef BLE_CONF_ROLE_PERIPHERAL
          if(central_conn == NULL)
          {
               err = advertising_start();
               IF_BLE_ERROR(err, "Advertising failed to start (err %d)", flag_ble_error=true)
          }
          #endif//BLE_CONF_ROLE_PERIPHERAL
		
          return;
	}
	
     err = bt_conn_get_info(conn, &info);
     IF_BLE_ERROR(err, "get conn info failed (Error: %d)")
     
     #ifdef BLE_CONF_ROLE_CENTRAL
     if (info.role == BT_CONN_ROLE_CENTRAL)
     {
          LOG_INF("Connected to the peripheral %s", addr);
          peripheral_conn = bt_conn_ref(conn);

          static struct bt_gatt_exchange_params exchange_params;                     //  Estructura para la negociacion del tamaño del MTU
          exchange_params.func = mtu_exchange_cb;                                    //  Callback para notificar la finalizacion de la notificacion de la MTU
          err = bt_gatt_exchange_mtu(conn, &exchange_params);                        //  Llama a la negociacion del tamañano del MTU
          IF_BLE_ERROR(err, "MTU exchange failed (Error: %d)")                       //  La MTU se configura en el prj.conf creo // FRAN          

          //err = bt_conn_set_security(conn, BLE_CONF_SECURITY_LEVEL);
          //IF_BLE_ERROR(err, "Failed to set security level (Error: %d)")//, gatt_discover(conn);Client_BLE_State = CLIENT_BLE_CONNECTED ) //FRAN

          err = bt_scan_stop();
          if(err == -EALREADY) LOG_WRN("Scanning was already stopped");
          else if(err) LOG_ERR("Stop LE scan failed (Error %d)", err);
     }
     #endif//BLE_CONF_ROLE_CENTRAL

     #ifdef BLE_CONF_ROLE_PERIPHERAL
     if (info.role == BT_CONN_ROLE_PERIPHERAL)
     {
          LOG_INF("Connected to the central %s", addr);
          central_conn = bt_conn_ref(conn);

          #ifdef BLE_CONF_ENABLE_CONN_PARAM
          static struct bt_le_conn_param conn_param = {
               .interval_min  = BLE_CONF_CONN_MIN_INTERVAL/1.25,
               .interval_max  = BLE_CONF_CONN_MAX_INTERVAL/1.25,
               .latency       = BLE_CONF_CONN_LATENCY,
               .timeout       = FINAL_TIMEOUT/10,
          };
          bt_conn_le_param_update(conn,&conn_param);
          #endif//BLE_CONF_ENABLE_CONN_PARAM
     }
     #endif//BLE_CONF_ROLE_PERIPHERAL

     LOG_DBG("Connection parameters:         interval %.2f ms - latency %d - timeout %d ms", info.le.interval*1.25, info.le.latency, info.le.timeout*10);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
     int err;
     struct bt_conn_info info;

	GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);

     err = bt_conn_get_info(conn, &info);
     IF_BLE_ERROR(err, "get conn info failed (Error: %d)")

     #ifdef BLE_CONF_ROLE_CENTRAL
     if (info.role == BT_CONN_ROLE_CENTRAL)
     {
          LOG_INF("Disconnected to the peripheral %s (Reason %u)", addr, reason);
          if(peripheral_conn == conn)
          {
               bt_conn_unref(conn);
               peripheral_conn = NULL;
          }
          else
          { 
               LOG_ERR("Disconnected from an unassigned peripheral");
               flag_ble_error = true;
          }
          err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
          IF_BLE_ERROR(err, "Scanning failed to start (Error: %d)", flag_ble_error=true)
          if(!err) LOG_INF("Scanning start");
     }
     #endif//BLE_CONF_ROLE_CENTRAL

     #ifdef BLE_CONF_ROLE_PERIPHERAL
     if (info.role == BT_CONN_ROLE_PERIPHERAL)
     {
          LOG_INF("Disconnected to the central %s (Reason %u)", addr, reason);
          if(central_conn == conn)
          {
               bt_conn_unref(conn);
               central_conn = NULL;
          }
          else
          {
               LOG_ERR("Disconnected from an unassigned central");
               flag_ble_error = true;
          }
          err = advertising_start();
          IF_BLE_ERROR(err, "Advertising failed to start (err %d)", flag_ble_error=true)
     }
     #endif//BLE_CONF_ROLE_PERIPHERAL
}

static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{
     LOG_INF("Connection parameters updated: interval %.2f ms - latency %d - timeout %d ms", interval*1.25, latency, timeout*10);
}

static bool le_param_request(struct bt_conn *conn, struct bt_le_conn_param *param)
{
     LOG_INF( "Connection parameters request: interval min %.2f ms - interval max %.2f ms - latency %d - timeout %d ms", 
              (param->interval_min)*1.25, (param->interval_max)*1.25, param->latency, (param->timeout)*10);
     return true;
}

/*static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (!err) {
		LOG_INF("Security changed: %s level %u", addr, level);
	} else {
		LOG_WRN("Security failed: %s level %u err %d", addr,
			level, err);
	}

	gatt_discover(conn);
	Client_BLE_State = CLIENT_BLE_CONNECTED;
}*/

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected          = connected,
	.disconnected       = disconnected,
     .le_param_updated   = le_param_updated,
     .le_param_req       = le_param_request,
	//.security_changed = security_changed
};


// BLE MACHINE ------------------------------------------------------------------------------------------------------------------------------------------------------

void BLE_manager()
{
     uint8_t err;
     switch (manager_state)
     {
          case BLE_INIT:
               LOG_DBG("BLE manager state init");
               
               err = bt_enable(NULL);
               IF_BLE_ERROR(err, "Bluetooth init failed (err %d)", flag_ble_error=true; return);

               #ifdef BLE_CONF_ROLE_CENTRAL
               err = scan_init();
               IF_BLE_ERROR(err, "scan_init failed (err %d)", flag_ble_error=true)

               err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
               IF_BLE_ERROR(err, "Scanning failed to start (err %d)", flag_ble_error=true)
               if(!err) LOG_INF("Scanning start");
               #endif//BLE_CONF_ROLE_CENTRAL

               #ifdef BLE_CONF_ROLE_PERIPHERAL
               err = advertising_start();
               IF_BLE_ERROR(err, "Advertising failed to start (err %d)", flag_ble_error=true)
               #endif//BLE_CONF_ROLE_PERIPHERAL

               flag_ble_error = false;
               manager_state  = BLE_WAITING_CONNECTION;

               break;
          
          case BLE_WAITING_CONNECTION:
               break;

          default:
               break;
     }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------
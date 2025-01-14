#ifdef CONF_BT_CENTRAL_PERIPHERAL

// INCLUDES ---------------------------------------------------------------------------------------------------------------------------------------------

#include "BLE_central_peripheral.h"


// VARIABLES ------------------------------------------------------------------------------------------------------------------------------------------------------

LOG_MODULE_REGISTER(LOG_BLE,BLE_CONF_LOG_LEVEL);                                // Registro del modulo LOG y configuracion del nivel
static uint8_t manager_state  = BLE_INIT;                                       // Estado del administrador del BLE
static bool flag_ble_error    = false;                                          // Bandera global para indicar la precencia de error, en true se reinicia BLE

static struct bt_conn *peripheral_conn;                                         // Instancia de la conneccion como central a un periferico

// FRAN TEMPORAL
static uint8_t adv_sensor_id[3] = {0x12,0x34,0x56};


// SCANING ------------------------------------------------------------------------------------------------------------------------------------------------------

static void scan_filter_match( struct bt_scan_device_info *device_info, struct bt_scan_filter_match *filter_match, bool connectable )
{
     GET_BT_ADDR_STR(device_info->recv_info->addr, addr);
	LOG_INF("Scan filters matched. Address: %s - Connectable: %s", addr, (connectable? "YES":"NO") );
}

static void scan_connecting_error(struct bt_scan_device_info *device_info)
{
     int err;

	GET_BT_ADDR_STR(device_info->recv_info->addr, addr);
	LOG_WRN("Scan connecting failed. Address: %s", addr);

     err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
     IF_BLE_ERROR(err, "Scanning failed to start (err %d)", flag_ble_error=true)
     if(!err) LOG_INF("Scanning start");
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


// CONNECTION ------------------------------------------------------------------------------------------------------------------------------------------------------

/*
static bool le_param_request(struct bt_conn *conn, struct bt_le_conn_param *param)
{
     int err;
     struct bt_conn_info info;
     err = bt_conn_get_info(conn, &info);
     IF_BLE_ERROR(err, "get conn info failed (Error: %d)")
     if (info.role == BT_CONN_ROLE_PERIPHERAL) LOG_INF("Role peripheral, Connection parameters request:");
     if (info.role == BT_CONN_ROLE_CENTRAL) LOG_INF("Role central, Connection parameters request:");

     LOG_INF( "     Interval MIN %.2f ms - Interval MAX %.2f ms - Latency %d - Timeout %d ms", 
              (param->interval_min)*1.25, (param->interval_max)*1.25, param->latency, (param->timeout)*10);
     return true;
}

static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{
     int err;
     struct bt_conn_info info;
     err = bt_conn_get_info(conn, &info);
     IF_BLE_ERROR(err, "get conn info failed (Error: %d)")
     if (info.role == BT_CONN_ROLE_PERIPHERAL) LOG_INF("Role peripheral, Connection parameters updated:");
     if (info.role == BT_CONN_ROLE_CENTRAL) LOG_INF("Role central, Connection parameters updated:");
     LOG_INF("     Interval %.2f ms - Latency %d - Timeout %d ms", interval*1.25, latency, timeout*10);
}

static void on_le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param)
{
     int err;
     struct bt_conn_info info;
     err = bt_conn_get_info(conn, &info);
     IF_BLE_ERROR(err, "get conn info failed (Error: %d)")
     if (info.role == BT_CONN_ROLE_PERIPHERAL) LOG_INF("Role peripheral, PHY updated:");
     if (info.role == BT_CONN_ROLE_CENTRAL) LOG_INF("Role central, PHY updated:");

     if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_1M) {
          LOG_INF("     New PHY: 1M");
     }
     else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_2M) {
          LOG_INF("     New PHY: 2M");
     }
     else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_CODED_S8) {
          LOG_INF("     New PHY: Long Range");
     }
     else LOG_INF("     New PHY don't detected");
}

static void on_le_data_len_updated(struct bt_conn *conn, struct bt_conn_le_data_len_info *info)
{
     int err;
     struct bt_conn_info inf;
     err = bt_conn_get_info(conn, &inf);
     IF_BLE_ERROR(err, "get conn info failed (Error: %d)")
     if (inf.role == BT_CONN_ROLE_PERIPHERAL) LOG_INF("Role peripheral, Data length updated:");
     if (inf.role == BT_CONN_ROLE_CENTRAL) LOG_INF("Role central, Data length updated:");

     LOG_INF("     Length tx:%d bytes - rx:%d bytes, time tx:%d us - rx:%d us", info->tx_max_len, info->rx_max_len, info->tx_max_time, info->rx_max_time);
}
*/

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
     int err;
     struct bt_conn_info info;

     GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);

     err = bt_conn_get_info(conn, &info);
     IF_BLE_ERROR(err, "Get conn info failed (Error: %d)")

	if(conn_err)
	{
		LOG_ERR("Connected failed to %s (Error: %d)", addr, conn_err);
          
          if(!bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE)) flag_ble_error=true;

          bt_conn_unref(peripheral_conn);
          peripheral_conn = NULL;
		
          return;
	}
     
     LOG_INF("Connected to %s", addr);
     peripheral_conn = bt_conn_ref(conn);      

     err = bt_scan_stop();
     if(err == -EALREADY);// LOG_WRN("Scan was already stopped");
     else if(err) LOG_ERR("Stop LE scan failed (Error %d)", err);

     /*err = bt_conn_set_security(conn, BLE_CONF_SECURITY_LEVEL);
     IF_BLE_ERROR(err, "Failed to set security level (Error: %d)")*/

     /*if (info.role == BT_CONN_ROLE_PERIPHERAL) LOG_DBG("Role peripheral:");
     if (info.role == BT_CONN_ROLE_CENTRAL)    LOG_DBG("Role central:");
     LOG_DBG("     Connection parameters:   interval %.2f ms - latency %d - timeout %d ms", info.le.interval*1.25, info.le.latency, info.le.timeout*10);

     LOG_DBG("     PHY RX: %dM - PHY TX: %dM",info.le.phy->rx_phy ,info.le.phy->tx_phy);
     LOG_DBG("     Length tx:%d bytes - rx:%d bytes, time tx:%d us - rx:%d us", info.le.data_len->tx_max_len, info.le.data_len->rx_max_len, info.le.data_len->tx_max_time, info.le.data_len->rx_max_time);
     LOG_DBG("     Security level %d",info.security.level);*/
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
     int err;
     struct bt_conn_info info;

     GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);

     err = bt_conn_get_info(conn, &info);
     IF_BLE_ERROR(err, "Get conn info failed (Error: %d)")

     LOG_INF("Disconnected to %s (Reason %x)", addr, reason);

     if(peripheral_conn != conn) LOG_ERR("Disconnected from an unassigned peripheral");
     bt_conn_unref(peripheral_conn);
     peripheral_conn = NULL;

     if(!bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE)) flag_ble_error=true;
}

static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
	GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);

	if (!err) LOG_INF("Security changed: %s level %u", addr, level);
	else LOG_WRN("Security changed failed: %s level %u err %d", addr, level, err);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected               = connected,
	.disconnected            = disconnected,
	.security_changed        = security_changed,
     //.le_param_req            = le_param_request,
     //.le_param_updated        = le_param_updated,
     //.le_phy_updated          = on_le_phy_updated,
     //.le_data_len_updated     = on_le_data_len_updated,
};


// AUTHENTUCATION AND PAIRING ------------------------------------------------------------------------------------------------------------------------------------------------------

/*
static void pairing_confirm(struct bt_conn *conn)
{
     GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);
	LOG_INF("pairing confirm request: %s", addr);
     
     #ifdef BLE_CONF_ROLE_PERIPHERAL 
     int err;
     if(bt_addr_le_eq(bt_conn_get_dst(conn),bt_conn_get_dst(central_conn)))
     {
          err = bt_conn_auth_pairing_confirm(central_conn);
          IF_BLE_ERROR(err, "pairing confirm failed (Error: %d)",return)
          LOG_INF("pairing confirmation request accepted");
     }
     else LOG_WRN("Pairing confirmation request denied");
     #endif//BLE_CONF_ROLE_PERIPHERAL
}

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);
	printk("Passkey for %s: %06u\n", addr, passkey);
}

static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey)
{
     GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);
	printk("Passkey confirm for %s: %06u\n", addr, passkey);
     //bt_conn_auth_passkey_confirm(conn);
}
*/

static void auth_cancel(struct bt_conn *conn)
{
	GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);
	LOG_WRN("Pairing cancelled: %s", addr);
}

static void passkey_entry(struct bt_conn *conn)
{
     GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);
	LOG_INF("Passkey entry request: %s", addr);
     
     int err = bt_conn_auth_passkey_entry(conn, BLE_CONF_PASSKEY);
     IF_BLE_ERROR(err, "Passkey entry failed (Error: %d)")
}

static struct bt_conn_auth_cb conn_auth_cb = {
	.cancel             = auth_cancel,
     .passkey_entry      = passkey_entry,
     //.passkey_display    = auth_passkey_display,
	//.passkey_confirm    = auth_passkey_confirm,         
     //.pairing_confirm    = pairing_confirm,
};

static void pairing_complete(struct bt_conn *conn, bool bonded)
{
	GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);
	LOG_INF("Pairing completed: %s, bonded: %d", addr, bonded);
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
	GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);
	LOG_WRN("Pairing failed conn: %s, reason %d", addr, reason);
}

static struct bt_conn_auth_info_cb conn_auth_info_cb = {
	.pairing_complete = pairing_complete,
	.pairing_failed = pairing_failed
};


// BLE MACHINE ------------------------------------------------------------------------------------------------------------------------------------------------------

void BLE_manager()
{
     uint8_t err;
     switch (manager_state)
     {
          case BLE_INIT:

               err = bt_conn_auth_cb_register(&conn_auth_cb);
               IF_BLE_ERROR(err, "Auth cb register failed (err %d)", flag_ble_error=true; return)

               err = bt_conn_auth_info_cb_register(&conn_auth_info_cb);
               IF_BLE_ERROR(err, "Auth info cb register failed (err %d)", flag_ble_error=true; return)

               err = bt_enable(NULL);
               IF_BLE_ERROR(err, "Bluetooth init failed (err %d)", flag_ble_error=true; return)

               err = scan_init();
               IF_BLE_ERROR(err, "Scan_init failed (err %d)", flag_ble_error=true)

               err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
               IF_BLE_ERROR(err, "Scanning failed to start (err %d)", flag_ble_error=true)
               if(!err) LOG_INF("Scanning start");

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

#endif//CONF_BT_CENTRAL_PERIPHERAL
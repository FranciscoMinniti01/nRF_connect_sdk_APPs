
#ifdef CONF_BT_PERIPHERAL

// INCLUDES ---------------------------------------------------------------------------------------------------------------------------------------------

#include "BLE_peripheral.h"


// VARIABLES --------------------------------------------------------------------------------------------------------------------------------------------

LOG_MODULE_REGISTER(LOG_BLE_PERIPHERAL,BLE_CONF_LOG_LEVEL);           // Registro del LOG
static uint8_t manager_state  = BLE_INIT;                             // Estado del administrador BLE
static bool    flag_ble_error = false;                                // Bandera para indicar un error en el administrador BLE
static bool    flag_adv_start = false;
static struct  bt_conn *central_conn;                                 // Referencia a la coneccion establecida
static uint8_t manufacturer_data[MAX_LEN_MANUFACTURER_DATA];          // Datos extras para el advertising


// ADVERTISING ------------------------------------------------------------------------------------------------------------------------------------------

static const struct bt_data ad[] = {
     BT_DATA_BYTES( BT_DATA_FLAGS, BLE_CONF_ADV_FLAGS),
	BT_DATA(BT_DATA_NAME_COMPLETE, BLE_CONF_DEVICE_NAME, sizeof(BLE_CONF_DEVICE_NAME) - 1),
};

static const struct bt_data sd[] = {
     BT_DATA_BYTES(BT_DATA_UUID128_ALL, BLE_CONF_ADV_UUID_SERVICE),
     BT_DATA( BT_DATA_MANUFACTURER_DATA, (unsigned char *)&manufacturer_data, sizeof(manufacturer_data)),
};

bool update_adv_manufacturer_data(uint8_t * data, uint8_t data_len)
{
     int err;
     
     if(data_len > MAX_LEN_MANUFACTURER_DATA) return false;
     
     for(uint8_t i = 0 ; i<data_len ; i++) manufacturer_data[i] = data[i];
     
     if(flag_adv_start)
     {
          err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));	
          if(err) LOG_ERR("Update manufacturer data failed (Error: %d)", err);
          else return true;
          return false;
     }
     return false;
} 

static int advertising_start()
{
     int err;

	struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM( BLE_CONF_ADV_OPTIONS,
                                                          BLE_CONF_ADV_MIN_INTERVAL,
                                                          BLE_CONF_ADV_MAX_INTERVAL,
                                                          BLE_CONF_ADV_ADDR_DIREC);

	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
     if(err == -EALREADY) LOG_INF("Advertising continuous");
     else if (err)
     {
          LOG_ERR("Advertising start failed (Error: %d)",err);
          return err;
     }
     else LOG_INF("Advertising start successfully");
     flag_adv_start = true;

     return 0;
}


// CONNECTION -------------------------------------------------------------------------------------------------------------------------------------------

static void on_le_data_len_updated(struct bt_conn *conn, struct bt_conn_le_data_len_info *info)
{
     LOG_INF("Data len update:"); 
     LOG_INF("     Length TX: %d bytes - Length RX: %d bytes, Time TX: %d us - Time RX: %d us", info->tx_max_len, info->rx_max_len, info->tx_max_time, info->rx_max_time);
}

static void on_le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param)
{
     LOG_INF("PHY updated:");

     if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_1M)            LOG_INF("     New PHY: 1M");
     else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_2M)       LOG_INF("     New PHY: 2M");
     else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_CODED_S8) LOG_INF("     New PHY: Long Range");
     else LOG_INF("     New PHY don't detected");
}

static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{
     int err;

     struct bt_conn_info info;
     err = bt_conn_get_info(conn, &info);
     IF_BLE_ERROR(err, "get conn info failed (Error: %d)")

     LOG_INF("Connection parameters updated:"); 
     LOG_INF("     Interval %.2f ms - Latency %d - Timeout %d ms", interval*1.25, latency, timeout*10);
}

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
     int err;

     GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);

	if(conn_err)
	{
		LOG_ERR("Connected to %s failed (Error: %d)", addr, conn_err);
          
          if(!advertising_start()) flag_ble_error=true;

          bt_conn_unref(central_conn);
          central_conn = NULL;

          return;
	}

     LOG_INF("Connected to %s", addr);
     central_conn = bt_conn_ref(conn);

     err = bt_passkey_set(BLE_CONF_PASSKEY);
     IF_BLE_ERROR(err, "Set passkey failed (Error: %d)")

     err = bt_conn_set_security(conn, BLE_CONF_SECURITY_LEVEL);
     IF_BLE_ERROR(err, "Set security level failed (Error: %d)")

     #ifdef BLE_CONF_ENABLE_CONN_PARAM_EXCHANGE
     static struct bt_le_conn_param conn_param = {
          .interval_min  = BLE_CONF_CONN_MIN_INTERVAL/1.25,
          .interval_max  = BLE_CONF_CONN_MAX_INTERVAL/1.25,
          .latency       = BLE_CONF_CONN_LATENCY,
          .timeout       = FINAL_TIMEOUT/10,
     };
     bt_conn_le_param_update(conn,&conn_param);
     #endif//BLE_CONF_ENABLE_CONN_PARAM_EXCHANGE
     
     #ifdef BLE_CONF_ENABLE_PHY_EXCHANGE
     const struct bt_conn_le_phy_param preferred_phy = {
        .options         = BLE_CONF_PHY_OPTION,
        .pref_rx_phy     = BLE_CONF_PHY_RX,
        .pref_tx_phy     = BLE_CONF_PHY_TX,
     };
     err = bt_conn_le_phy_update(conn, &preferred_phy);
     if(err) LOG_ERR("PHY update failet (Error: %d)", err);
     #endif//BLE_CONF_ENABLE_PHY_EXCHANGE

     #ifdef BLE_CONF_ENABLE_LOG_CONN_PARAM
     struct bt_conn_info info;
     err = bt_conn_get_info(conn, &info);
     IF_BLE_ERROR(err, "Get conn info failed (Error: %d)")

     LOG_DBG("Connection Parameters:");
     LOG_DBG("     Interval %.2f ms - Latency %d - Timeout %d ms", info.le.interval*1.25, info.le.latency, info.le.timeout*10);
     LOG_DBG("     PHY RX: %d M - PHY TX: %d M",info.le.phy->rx_phy ,info.le.phy->tx_phy);
     LOG_DBG("     Length TX: %d bytes - Length RX: %d bytes, Time TX: %d us - Time RX: %d us", info.le.data_len->tx_max_len, info.le.data_len->rx_max_len, info.le.data_len->tx_max_time, info.le.data_len->rx_max_time);
     LOG_DBG("     Security Level %d",info.security.level);
     #endif//BLE_CONF_ENABLE_LOG_CONN_PARAM
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
     GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);

     LOG_INF("Disconnected to %s (Reason %x)", addr, reason);

     if(central_conn != conn) LOG_ERR("Disconnected from an unassigned central");
     bt_conn_unref(central_conn);
     central_conn = NULL;
     
     if(!advertising_start()) flag_ble_error=true;
}

static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
	GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);

	if (!err) LOG_INF("Security changed to %s - level %u", addr, level);
	else LOG_WRN("Security changed to %s failed - level %u (EROOR: %d)", addr, level, err);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected               = connected,
	.disconnected            = disconnected,
	.security_changed        = security_changed,

     .le_data_len_updated     = on_le_data_len_updated,
     .le_phy_updated          = on_le_phy_updated,
     .le_param_updated        = le_param_updated,
};


// AUTHENTUCATION AND PAIRING ---------------------------------------------------------------------------------------------------------------------------

/*
static void passkey_entry(struct bt_conn *conn)
{
     GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);
	LOG_INF("Passkey entry request: %s", addr);
     
     #ifdef BLE_CONF_ROLE_CENTRAL
     int err;
     err = bt_conn_auth_passkey_entry(conn, BLE_CONF_PASSKEY);
     IF_BLE_ERROR(err, "Passkey entry failed (Error: %d)")
     #endif//BLE_CONF_ROLE_CENTRAL
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
     bt_conn_auth_passkey_confirm(conn);
}
*/

static void auth_cancel(struct bt_conn *conn)
{
	GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);
	LOG_WRN("Pairing cancelled to %s", addr);
}

static void pairing_confirm(struct bt_conn *conn)
{
     GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);
	LOG_INF("Pairing confirm request to %s", addr);
     
     int err = bt_conn_auth_pairing_confirm(central_conn);
     IF_BLE_ERROR(err, "Pairing confirm failed (Error: %d)",return)
     IF_BLE_NO_ERROR(err, "Pairing confirm accepted")

     /*if(bt_addr_le_eq(bt_conn_get_dst(conn),bt_conn_get_dst(central_conn)))
     {
          int err = bt_conn_auth_pairing_confirm(central_conn);
          IF_BLE_ERROR(err, "pairing confirm failed (Error: %d)",return)
          LOG_INF("pairing confirmation request accepted");
     }
     else LOG_WRN("Pairing confirmation request denied");*/
}

static struct bt_conn_auth_cb conn_auth_cb = {
	.cancel             = auth_cancel,
     .pairing_confirm    = pairing_confirm,
     //.passkey_display    = auth_passkey_display,
	//.passkey_confirm    = auth_passkey_confirm,
     //.passkey_entry      = passkey_entry,
};

static void pairing_complete(struct bt_conn *conn, bool bonded)
{
	GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);
	LOG_INF("Pairing complete to %s, bonded: %d", addr, bonded);
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
	GET_BT_ADDR_STR(bt_conn_get_dst(conn), addr);
	LOG_WRN("Pairing failed to %s (Reason %d)", addr, reason);
}

static struct bt_conn_auth_info_cb conn_auth_info_cb = {
	.pairing_complete = pairing_complete,
	.pairing_failed = pairing_failed,
};


// BLE MACHINE ------------------------------------------------------------------------------------------------------------------------------------------

void BLE_manager()
{
     uint8_t err;
     switch (manager_state)
     {
          case BLE_INIT:

               err = bt_conn_auth_cb_register(&conn_auth_cb);
               if(err) LOG_ERR("Auth cb register failed (Error: %d)",err);
               //IF_BLE_ERROR(err, "Auth cb register failed (Error: %d)", flag_ble_error=true; return)

               err = bt_conn_auth_info_cb_register(&conn_auth_info_cb);
               if(err) LOG_ERR("Auth info cb register failed (Error: %d)",err);
               //IF_BLE_ERROR(err, "Auth info cb register failed (Error: %d)", flag_ble_error=true; return)

               err = bt_enable(NULL);
               if(err) LOG_ERR("Bluetooth init failed (Error: %d)",err);
               //IF_BLE_ERROR(err, "Bluetooth init failed (Error: %d)", flag_ble_error=true; return)

               err = advertising_start();
               if(err) LOG_ERR("Advertising star in init failed (Error: %d)",err);
               //IF_BLE_ERROR(err, "Advertising star in init failed (Error: %d)", flag_ble_error = false; return)

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

#endif//CONF_BT_PERIPHERAL
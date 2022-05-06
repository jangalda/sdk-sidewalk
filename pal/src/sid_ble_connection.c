/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_ble_connection.h>
#include <sid_ble_adapter_callbacks.h>

#include <bluetooth/bluetooth.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(sid_ble_conn, CONFIG_SIDEWALK_LOG_LEVEL);

static void ble_ev_connected(struct bt_conn *conn, uint8_t err);
static void ble_ev_disconnected(struct bt_conn *conn, uint8_t reason);

static sid_ble_conn_params_t conn_params;
static sid_ble_conn_params_t *p_conn_params_out;

static struct bt_conn_cb conn_callbacks = {
	.connected = ble_ev_connected,
	.disconnected = ble_ev_disconnected,
};

/**
 * @brief The function is called when a new connection is established.
 *
 * @param conn new connection object.
 * @param err HCI error, zero for success, non-zero otherwise.
 */
static void ble_ev_connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_ERR("Connection failed (err %u)\n", err);
		return;
	}

	const bt_addr_le_t *bt_addr_le = bt_conn_get_dst(conn);
	if (bt_addr_le) {
		memcpy(conn_params.addr, bt_addr_le->a.val, BT_ADDR_SIZE);
	} else {
		LOG_ERR("Connection bt address not found.");
		memset(conn_params.addr, 0x00, BT_ADDR_SIZE);
	}

	conn_params.conn = bt_conn_ref(conn);
	sid_ble_adapter_conn_connected((const uint8_t *)conn_params.addr);
}

/**
 * @brief The function is called when a connection has been disconnected.
 *
 * @param conn connection object.
 * @param err HCI disconnection reason.
 */
static void ble_ev_disconnected(struct bt_conn *conn, uint8_t reason)
{
	if (conn_params.conn == conn) {
		bt_conn_unref(conn_params.conn);
		conn_params.conn = NULL;

		sid_ble_adapter_conn_disconnected((const uint8_t *)conn_params.addr);
	}
}

const sid_ble_conn_params_t *sid_ble_conn_params_get(void)
{
	return (const sid_ble_conn_params_t *)p_conn_params_out;
}

void sid_ble_conn_init(void)
{
	p_conn_params_out = &conn_params;

	bt_conn_cb_register(&conn_callbacks);
}

int sid_ble_conn_disconnect(void)
{
	return bt_conn_disconnect(conn_params.conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

void sid_ble_conn_deinit(void)
{
	p_conn_params_out = NULL;
}
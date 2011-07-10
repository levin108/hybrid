#include "util.h"
#include "account.h"
#include "gtkconn.h"

#ifdef USE_NETWORKMANAGER
#include <dbus/dbus-glib.h>
#include <NetworkManager.h>

static DBusGConnection *nm_conn = NULL;
static DBusGProxy *nm_proxy = NULL;

static void
nm_state_change_cb(DBusGProxy *proxy, NMState state, gpointer data)
{
	switch(state)
	{
		case NM_STATE_CONNECTED:
			hybrid_debug_info("conn", "network is connected");
			hybrid_account_enable_all();
			break;
		case NM_STATE_ASLEEP:
			hybrid_debug_info("conn", "network is sleeping...");
			break;
		case NM_STATE_CONNECTING:
			break;
		case NM_STATE_DISCONNECTED:
			hybrid_account_close_all();
			hybrid_debug_info("conn", "network is disconnected");
			break;
		case NM_STATE_UNKNOWN:
			hybrid_debug_info("conn", "unknown network state");
		default:
			break;
	}
}

void
hybrid_conn_init(void)
{
	GError *error = NULL;

	nm_conn = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);

	if (!nm_conn) {

		hybrid_debug_error("conn",
				"Error connecting to DBus System service: %s.\n", 
				error->message);

	} else {
		nm_proxy = dbus_g_proxy_new_for_name(nm_conn,
		                                     NM_DBUS_SERVICE,
		                                     NM_DBUS_PATH,
		                                     NM_DBUS_INTERFACE);
		dbus_g_proxy_add_signal(nm_proxy, "StateChange", G_TYPE_UINT, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(nm_proxy, "StateChange",
		                        G_CALLBACK(nm_state_change_cb), NULL, NULL);
	}
}

#else /* USE_NETWORKMANAGER */

void
hybrid_conn_init(void)
{
	return;
}

#endif /* USE_NETWORKMANAGER */

#include <glib.h>
#include "util.h"
#include "eventloop.h"
#include "account.h"
#include "module.h"
#include "info.h"
#include "blist.h"
#include "notify.h"
#include "action.h"
#include "conv.h"
#include "gtkutils.h"
#include "tooltip.h"

const gchar *jabber_server = "talk.l.google.com";

static gboolean
init_stream_cb(gint sk, gpointer user_data)
{
	gchar buf[BUF_LENGTH];
	gint n;

	if ((n = recv(sk, buf, strlen(buf), 0)) == -1) {
		
		hybrid_debug_error("xmpp", "init stream error.");

		return FALSE;
	}

	buf[n] = '\0';

	g_print("%s", buf);

	return TRUE;
}

static gboolean
init_connect_cb(gint sk, gpointer user_data)
{
	const gchar *msg;

	/* send version. */
	msg = "<?xml version='1.0' ?>";

	if (send(sk, msg, strlen(msg), 0) == -1) {

		hybrid_debug_error("xmpp", "send initial jabber request failed");

		return FALSE;
	}

	/* send stream request. */
	msg = "<stream:stream to='gmail.com' xmlns='jabber:client'"
		" xmlns:stream='http://etherx.jabber.org/streams' version='1.0'>";

	if (send(sk, msg, strlen(msg), 0) == -1) {

		hybrid_debug_error("xmpp", "send initial jabber request failed");

		return FALSE;
	}

	hybrid_event_add(sk, HYBRID_EVENT_READ, init_stream_cb, NULL);

	return FALSE;
}

static gboolean
xmpp_login(HybridAccount *account)
{
	hybrid_proxy_connect(jabber_server, 5222, init_connect_cb, NULL);

	return FALSE;
}

HybridModuleInfo module_info = {
	"xmpp",                     /**< name */
	"levin108",                   /**< author */
	N_("jabber client"),          /**< summary */
	/* description */
	N_("implement xmpp protocol"), 
	"http://basiccoder.com",      /**< homepage */
	"0","1",                      /**< major version, minor version */
	"xmpp",                     /**< icon name */

	xmpp_login,                 /**< login */
	NULL,              /**< get_info */
	NULL,          /**< change_state */
	NULL,            /**< keep_alive */
	NULL,       /**< account_tooltip */
	NULL,         /**< buddy_tooltip */
	NULL,            /**< buddy_move */
	NULL,                /**< buddy_remove */
	NULL,                /**< buddy_rename */
	NULL,             /**< buddy_add */
	NULL,          /**< group_rename */
	NULL,          /**< group_remove */
	NULL,             /**< group_add */
	NULL,       /**< chat_word_limit */
	NULL,            /**< chat_start */
	NULL,             /**< chat_send */
	NULL,                 /**< close */
	NULL,               /**< actions */
};

void 
xmpp_module_init(HybridModule *module)
{

}

HYBRID_MODULE_INIT(xmpp_module_init, &module_info);

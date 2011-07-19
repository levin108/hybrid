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

#include "xmpp_stream.h"
#include "xmpp_buddy.h"

const gchar *jabber_server = "talk.l.google.com";

static gboolean
xmpp_login(HybridAccount *account)
{
	XmppAccount *ac;
	XmppStream *stream;

	ac = xmpp_account_create(account, account->username,
						account->password, "gmail.com");
	
	stream = xmpp_stream_create(ac);

	hybrid_account_set_protocol_data(account, stream);

	hybrid_proxy_connect(jabber_server, 5222,
			(connect_callback)xmpp_stream_init, stream);

	return FALSE;
}

static gboolean
xmpp_buddy_tooltip(HybridAccount *account, HybridBuddy *buddy,
		HybridTooltipData *tip_data)
{
	XmppBuddy *bd;

	if (!(bd = xmpp_buddy_find(buddy->id))) {
		return FALSE;
	}

	hybrid_tooltip_data_add_pair(tip_data, "ID", bd->jid);
	hybrid_tooltip_data_add_pair(tip_data, "Name", bd->name);
	hybrid_tooltip_data_add_pair(tip_data, "Mood", bd->status);
	hybrid_tooltip_data_add_pair(tip_data, "Status",
	                             hybrid_get_presence_name(buddy->state));
	hybrid_tooltip_data_add_pair(tip_data, "Subscription", bd->subscription);
	hybrid_tooltip_data_add_pair(tip_data, "Resource", bd->resource);

	return TRUE;
}

static gboolean
xmpp_buddy_rename(HybridAccount *account, HybridBuddy *buddy, const gchar *text)
{
	XmppBuddy *xbuddy;

	if (!(xbuddy = xmpp_buddy_find(buddy->id))) {
		return FALSE;
	}

	if (xmpp_buddy_alias(xbuddy, text) != HYBRID_OK) {
		return FALSE;
	}

	return TRUE;
}

static void 
xmpp_group_add(HybridAccount *account, const gchar *text)
{
	hybrid_blist_add_group(account, text, text);
}

static gboolean 
xmpp_buddy_move(HybridAccount *account, HybridBuddy *buddy,
		HybridGroup *new_group)
{
	XmppBuddy *xbuddy;

	if (!(xbuddy = xmpp_buddy_find(buddy->id))) {
		return FALSE;
	}

	if (xmpp_buddy_set_group(xbuddy, new_group->name) != HYBRID_OK) {
		return FALSE;
	}

	return TRUE;
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
	NULL,
	NULL,
	NULL,          /**< change_state */
	NULL,            /**< keep_alive */
	NULL,       /**< account_tooltip */
	xmpp_buddy_tooltip,         /**< buddy_tooltip */
	xmpp_buddy_move,            /**< buddy_move */
	NULL,                /**< buddy_remove */
	xmpp_buddy_rename,                /**< buddy_rename */
	NULL,             /**< buddy_add */
	NULL,          /**< group_rename */
	NULL,          /**< group_remove */
	xmpp_group_add,             /**< group_add */
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

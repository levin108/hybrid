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
#include "xmpp_account.h"
#include "xmpp_message.h"

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
xmpp_modify_name(HybridAccount *account, const gchar *name)
{
	XmppStream *stream;

	stream = hybrid_account_get_protocol_data(account);

	if (xmpp_account_modify_name(stream, name) != HYBRID_OK) {
		return FALSE;
	}

	return TRUE;
}

static gboolean
xmpp_modify_status(HybridAccount *account, const gchar *status)
{
	XmppStream *stream;

	stream = hybrid_account_get_protocol_data(account);

	if (xmpp_account_modify_status(stream, account->state, status) != HYBRID_OK) {
		return FALSE;
	}

	return TRUE;
}

static gboolean
xmpp_modify_photo(HybridAccount *account, const gchar *filename)
{
	XmppStream *stream;

	stream = hybrid_account_get_protocol_data(account);

	if (xmpp_account_modify_photo(stream, filename) != HYBRID_OK) {
		return FALSE;
	}

	return TRUE;
}

static gboolean
xmpp_change_state(HybridAccount *account, gint state)
{
	XmppStream *stream;

	stream = hybrid_account_get_protocol_data(account);

	if (xmpp_account_modify_status(stream, state,
				account->status_text) != HYBRID_OK) {
		return FALSE;
	}

	return TRUE;
}

static gboolean
xmpp_account_tooltip(HybridAccount *account, HybridTooltipData *tip_data)
{
	XmppStream *stream;
	gchar *status;

	stream = hybrid_account_get_protocol_data(account);

	status = g_strdup_printf("[%s] %s", 
			hybrid_get_presence_name(account->state),
			account->status_text ? account->status_text : "");

	hybrid_tooltip_data_add_title(tip_data, account->username);
	if (account->nickname) {
		hybrid_tooltip_data_add_pair(tip_data, "Name", account->nickname);
	}
	hybrid_tooltip_data_add_pair(tip_data, "Status", status);
	//hybrid_tooltip_data_add_pair(tip_data, "Resource", bd->resource);

	return TRUE;
}

static gboolean
xmpp_buddy_tooltip(HybridAccount *account, HybridBuddy *buddy,
		HybridTooltipData *tip_data)
{
	XmppBuddy *bd;
	gchar *status;

	if (!(bd = xmpp_buddy_find(buddy->id))) {
		return FALSE;
	}

	status = g_strdup_printf("[%s] %s", 
			hybrid_get_presence_name(buddy->state),
			bd->status ? bd->status : "");

	hybrid_tooltip_data_add_title(tip_data, bd->jid);
	hybrid_tooltip_data_add_pair(tip_data, "Name", bd->name);
	hybrid_tooltip_data_add_pair(tip_data, "Status", status);
	hybrid_tooltip_data_add_pair(tip_data, "Subscription", bd->subscription);
	hybrid_tooltip_data_add_pair(tip_data, "Resource", bd->resource);

	g_free(status);

	return TRUE;
}

static gboolean
xmpp_buddy_remove(HybridAccount *account, HybridBuddy *buddy)
{
	XmppBuddy *xbuddy;

	if (!(xbuddy = xmpp_buddy_find(buddy->id))) {
		return TRUE;
	}

	if (xmpp_buddy_delete(xbuddy) != HYBRID_OK) {
		return FALSE;
	}

	xmpp_buddy_destroy(xbuddy);

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

static gboolean
xmpp_buddy_add(HybridAccount *account, HybridGroup *group, const gchar *name,
			const gchar *alias, const gchar *tips)
{
	XmppStream *stream;

	stream = hybrid_account_get_protocol_data(account);

	if (xmpp_roster_add_item(stream, name, alias, group->name) != HYBRID_OK) {
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

static void
xmpp_chat_send(HybridAccount *account, HybridBuddy *buddy, const gchar *text)
{
	XmppStream *stream;

	stream = hybrid_account_get_protocol_data(account);

	xmpp_message_send(stream, text, buddy->id);
}

static void
xmpp_close(HybridAccount *account)
{
	XmppStream *stream;

	stream = hybrid_account_get_protocol_data(account);

	g_source_remove(stream->source);
	close(stream->sk);

	xmpp_stream_destroy(stream);

	xmpp_buddy_clear();
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
	xmpp_modify_name,           /**< modify_name */
	xmpp_modify_status,         /**< modify_status */
	xmpp_modify_photo,          /**< modify_photo */
	xmpp_change_state,          /**< change_state */
	NULL,            /**< keep_alive */
	xmpp_account_tooltip,       /**< account_tooltip */
	xmpp_buddy_tooltip,         /**< buddy_tooltip */
	xmpp_buddy_move,            /**< buddy_move */
	xmpp_buddy_remove,          /**< buddy_remove */
	xmpp_buddy_rename,          /**< buddy_rename */
	xmpp_buddy_add,             /**< buddy_add */
	NULL,          /**< group_rename */
	NULL,          /**< group_remove */
	xmpp_group_add,             /**< group_add */
	NULL,       /**< chat_word_limit */
	NULL,            /**< chat_start */
	xmpp_chat_send,             /**< chat_send */
	xmpp_close,                 /**< close */
	NULL,               /**< actions */
};

void 
xmpp_module_init(HybridModule *module)
{

}

HYBRID_MODULE_INIT(xmpp_module_init, &module_info);

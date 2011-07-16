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
#include "xmpp_login.h"

const gchar *jabber_server = "talk.l.google.com";

static gboolean
xmpp_login(HybridAccount *account)
{
	XmppStream *stream = xmpp_stream_create();

	stream->account = account;

	hybrid_account_set_protocol_data(account, stream);

	hybrid_proxy_connect(jabber_server, 5222,
			(connect_callback)init_connect, stream);

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

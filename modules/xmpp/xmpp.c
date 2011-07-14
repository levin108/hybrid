#include <glib.h>
#include "util.h"
#include "account.h"
#include "module.h"
#include "info.h"
#include "blist.h"
#include "notify.h"
#include "action.h"
#include "conv.h"
#include "gtkutils.h"
#include "tooltip.h"

static gboolean
xmpp_login(HybridAccount *account)
{
	printf("%s, %s\n", account->username, account->password);

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

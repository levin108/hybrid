#include <glib.h>
#include "util.h"
#include "account.h"
#include "module.h"
#include "info.h"
#include "blist.h"
#include "notify.h"

#include "fetion.h"
#include "fx_trans.h"
#include "fx_login.h"
#include "fx_account.h"
#include "fx_group.h"
#include "fx_buddy.h"
#include "fx_msg.h"

fetion_account *ac;

/**
 * Process "presence changed" message.
 */
static void
process_presence(fetion_account *ac, const gchar *sipmsg)
{
	GSList *list;
	GSList *pos;
	fetion_buddy *buddy;
	HybridBuddy *imbuddy;

	list = sip_parse_presence(ac, sipmsg);

	for (pos = list; pos; pos = pos->next) {

		buddy = (fetion_buddy*)pos->data;
		imbuddy = hybrid_blist_find_buddy(ac->account, buddy->userid);

		hybrid_blist_set_buddy_name(imbuddy, buddy->nickname);
		hybrid_blist_set_buddy_mood(imbuddy, buddy->mood_phrase);

		switch (buddy->state) {
			case P_ONLINE:
				hybrid_blist_set_buddy_state(imbuddy, HYBRID_STATE_ONLINE);
				break;
			case P_OFFLINE:
				hybrid_blist_set_buddy_state(imbuddy, HYBRID_STATE_OFFLINE);
				break;
			case P_INVISIBLE:
				hybrid_blist_set_buddy_state(imbuddy, HYBRID_STATE_OFFLINE);
				break;
			case P_AWAY:
				hybrid_blist_set_buddy_state(imbuddy, HYBRID_STATE_AWAY);
				break;
			case P_BUSY:
				hybrid_blist_set_buddy_state(imbuddy, HYBRID_STATE_BUSY);
				break;
			default:
				hybrid_blist_set_buddy_state(imbuddy, HYBRID_STATE_AWAY);
				break;
		}

		fetion_update_portrait(ac, buddy);
	}
}

/**
 * Process deregister message. When the same account logins
 * at somewhere else, this message will be received.
 */
static void
process_dereg_cb(fetion_account *ac, const gchar *sipmsg)
{
	hybrid_account_error_reason(ac->account, 
			_("Your account has logined elsewhere. You are forced to quit."));
}

/**
 * Process notification routine.
 */
static void
process_notify_cb(fetion_account *ac, const gchar *sipmsg)
{
	gint notify_type;
	gint event_type;

	sip_parse_notify(sipmsg, &notify_type, &event_type);

	if (notify_type == NOTIFICATION_TYPE_UNKNOWN ||
			event_type == NOTIFICATION_EVENT_UNKNOWN) {

		hybrid_debug_info("fetion", "recv unknown notification:\n%s", sipmsg);
		return;
	}

	switch (notify_type) {

		case NOTIFICATION_TYPE_PRESENCE:
			if (event_type == NOTIFICATION_EVENT_PRESENCECHANGED) {
				process_presence(ac, sipmsg);
			}
			break;

		case NOTIFICATION_TYPE_CONVERSATION :
			if (event_type == NOTIFICATION_EVENT_USERLEFT) {
			//	process_left_cb(ac, sipmsg);
				break;

			} else 	if (event_type == NOTIFICATION_EVENT_USERENTER) {
			//	process_enter_cb(ac, sipmsg);
				break;
			}
			break;

		case NOTIFICATION_TYPE_REGISTRATION :
			if (event_type == NOTIFICATION_EVENT_DEREGISTRATION) {
				process_dereg_cb(ac, sipmsg);
			}
			break;

		case NOTIFICATION_TYPE_SYNCUSERINFO :
			if (event_type == NOTIFICATION_EVENT_SYNCUSERINFO) {
			//	process_sync_info(ac, sipmsg);
			}
			break;

		case NOTIFICATION_TYPE_CONTACT :
			if (event_type == NOTIFICATION_EVENT_ADDBUDDYAPPLICATION) {
			//	process_add_buddy(ac, sipmsg);
			}
			break;
#if 0
		case NOTIFICATION_TYPE_PGGROUP :
			break;
#endif
		default:
			break;
	}
}

/**
 * Process the sip response message.
 */
static void
process_sipc_cb(fetion_account *ac, const gchar *sipmsg)
{
	gchar *callid;
	gint callid0;
	fetion_transaction *trans;
	GSList *trans_cur;

	if (!(callid = sip_header_get_attr(sipmsg, "I"))) {
		hybrid_debug_error("fetion", "invalid sipc message received\n%s",
				sipmsg);
		g_free(callid);
		return;
	}
	
	callid0 = atoi(callid);

	trans_cur = ac->trans_list;

	while(trans_cur) {
		trans = (struct transaction*)(trans_cur->data);

		if (trans->callid == callid0) {

			if (trans->callback) {
				(trans->callback)(ac, sipmsg, trans);
			}

			transaction_remove(ac, trans);

			break;
		}

		trans_cur = g_slist_next(trans_cur);
	}
	printf("%s\n", sipmsg);
}

/**
 * Process the message sip message.
 */
static void
process_message_cb(fetion_account *ac, const gchar *sipmsg)
{
	gchar *event;
	gchar *sysmsg_text;
	gchar *sysmsg_url;
	HybridNotify *notify;

	if ((event = sip_header_get_attr(sipmsg, "N")) &&
			g_strcmp0(event, "system-message") == 0) {
		if (fetion_message_parse_sysmsg(sipmsg, &sysmsg_text,
					&sysmsg_url) != HYBRID_OK) {
			return;
		}

		notify = hybrid_notify_create(ac->account, _("System Message"));
		hybrid_notify_set_text(notify, sysmsg_text);

		g_free(sysmsg_text);
		g_free(sysmsg_url);
	}

	g_free(event);
}

/**
 * Process the pushed message.
 */
void
process_pushed(fetion_account *ac, const gchar *sipmsg)
{
	gint type;
	
	type = fetion_sip_get_msg_type(sipmsg);

	switch (type) {
		case SIP_NOTIFICATION :	
			process_notify_cb(ac, sipmsg);
			break;
		case SIP_MESSAGE:
			process_message_cb(ac, sipmsg);
			break;
		case SIP_INVITATION:
			//process_invite_cb(ac, sipmsg);
			break;
		case SIP_INFO:
			//process_info_cb(ac, sipmsg);		
			break;
		case SIP_SIPC_4_0:
			process_sipc_cb(ac, sipmsg);	
			break;
		default:
			hybrid_debug_info("fetion", "recevie unknown msg:\n%s", sipmsg);
			break;
	}
}

static gboolean
fetion_login(HybridAccount *imac)
{
	HybridSslConnection *conn;

	hybrid_debug_info("fetion", "fetion is now logining...");

	ac = fetion_account_create(imac, imac->username, imac->password);

	hybrid_account_set_protocol_data(imac, ac);

	conn = hybrid_ssl_connect(SSI_SERVER, 443, ssi_auth_action, ac);

	return TRUE;
}

/**
 * Callback function for the get_info transaction.
 */
static gint 
get_info_cb(fetion_account *ac, const gchar *sipmsg, fetion_transaction *trans)
{
	HybridInfo *info;
	fetion_buddy *buddy;

	info = (HybridInfo*)trans->data;

	if (!(buddy = fetion_buddy_parse_info(ac, trans->userid, sipmsg))) {
		/* TODO show an error msg in the get-info box. */
		return HYBRID_ERROR;
	}

	hybrid_info_add_pair(info, _("Nickname"), buddy->nickname);
	hybrid_info_add_pair(info, _("Fetion-no"), buddy->sid);
	hybrid_info_add_pair(info, _("Mobile-no"), buddy->mobileno);
	hybrid_info_add_pair(info, _("Gender"), 
		buddy->gender == 1 ? _("Male") :
		(buddy->gender == 2 ? _("Female") : _("Secrecy")));
	hybrid_info_add_pair(info, _("Mood"), buddy->mood_phrase);
	/* TODO convert the code into the name. */
	hybrid_info_add_pair(info, _("Country"), buddy->country);
	hybrid_info_add_pair(info, _("Province"), buddy->province);
	hybrid_info_add_pair(info, _("City"), buddy->city);

	return HYBRID_OK;
}

static gboolean
fetion_change_state(HybridAccount *account, gint state)
{
	fetion_account *ac;
	gint fetion_state;

	ac = hybrid_account_get_protocol_data(account);

	switch(state) {
		case HYBRID_STATE_ONLINE:
			fetion_state = P_ONLINE;
			break;
		case HYBRID_STATE_AWAY:
			fetion_state = P_AWAY;
			break;
		case HYBRID_STATE_BUSY:
			fetion_state = P_BUSY;
			break;
		case HYBRID_STATE_INVISIBLE:
			fetion_state = P_INVISIBLE;
			break;
		default:
			fetion_state = P_ONLINE;
			break;
	}

	if (fetion_account_update_state(ac, fetion_state) != HYBRID_OK) {
		return FALSE;
	}

	return TRUE;
}

static gboolean 
fetion_buddy_move(HybridAccount *account, HybridBuddy *buddy,
		HybridGroup *new_group)
{
	fetion_account *ac;

	ac = hybrid_account_get_protocol_data(account);

	fetion_buddy_move_to(ac, buddy->id, new_group->id);
	return TRUE;
}

static void
fetion_get_info(HybridAccount *account, HybridBuddy *buddy)
{
	HybridInfo *info;
	fetion_account *ac;

	info = hybrid_info_create(buddy);

	ac = hybrid_account_get_protocol_data(account);

	fetion_buddy_get_info(ac, buddy->id, get_info_cb, info);
}

static gboolean
fetion_buddy_remove(HybridAccount *account, HybridBuddy *buddy)
{

	return TRUE;
}

static void
fetion_close(HybridAccount *account)
{
	GSList *pos;
	fetion_account *ac;
	fetion_buddy *buddy;
	fetion_group *group;

	ac = hybrid_account_get_protocol_data(account);

	/* close the socket */
	hybrid_event_remove(ac->source);
	close(ac->sk);

	/* destroy the group list */
	while (ac->groups) {
		pos = ac->groups;
		group = (fetion_group*)pos->data;
		ac->groups = g_slist_remove(ac->groups, group);
		fetion_group_destroy(group);
	}

	/* destroy the buddy list */
	while (ac->buddies) {
		pos = ac->buddies;
		buddy = (fetion_buddy*)pos->data;
		ac->buddies = g_slist_remove(ac->buddies, buddy);
		fetion_buddy_destroy(buddy);
	}
}

HybridModuleInfo module_info = {
	"fetion",                     /**< name */
	"levin108",                   /**< author */
	N_("fetion client"),          /**< summary */
	/* description */
	N_("hybrid plugin implementing Fetion Protocol version 4"), 
	"http://basiccoder.com",      /**< homepage */
	"0","1",                      /**< major version, minor version */
	"fetion",                     /**< icon name */

	fetion_login,                 /**< login */
	fetion_get_info,              /**< get_info */
	fetion_change_state,          /**< change_state */
	fetion_buddy_move,            /**< buddy_move */
	fetion_buddy_remove,          /**< buddy_remove */
	fetion_close,                 /**< close */
};

void 
fetion_module_init(HybridModule *module)
{

}

HYBRID_MODULE_INIT(fetion_module_init, &module_info);

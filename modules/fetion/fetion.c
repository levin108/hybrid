#include <glib.h>
#include "util.h"
#include "account.h"
#include "module.h"
#include "blist.h"

#include "fetion.h"
#include "fx_login.h"
#include "fx_account.h"
#include "fx_buddy.h"

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
	HybirdBuddy *imbuddy;

	list = sip_parse_presence(ac, sipmsg);

	for (pos = list; pos; pos = pos->next) {

		buddy = (fetion_buddy*)pos->data;
		imbuddy = hybird_blist_find_buddy(buddy->userid);

		hybird_blist_set_buddy_name(imbuddy, buddy->nickname);
		hybird_blist_set_buddy_mood(imbuddy, buddy->mood_phrase);

		switch (buddy->state) {
			case P_ONLINE:
				hybird_blist_set_buddy_state(imbuddy, Hybird_STATE_ONLINE);
				break;
			case P_OFFLINE:
				hybird_blist_set_buddy_state(imbuddy, Hybird_STATE_OFFLINE);
				break;
			case P_INVISIBLE:
				hybird_blist_set_buddy_state(imbuddy, Hybird_STATE_OFFLINE);
				break;
			case P_AWAY:
				hybird_blist_set_buddy_state(imbuddy, Hybird_STATE_AWAY);
				break;
			case P_BUSY:
				hybird_blist_set_buddy_state(imbuddy, Hybird_STATE_BUSY);
				break;
			default:
				hybird_blist_set_buddy_state(imbuddy, Hybird_STATE_AWAY);
				break;
		}

		fetion_update_portrait(ac, buddy);
	}
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

		hybird_debug_info("fetion", "recv unknown notification:\n%s", sipmsg);
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
			//	process_dereg_cb(ac, sipmsg);
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
			//process_message_cb(ac, sipmsg);
			break;
		case SIP_INVITATION:
			//process_invite_cb(ac, sipmsg);
			break;
		case SIP_INFO:
			//process_info_cb(ac, sipmsg);		
			break;
		case SIP_SIPC_4_0:
			//process_sipc_cb(ac, sipmsg);	
			break;
		default:
			hybird_debug_info("fetion", "recevie unknown msg:\n%s", sipmsg);
			break;
	}
}

gboolean
fetion_login(HybirdAccount *imac)
{
	hybird_debug_info("fetion", "fetion is now logining...");

	ac = fetion_account_create(imac, imac->username, imac->password);

	hybird_ssl_connect(SSI_SERVER, 443, ssi_auth_action, ac);

	return TRUE;
}

HybirdModuleInfo module_info = {
	"fetion",
	"levin108",
	N_("fetion client"),
	N_("libpurple plugin implementing Fetion Protocol version 4"),
	"http://basiccoder.com",
	"0","1",
	fetion_login,
};

void 
fetion_module_init(HybirdModule *module)
{
	printf("hello world\n");
}

Hybird_MODULE_INIT(fetion_module_init, &module_info);

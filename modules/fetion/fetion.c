/***************************************************************************
 *   Copyright (C) 2011 by levin                                           *
 *   levin108@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 ***************************************************************************/

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
#include "buddyreq.h"

#include "fetion.h"
#include "fx_trans.h"
#include "fx_login.h"
#include "fx_account.h"
#include "fx_group.h"
#include "fx_buddy.h"
#include "fx_msg.h"
#include "fx_util.h"

fetion_account *ac;

extern GSList *channel_list;

/**
 * Process "presence changed" message.
 */
static void
process_presence(fetion_account *ac, const gchar *sipmsg)
{
    GSList       *list;
    GSList       *pos;
    fetion_buddy *buddy;
    HybridBuddy  *imbuddy;

    list = sip_parse_presence(ac, sipmsg);

    for (pos = list; pos; pos = pos->next) {

        buddy   = (fetion_buddy*)pos->data;
        imbuddy = hybrid_blist_find_buddy(ac->account, buddy->userid);

        if (buddy->localname && *(buddy->localname) != '\0') {
            hybrid_blist_set_buddy_name(imbuddy, buddy->localname);
        }
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
                                _("Your account has logined elsewhere."
                                  "You are forced to quit."));
}

/**
 * Process the user left message, we close the current channel,
 * and remove the session from the session list.
 */
static void
process_left_cb(fetion_account *ac, const gchar *sipmsg)
{
    extern GSList *channel_list;
    GSList        *pos;

    g_return_if_fail(ac != NULL);
    g_return_if_fail(sipmsg != NULL);

    hybrid_debug_info("fetion", "buddy left, recv:\n%s", sipmsg);

    for (pos = channel_list; pos; pos = pos->next) {
        if (pos->data == ac) {
            channel_list = g_slist_remove(channel_list, ac);

            goto channel_removed;
        }
    }

    hybrid_debug_error("fetion", "FATAL, can't find channel");

    return;

channel_removed:
    /* remove the read event source. */
    g_source_remove(ac->source);

    /* close the channel. */
    close(ac->sk);

    /* destroy the account. */
    fetion_account_destroy(ac);
}

/**
 * Process the user entered message. The fetion prococol dont allow us
 * to send messages to an online buddy who's conversation channel is not
 * ready, so before chating with an online buddy, we should first start
 * a new conversation channel, and invite the buddy to the conversation,
 * but when the channel is ready? yes, when we got the User-Entered
 * message through the new channel established, the channel is ready!
 */
static void
process_enter_cb(fetion_account *ac, const gchar *sipmsg)
{
    GSList             *pos;
    fetion_transaction *trans;

    g_return_if_fail(ac != NULL);
    g_return_if_fail(sipmsg != NULL);

    hybrid_debug_info("fetion", "user entered:\n%s", sipmsg);

    /* Set the channel's ready flag. */
    ac->channel_ready = TRUE;

    /* Check the transaction waiting list, wakeup the sleeping transaction,
     * the transaction is either sending a SMS or sending a nudge, we got
     * the information from the transaction context, and restore the transaction. */
    while (ac->trans_wait_list) {
        pos = ac->trans_wait_list;
        trans = (fetion_transaction*)pos->data;

        if (trans->msg && *(trans->msg) != '\0') {
            fetion_message_send(ac, trans->userid, trans->msg);
        }

        transaction_wakeup(ac, trans);
    }
}

/**
 * Process the synchronization message, when the contact list or the personal info
 * changed, the server will push this message to tell the client to update its
 * local cache file, well, we will not update the local cache file, we keep the
 * old version numbers, and reload it after the next logining.
 */
static void
process_sync_info(fetion_account *ac, const gchar *sipmsg)
{
    GSList       *list;
    gchar        *sid;
    fetion_buddy *buddy;
    HybridBuddy  *hb;

    hybrid_debug_info("fetion", "sync info,recv:\n%s", sipmsg);

    if (!(list = sip_parse_sync(ac, sipmsg))) {
        return;
    }

    while (list) {
        buddy = (fetion_buddy*)list->data;

        list = g_slist_remove(list, buddy);

        if (buddy->status == 0) {
            continue;
        }

        if (!(hb = hybrid_blist_find_buddy(ac->account, buddy->userid))) {
            continue;
        }

        if (buddy->status == 1) {

            hybrid_blist_set_buddy_status(hb, TRUE);

        } else {
            hybrid_blist_set_buddy_status(hb, FALSE);
        }

        sid = get_sid_from_sipuri(buddy->sipuri);

        hybrid_message_box_show(HYBRID_MESSAGE_INFO,
                _("Buddy <b>%s</b> has %s your add-buddy request."),
                buddy->localname && *(buddy->localname) != '\0' ?
                buddy->localname : sid,
                buddy->status == 1 ? _("accepted") : _("declined"));
    }

}

static void
process_add_buddy(fetion_account *ac, const gchar *sipmsg)
{
    gchar                *sipuri;
    gchar                *userid;
    gchar                *desc;
    HybridBuddyReqWindow *req;

    hybrid_debug_info("fetion", "received add-buddy request:\n%s", sipmsg);

    if (sip_parse_appbuddy(sipmsg, &userid, &sipuri, &desc) != HYBRID_OK) {
        return;
    }

    req = hybrid_buddy_request_window_create(ac->account, userid, desc);
    hybrid_buddy_request_set_user_data(req, sipuri, g_free);

    g_free(userid);
    g_free(desc);
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
                process_left_cb(ac, sipmsg);
                break;

            } else     if (event_type == NOTIFICATION_EVENT_USERENTER) {
                process_enter_cb(ac, sipmsg);
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
                process_sync_info(ac, sipmsg);
            }
            break;

        case NOTIFICATION_TYPE_CONTACT :
            if (event_type == NOTIFICATION_EVENT_ADDBUDDYAPPLICATION) {
                process_add_buddy(ac, sipmsg);
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
    gchar              *callid;
    gint                callid0;
    fetion_transaction *trans;
    GSList             *trans_cur;

    if (!(callid = sip_header_get_attr(sipmsg, "I"))) {
        hybrid_debug_error("fetion", "invalid sipc message received\n%s",
                sipmsg);
        g_free(callid);
        return;
    }

    callid0 = atoi(callid);

    trans_cur = ac->trans_list;

    while(trans_cur) {
        trans = (fetion_transaction*)(trans_cur->data);

        if (trans->callid == callid0) {

            if (trans->callback) {
                (trans->callback)(ac, sipmsg, trans);
            }

            transaction_remove(ac, trans);

            break;
        }

        trans_cur = g_slist_next(trans_cur);
    }
}

/**
 * Process the message sip message.
 */
static void
process_message_cb(fetion_account *ac, const gchar *sipmsg)
{
    gchar        *event;
    gchar        *sysmsg_text;
    gchar        *sysmsg_url;
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

    hybrid_debug_info("fetion", "received message:\n%s", sipmsg);

    fetion_process_message(ac, sipmsg);

    g_free(event);
}

/**
 * Process the invitation message.
 */
static void
process_invite_cb(fetion_account *ac, const gchar *sipmsg)
{
    hybrid_debug_info("fetion", "invitation message recv:\n%s", sipmsg);

    fetion_process_invite(ac, sipmsg);
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
            process_invite_cb(ac, sipmsg);
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
fx_login(HybridAccount *imac)
{
    HybridSslConnection *conn;

    hybrid_debug_info("fetion", "fetion is now logging in...");

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
    HybridNotifyInfo *info;
    HybridBuddy      *hybrid_buddy;
    fetion_buddy     *buddy;
    gchar            *province;
    gchar            *city;
    GdkPixbuf        *pixbuf = NULL;

    //info = (HybridInfo*)trans->data;

    if (!(buddy = fetion_buddy_parse_info(ac, trans->userid, sipmsg))) {
        /* TODO show an error msg in the get-info box. */
        return HYBRID_ERROR;
    }

    province = buddy->province && *(buddy->province) != '\0' ?
                get_province_name(buddy->province) : g_strdup(_("Unknown"));

    city = buddy->city && *(buddy->city) != '\0' ?
                get_city_name(buddy->province, buddy->city) :
                g_strdup(_("Unknown"));

    info = hybrid_notify_info_create();

    hybrid_info_add_pair(info, _("Nickname"), buddy->nickname);
    if (*buddy->localname) {
        hybrid_info_add_pair(info, _("Localname"), buddy->localname);
    }
    hybrid_info_add_pair(info, _("Fetion-no"), buddy->sid);
    if (buddy->mobileno && *buddy->mobileno) {
        hybrid_info_add_pair(info, _("Mobile-no"), buddy->mobileno);
    }
    hybrid_info_add_pair(info, _("Gender"),
        buddy->gender == 1 ? _("Male") :
        (buddy->gender == 2 ? _("Female") : _("Secrecy")));

    if (*buddy->mood_phrase) {
        hybrid_info_add_pair(info, _("Mood"), buddy->mood_phrase);
    }

    hybrid_info_add_pair(info, _("Country"),
            !buddy->country || g_strcmp0(buddy->country, "CN") == 0 || !*buddy->country ?
            "China" : buddy->country);
    hybrid_info_add_pair(info, _("Province"), province);
    hybrid_info_add_pair(info, _("City"), city);

    if ((hybrid_buddy = hybrid_blist_find_buddy(ac->account, buddy->userid))) {

        if (hybrid_buddy->icon_data) {
            pixbuf = hybrid_create_pixbuf(hybrid_buddy->icon_data,
                    hybrid_buddy->icon_data_length);
            hybrid_info_add_pixbuf_pair(info, _("Photo"), pixbuf);

            if (pixbuf) {
                g_object_unref(pixbuf);
            }
        }
    }

    g_free(province);
    g_free(city);

    hybrid_info_notify(ac->account, info, buddy->userid);

    hybrid_notify_info_destroy(info);

    return HYBRID_OK;
}

static void
sms_to_me_send_cb(HybridAccount *account, const gchar *text)
{
    fetion_account *ac;

    ac = hybrid_account_get_protocol_data(account);

    /* TODO set feedback to the chat window. */
    fetion_message_send_to_me(ac, text);
}


static void
sms_to_me_cb(HybridAction *action)
{
    HybridAccount *account;
    HybridChatWindow *window;

    account = hybrid_action_get_account(action);

    window = hybrid_chat_window_create(
                account, "000000", HYBRID_CHAT_PANEL_USER_DEFINED
            );

    hybrid_chat_window_set_title(window, _("SMS To Me"));
    hybrid_chat_window_set_callback(window, (ChatCallback)sms_to_me_send_cb);
}

static gboolean
fx_change_state(HybridAccount *account, gint state)
{
    fetion_account *ac;
    gint            fetion_state;

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
fx_keep_alive(HybridAccount *account)
{
    fetion_account *ac;
    GSList         *pos;

    ac = hybrid_account_get_protocol_data(account);

    if (fetion_account_keep_alive(ac) != HYBRID_OK) {
        return FALSE;
    }

    for (pos = channel_list; pos; pos = pos->next) {
        ac = (fetion_account*)pos->data;
        fetion_account_keep_alive(ac);
    }

    return TRUE;
}

static gboolean
fx_account_tooltip(HybridAccount *account, HybridTooltipData *tip_data)
{
    fetion_account *ac;

    ac = hybrid_account_get_protocol_data(account);

    if (ac->mobileno && *ac->mobileno) {
        hybrid_tooltip_data_add_title(tip_data, ac->mobileno);

    } else {
        hybrid_tooltip_data_add_title(tip_data, ac->sid);
    }

    hybrid_tooltip_data_add_pair(tip_data, _("Name"), ac->nickname);
    hybrid_tooltip_data_add_pair(tip_data, _("Status"),
                                 hybrid_get_presence_name(account->state));

    if (ac->mobileno && *ac->mobileno) {
        hybrid_tooltip_data_add_pair(tip_data, _("Fetion Number"), ac->sid);
    } else {
        hybrid_tooltip_data_add_pair(tip_data, _("Mobile Number"),
                                     ac->mobileno);
    }
    hybrid_tooltip_data_add_pair(tip_data, _("Mood"), ac->mood_phrase);

    return TRUE;
}

static gboolean
fx_buddy_tooltip(HybridAccount *account, HybridBuddy *buddy, HybridTooltipData *tip_data)
{
    fetion_account *ac;
    fetion_buddy *bd;

    ac = hybrid_account_get_protocol_data(account);

    if (!(bd = fetion_buddy_find_by_userid(ac, buddy->id))) {
        return FALSE;
    }

    if (bd->mobileno && *bd->mobileno) {
        hybrid_tooltip_data_add_title(tip_data, bd->mobileno);

    } else {
        hybrid_tooltip_data_add_title(tip_data, bd->sid);
    }

    hybrid_tooltip_data_add_pair(tip_data, _("Name"), bd->nickname);

    if (bd->localname && *bd->localname) {
        hybrid_tooltip_data_add_pair(tip_data, _("Alias"), bd->localname);
    }
    hybrid_tooltip_data_add_pair(tip_data, _("Status"),
                                 hybrid_get_presence_name(buddy->state));

    if (bd->mobileno && *bd->mobileno) {
        hybrid_tooltip_data_add_pair(tip_data, _("Fetion Number"), bd->sid);
    } else {
        hybrid_tooltip_data_add_pair(tip_data, _("Mobile Number"),
                                     bd->mobileno);
    }

    if (bd->mood_phrase && *bd->mood_phrase) {
        hybrid_tooltip_data_add_pair(tip_data, _("Mood"), bd->mood_phrase);
    }

    return TRUE;
}

static gboolean
fx_buddy_move(HybridAccount *account, HybridBuddy *buddy,
        HybridGroup *new_group)
{
    fetion_account *ac;

    ac = hybrid_account_get_protocol_data(account);

    fetion_buddy_move_to(ac, buddy->id, new_group->id);
    return TRUE;
}

static void
fx_get_info(HybridAccount *account, HybridBuddy *buddy)
{
    HybridInfo *info;
    fetion_account *ac;

    info = hybrid_info_create(buddy);

    ac = hybrid_account_get_protocol_data(account);

    fetion_buddy_get_info(ac, buddy->id, get_info_cb, info);
}

static gboolean
fx_modify_name(HybridAccount *account, const gchar *text)
{
    fetion_account *ac;

    ac = hybrid_account_get_protocol_data(account);

    if (fetion_account_modify_name(ac, text) != HYBRID_OK) {
        return FALSE;
    }

    return TRUE;
}

static gboolean
fx_modify_status(HybridAccount *account, const gchar *text)
{
    fetion_account *ac;

    ac = hybrid_account_get_protocol_data(account);

    if (fetion_account_modify_status(ac, text) != HYBRID_OK) {
        return FALSE;
    }

    return TRUE;
}

static gboolean
fx_remove(HybridAccount *account, HybridBuddy *buddy)
{
    fetion_account *ac;

    ac = hybrid_account_get_protocol_data(account);

    if (fetion_buddy_remove(ac, buddy->id) != HYBRID_OK) {
        return FALSE;
    }

    return TRUE;
}

static gboolean
fx_rename(HybridAccount *account, HybridBuddy *buddy, const gchar *text)
{
    fetion_account *ac;

    ac = hybrid_account_get_protocol_data(account);

    if (fetion_buddy_rename(ac, buddy->id, text) != HYBRID_OK) {
        return FALSE;
    }

    return TRUE;
}

static gboolean
fx_buddy_req(HybridAccount *account, HybridGroup *group,
             const gchar *id, const gchar *alias,
             gboolean accept, const gpointer user_data)
{
    fetion_account *ac;
    const gchar    *sipuri;

    ac = hybrid_account_get_protocol_data(account);

    sipuri = (const gchar*)user_data;

    fetion_buddy_handle_request(ac, sipuri, id, alias, group->id, accept);

    return TRUE;
}

static gboolean
fx_buddy_add(HybridAccount *account, HybridGroup *group, const gchar *name,
             const gchar *alias, const gchar *tips)
{
    fetion_account *ac;

    ac = hybrid_account_get_protocol_data(account);

    if (fetion_buddy_add(ac, group->id, name, alias) != HYBRID_OK) {
        return FALSE;
    }

    return TRUE;
}

static gboolean
fx_group_rename(HybridAccount *account, HybridGroup *group, const gchar *text)
{
    fetion_account *ac;

    ac = hybrid_account_get_protocol_data(account);

    if (fetion_group_edit(ac, group->id, text) != HYBRID_OK) {
        return FALSE;
    }

    return TRUE;
}

static gboolean
fx_group_remove(HybridAccount *account, HybridGroup *group)
{
    fetion_account *ac;

    ac = hybrid_account_get_protocol_data(account);

    if (fetion_group_remove(ac, group->id) != HYBRID_OK) {
        return FALSE;
    }

    return TRUE;
}

static void
fx_group_add(HybridAccount *account, const gchar *text)
{
    fetion_account *ac;

    ac = hybrid_account_get_protocol_data(account);

    fetion_group_add(ac, text);
}

static gint
fx_chat_word_limit(HybridAccount *account)
{
    return 180;
}

static gboolean
fx_chat_start(HybridAccount *account, HybridBuddy *buddy)
{
    fetion_account *ac;

    ac = hybrid_account_get_protocol_data(account);

    if (!hybrid_blist_get_buddy_authorized(buddy)) {

        hybrid_message_box_show(HYBRID_MESSAGE_WARNING,
                "This buddy hasn't been authorized, you can't\n"
                "start a chat with him.");

        return FALSE;
    }

    return TRUE;
}

static void
fx_chat_send(HybridAccount *account, HybridBuddy *buddy, const gchar *text)
{
    fetion_account *ac;
    fetion_account *tmp_ac;
    extern GSList  *channel_list;
    GSList         *pos;

    ac = hybrid_account_get_protocol_data(account);

    if (BUDDY_IS_OFFLINE(buddy) || BUDDY_IS_INVISIBLE(buddy)) {
        fetion_message_send(ac, buddy->id, text);

    } else {
        /*
         * If the buddy's state is greater than 0, then we should
         * invite the buddy first to start a new socket channel,
         * then we can send the message through the new channel.
         * Now, we check whether a channel related to this buddy has
         * been established, if so, just send the message through
         * the existing one, otherwise, start a new channel.
         */
        for (pos = channel_list; pos; pos = pos->next) {
            tmp_ac = (fetion_account*)pos->data;

            if (g_strcmp0(tmp_ac->who, buddy->id) == 0) {
                /* yes, we got one. */
                fetion_message_send(tmp_ac, tmp_ac->who, text);

                return;
            }
        }

        fetion_message_new_chat(ac, buddy->id, text);
    }
}

static void
fx_close(HybridAccount *account)
{
    GSList         *pos;
    fetion_account *ac;
    fetion_buddy   *buddy;
    fetion_group   *group;

    ac = hybrid_account_get_protocol_data(account);

    /* close the socket */
    if (ac->source) {
        hybrid_event_remove(ac->source);
    }

    if (ac->sk) {
        close(ac->sk);
    }

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

static GSList*
fetion_actions(HybridAccount *account)
{
    GSList       *list = NULL;
    HybridAction *action;

    action = hybrid_action_create(account, _("SMS To Me"), sms_to_me_cb);
    list   = g_slist_append(list, action);

    return list;
}

HybridIMOps im_ops = {
    fx_login,                   /**< login */
    fx_get_info,                /**< get_info */
    fx_modify_name,             /**< modify_name */
    fx_modify_status,           /**< modify_status */
    NULL,                       /**< modify_photo */
    fx_change_state,            /**< change_state */
    fx_keep_alive,              /**< keep_alive */
    fx_account_tooltip,         /**< account_tooltip */
    fx_buddy_tooltip,           /**< buddy_tooltip */
    fx_buddy_move,              /**< buddy_move */
    fx_remove,                  /**< buddy_remove */
    fx_rename,                  /**< buddy_rename */
    fx_buddy_add,               /**< buddy_add */
    fx_buddy_req,               /**< buddy_req */
    fx_group_rename,            /**< group_rename */
    fx_group_remove,            /**< group_remove */
    fx_group_add,               /**< group_add */
    fx_chat_word_limit,         /**< chat_word_limit */
    fx_chat_start,              /**< chat_start */
    NULL,                       /**< chat_send_typing */
    fx_chat_send,               /**< chat_send */
    fx_close,                   /**< close */
};

HybridModuleInfo module_info = {
    "fetion",                   /**< name */
    "levin108",                 /**< author */
    N_("fetion client"),        /**< summary */
    /* description */
    N_("hybrid plugin implementing Fetion Protocol version 4"),
    "http://basiccoder.com",      /**< homepage */
    "0","1",                    /**< major version, minor version */
    "fetion",                   /**< icon name */
    MODULE_TYPE_IM,
    &im_ops,
    NULL,
    fetion_actions,             /**< actions */
    NULL,
};

void
fetion_module_init(HybridModule *module)
{

}

HYBRID_MODULE_INIT(fetion_module_init, &module_info);

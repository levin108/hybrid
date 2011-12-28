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

#include "util.h"
#include "conv.h"
#include "connect.h"
#include "xmlnode.h"

#include "fx_login.h"
#include "fx_msg.h"
#include "fx_buddy.h"
#include "fx_trans.h"

typedef struct {
    fetion_transaction *trans;
    gchar              *credential;
} invite_data;

typedef struct {
    fetion_account *account;
    gchar          *credential;
} invite_conn_data;


static gchar *generate_invite_buddy_body(const gchar *sipuri);

gint
fetion_message_parse_sysmsg(const gchar *sipmsg, gchar **content, gchar **url)
{
    gchar   *pos;
    xmlnode *root;
    xmlnode *node;

    if (!(pos = strstr(sipmsg, "\r\n\r\n"))) {
        goto sysmsg_error;
    }

    if (!(root = xmlnode_root(pos, strlen(pos)))) {
        goto sysmsg_error;
    }

    if (!(node = xmlnode_find(root, "content"))) {
        xmlnode_free(root);
        goto sysmsg_error;
    }

    *content = xmlnode_content(node);

    if ((node = xmlnode_find(root, "url"))) {
        *url = xmlnode_content(node);

    } else {
        *url = NULL;
    }

    xmlnode_free(root);

    return HYBRID_OK;

sysmsg_error:
    hybrid_debug_error("fetion", "invalid system message");
    return HYBRID_ERROR;
}


static gint
sms_response_cb(fetion_account *account, const gchar *sipmsg,
                fetion_transaction *trans)
{
    gint code;

    g_source_remove(trans->timer);

    code = fetion_sip_get_code(sipmsg);

    hybrid_debug_info("fetion", "send message response:\n%s", sipmsg);

    if (code != 200 && code != 280) {
        return HYBRID_ERROR;
    }

    /* TODO add error message to textview */


    return HYBRID_OK;
}

static void
sms_timeout_cb(fetion_transaction *trans)
{
    fetion_account *account;

    account = trans->data;

    /* TODO add error message to textview */
    hybrid_debug_error("fetion", "send message time out\n");

    g_source_remove(trans->timer);
    transaction_remove(account, trans);
}

gint
fetion_message_send(fetion_account *account, const gchar *userid,
                    const gchar *text)
{
    fetion_sip   *sip;
    sip_header   *toheader;
    sip_header   *cheader;
    sip_header   *kheader;
    sip_header   *nheader;
    gchar        *sip_text;
    fetion_buddy *buddy;

    g_return_val_if_fail(account != NULL, HYBRID_ERROR);
    g_return_val_if_fail(userid != NULL && *userid != '\0', HYBRID_ERROR);
    g_return_val_if_fail(text != NULL, HYBRID_ERROR);

    sip = account->sip;

    if (!(buddy = fetion_buddy_find_by_userid(account, userid))) {
        hybrid_debug_error("fetion", "FATAL, can't find specified buddy");

        return HYBRID_ERROR;
    }

    fetion_transaction *trans = transaction_create();

    transaction_set_userid(trans, userid);
    transaction_set_msg(trans, text);

    if (!account->channel_ready) {
        /* If the channel is not ready, make the transaction to wait
         * until the transaction is ready */
        transaction_wait(account, trans);

        hybrid_debug_info("fetion", "channel not ready, transaction sleep.");

        return HYBRID_OK;
    }

    fetion_sip_set_type(sip, SIP_MESSAGE);

    nheader  = sip_event_header_create(SIP_EVENT_CATMESSAGE);
    toheader = sip_header_create("T", buddy->sipuri);
    cheader  = sip_header_create("C", "text/plain");
    kheader  = sip_header_create("K", "SaveHistory");
    fetion_sip_add_header(sip, toheader);
    fetion_sip_add_header(sip, cheader);
    fetion_sip_add_header(sip, kheader);
    fetion_sip_add_header(sip, nheader);

    transaction_set_callid(trans, sip->callid);
    transaction_set_callback(trans, sms_response_cb);

    transaction_set_data(trans, account);
    transaction_set_timeout(trans, (GSourceFunc)sms_timeout_cb, trans);
    transaction_add(account, trans);

    sip_text = fetion_sip_to_string(sip, text);

    hybrid_debug_info("fetion", "send message, send:\n%s", sip_text);

    if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {
        g_free(sip_text);

        return HYBRID_ERROR;
    }

    g_free(sip_text);

    return HYBRID_OK;
}

static gint
sms_to_me_cb(fetion_account *account, const gchar *sipmsg,
             fetion_transaction *trans)
{
    printf("%s\n", sipmsg);

    return HYBRID_OK;
}

gint
fetion_message_send_to_me(fetion_account *account, const gchar *text)
{
    sip_header         *toheader;
    sip_header         *eheader;
    gchar              *sip_text;
    fetion_sip         *sip;
    fetion_transaction *trans;

    g_return_val_if_fail(account != NULL, HYBRID_ERROR);
    g_return_val_if_fail(text != NULL, HYBRID_ERROR);

    sip = account->sip;

    fetion_sip_set_type(sip, SIP_MESSAGE);

    toheader = sip_header_create("T", account->sipuri);
    eheader  = sip_event_header_create(SIP_EVENT_SENDCATMESSAGE);

    fetion_sip_add_header(sip, toheader);
    fetion_sip_add_header(sip, eheader);

    trans = transaction_create();
    transaction_set_callid(trans, sip->callid);
    transaction_set_callback(trans, sms_to_me_cb);
    transaction_add(account, trans);

    sip_text = fetion_sip_to_string(sip, text);

    hybrid_debug_info("fetion", "send sms to youself,send:\n%s", sip_text);

    if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {

        hybrid_debug_error("fetion", "send message to yourself error.");

        return HYBRID_ERROR;
    }

    g_free(sip_text);

    return HYBRID_OK;
}

gint
fetion_process_message(fetion_account *account, const gchar *sipmsg)
{
    gchar        *from;
    gchar        *sid;
    gchar        *callid;
    gchar        *sequence;
    gchar        *sendtime;
    gchar        *text;
    gchar        *sip_text;
    fetion_buddy *buddy;

    g_return_val_if_fail(account != NULL, HYBRID_ERROR);
    g_return_val_if_fail(sipmsg != NULL, HYBRID_ERROR);

    if (!(text = strstr(sipmsg, "\r\n\r\n"))) {
        hybrid_debug_error("fetion", "invalid message received\n");

        return HYBRID_ERROR;
    }

    text += 4;

    from     = sip_header_get_attr(sipmsg, "F");
    callid   = sip_header_get_attr(sipmsg, "I");
    sendtime = sip_header_get_attr(sipmsg, "D");
    sequence = sip_header_get_attr(sipmsg, "Q");

    sip_text = g_strdup_printf(
                    "SIP-C/4.0 200 OK\r\n"
                    "I: %s\r\n"
                    "Q: %s\r\n"
                    "F: %s\r\n\r\n", callid, sequence, from);
    g_free(callid);
    g_free(sendtime);
    g_free(sequence);

    hybrid_debug_info("fetion", "message response, send:\n%s", sip_text);

    if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {
        g_free(sip_text);
        g_free(from);

        return HYBRID_ERROR;
    }

    g_free(sip_text);

    sid = get_sid_from_sipuri(from);
    g_free(from);

    if (!(buddy = fetion_buddy_find_by_sid(account, sid))) {

        hybrid_debug_error("fetion", "invalid message received\n");
        g_free(sid);

        return HYBRID_ERROR;
    }

    hybrid_conv_got_message(account->account, buddy->userid, text, time(NULL));

    g_free(sid);


    return HYBRID_OK;
}

static gint
process_invite_conn_cb(gint sk, gpointer user_data)
{
    invite_conn_data *data;
    fetion_account   *account;
    fetion_sip       *sip;
    gchar            *credential;
    gchar            *sip_text;
    sip_header       *aheader;
    sip_header       *theader;
    sip_header       *mheader;
    sip_header       *nheader;

    data       = (invite_conn_data*)user_data;
    account    = data->account;
    credential = data->credential;
    g_free(data);

    account->sk = sk;
    sip         = account->sip;

    account->source = hybrid_event_add(sk, HYBRID_EVENT_READ,
                            hybrid_push_cb, account);

    if (account->source == 0) {

        hybrid_debug_error("fetion", "add read event error");

        return HYBRID_ERROR;
    }

    fetion_sip_set_type(sip, SIP_REGISTER);
    aheader = sip_credential_header_create(credential);
    theader = sip_header_create("K", "text/html-fragment");
    mheader = sip_header_create("K", "multiparty");
    nheader = sip_header_create("K", "nudge");

    g_free(credential);

    fetion_sip_add_header(sip, aheader);
    fetion_sip_add_header(sip, theader);
    fetion_sip_add_header(sip, mheader);
    fetion_sip_add_header(sip, nheader);

    sip_text = fetion_sip_to_string(sip, NULL);

    hybrid_debug_info("feiton", "register to a new channel:\n%s", sip_text);

    if (send(sk, sip_text, strlen(sip_text), 0) == -1) {

        hybrid_debug_error("fetion", "register to new channel error.");

        g_free(sip_text);

        return HYBRID_ERROR;
    }

    g_free(sip_text);

    return HYBRID_OK;
}

gint
fetion_process_invite(fetion_account *account, const gchar *sipmsg)
{
    gchar            *from;
    gchar            *auth;
    gchar            *ip;
    gchar            *credential;
    gchar            *sid;
    gint              port;
    gchar            *sip_text;
    fetion_account   *new_account;
    fetion_buddy     *buddy;
    invite_conn_data *data;

    g_return_val_if_fail(account != NULL, HYBRID_ERROR);
    g_return_val_if_fail(sipmsg != NULL, HYBRID_ERROR);

    from = sip_header_get_attr(sipmsg, "F");
    auth = sip_header_get_attr(sipmsg, "A");

    sip_header_get_auth(auth, &ip, &port, &credential);
    g_free(auth);

    sip_text = g_strdup_printf("SIP-C/4.0 200 OK\r\n"
                                "F: %s\r\n"
                                "I: 61\r\n"
                                "Q: 200002 I\r\n\r\n", from);

    hybrid_debug_info("fetion", "invite, send back:\n%s", sip_text);

    if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {

        hybrid_debug_error("fetion", "process an invitation error.");

        g_free(from);
        g_free(ip);
        g_free(credential);
        g_free(sip_text);

        return HYBRID_ERROR;
    }

    g_free(sip_text);

    sid = get_sid_from_sipuri(from);

    if (!(buddy = fetion_buddy_find_by_sid(account, sid))) {

        hybrid_debug_error("fetion", "can't find buddy %s", from);

        g_free(from);
        g_free(ip);
        g_free(credential);
        g_free(sid);

        return HYBRID_ERROR;
    }

    g_free(sid);

    new_account = fetion_account_clone(account);
    fetion_account_set_who(new_account, buddy->userid);

    data = g_new0(invite_conn_data, 1);
    data->account = new_account;
    data->credential = credential;

    hybrid_proxy_connect(ip, port, process_invite_conn_cb, data);

    g_free(from);
    g_free(ip);

    return HYBRID_OK;
}

static gint
invite_buddy_cb(fetion_account *account, const gchar *sipmsg,
                fetion_transaction *trans)
{
    hybrid_debug_info("fetion", "invite buddy response:\n%s", sipmsg);

    if (trans->msg && *(trans->msg) != '\0') {
        fetion_message_send(account, trans->userid, trans->msg);
    }

    return HYBRID_OK;
}

/**
 * Callback function to handle the invite-connect event, when we
 * get this acknowledge message, we should start to invite the buddy
 * to the conversation.
 *
 * The message received is:
 *
 * SIP-C/4.0 200 OK
 * I: 5
 * Q: 2 R
 * XI: 3d2ef745db9741a8946a57c40b0eb4d5
 * X: 1200
 * K: text/plain
 * K: text/html-fragment
 * K: multiparty
 * K: nudge
 *
 * then we send out invite-buddy request:
 */
static gint
chat_reg_cb(fetion_account *account, const gchar *sipmsg,
            fetion_transaction *trans)
{
    fetion_sip         *sip;
    sip_header         *eheader;
    gchar              *body;
    gchar              *sip_text;
    fetion_transaction *new_trans;
    fetion_buddy       *buddy;

    sip = account->sip;

    fetion_sip_set_type(sip, SIP_SERVICE);

    if (!(buddy = fetion_buddy_find_by_userid(account, trans->userid))) {

        hybrid_debug_error("fetion", "invite new buddy failed");

        return HYBRID_ERROR;
    }

    eheader = sip_event_header_create(SIP_EVENT_INVITEBUDDY);
    fetion_sip_add_header(sip, eheader);

    body = generate_invite_buddy_body(buddy->sipuri);

    new_trans = transaction_clone(trans);
    transaction_set_callid(new_trans, sip->callid);
    transaction_set_callback(new_trans, invite_buddy_cb);
    transaction_add(account, new_trans);

    sip_text = fetion_sip_to_string(sip, body);
    g_free(body);

    hybrid_debug_info("fetion", "invite new buddy,send:\n%s", sip_text);

    if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {

        hybrid_debug_error("fetion", "invite new buddy failed");

        return HYBRID_ERROR;
    }

    g_free(sip_text);

    return HYBRID_OK;
}

/**
 * Callback function to handle the new-chat connecting event.
 * The message is:
 *
 * R fetion.com.cn SIP-C/4.0
 * F: 547264589
 * I: 5
 * Q: 2 R
 * A: TICKS auth="2051600954.830102111"
 * K: text/html-fragment
 * K: multiparty
 * K: nudge
 */
static gint
invite_connect_cb(gint sk, gpointer user_data)
{
    fetion_transaction *trans;
    fetion_account     *account;
    fetion_sip         *sip;
    invite_data        *data;
    gchar              *credential;
    gchar              *sip_text;

    sip_header *aheader;
    sip_header *theader;
    sip_header *mheader;
    sip_header *nheader;

    data       = (invite_data*)user_data;
    trans      = data->trans;
    credential = data->credential;
    account    = trans->data;

    g_free(data);

    sip = account->sip;
    account->sk = sk;

    /* listen for this thread */
    account->source = hybrid_event_add(sk, HYBRID_EVENT_READ,
                        hybrid_push_cb, account);

    fetion_sip_set_type(sip, SIP_REGISTER);
    aheader = sip_credential_header_create(credential);
    theader = sip_header_create("K", "text/html-fragment");
    mheader = sip_header_create("K", "multiparty");
    nheader = sip_header_create("K", "nudge");

    transaction_set_callid(trans, sip->callid);
    transaction_set_callback(trans, chat_reg_cb);
    transaction_add(account, trans);

    fetion_sip_add_header(sip, aheader);
    fetion_sip_add_header(sip, theader);
    fetion_sip_add_header(sip, mheader);
    fetion_sip_add_header(sip, nheader);

    sip_text = fetion_sip_to_string(sip, NULL);

    hybrid_debug_info("fetion", "register, send:\n%s", sip_text);

    if (send(sk, sip_text, strlen(sip_text), 0) == -1) {

        hybrid_debug_error("fetion", "register to the new chat channel failed");

        return HYBRID_ERROR;
    }

    g_free(sip_text);
    g_free(credential);

    return HYBRID_OK;
}

/**
 * Callback function to handle the new_chat response message, if success
 * we would get the following message:
 *
 * SIP-C/4.0 200 OK
 * I: 4
 * Q: 2 S
 * A: CS address="221.176.31.128:8080;221.176.31.128:443",credential="439333922.916967705"
 *
 * Now we should start a new socket connect to 221.176.31.128:8080 with port 443 as
 * a back port if 8080 failed to connect.
 */
gint
new_chat_cb(fetion_account *account, const gchar *sipmsg,
            fetion_transaction *trans)
{
    gchar              *auth;
    gchar              *ip;
    gint                port;
    gchar              *credential;
    invite_data        *data;
    fetion_transaction *new_trans;
    fetion_account     *new_account;

    hybrid_debug_info("fetion", "%s\n", sipmsg);

    if (!(auth = sip_header_get_attr(sipmsg, "A"))) {

        hybrid_debug_error("fetion", "invalid invitation response.");

        return HYBRID_ERROR;
    }

    if (sip_header_get_auth(auth, &ip, &port, &credential) != HYBRID_OK) {

        hybrid_debug_error("fetion", "invalid invitation response.");

        return HYBRID_ERROR;
    }

    g_free(auth);

    new_trans = transaction_clone(trans);

    new_account = fetion_account_clone(account);
    fetion_account_set_who(new_account, trans->userid);

    transaction_set_data(new_trans, new_account);

    data = g_new0(invite_data, 1);
    data->trans = new_trans;
    data->credential = credential;

    hybrid_proxy_connect(ip, port, invite_connect_cb, data);

    g_free(ip);

    return HYBRID_OK;
}

gint
fetion_message_new_chat(fetion_account *account, const gchar *userid,
                        const gchar *text)
{
    fetion_sip         *sip;
    sip_header         *eheader;
    fetion_transaction *trans;
    gchar              *sip_text;

    g_return_val_if_fail(account != NULL, HYBRID_ERROR);
    g_return_val_if_fail(userid != NULL, HYBRID_ERROR);

    sip = account->sip;

    /*start chat*/
    fetion_sip_set_type(sip, SIP_SERVICE);
    eheader = sip_event_header_create(SIP_EVENT_STARTCHAT);
    fetion_sip_add_header(sip, eheader);

    trans = transaction_create();
    transaction_set_callid(trans, sip->callid);
    transaction_set_userid(trans, userid);
    transaction_set_msg(trans, text);
    transaction_set_callback(trans, new_chat_cb);
    transaction_add(account, trans);

    sip_text = fetion_sip_to_string(sip, NULL);

    hybrid_debug_info("fetion", "new chat,send:\n%s", sip_text);

    if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {

        hybrid_debug_error("fetion", "new chat failed");

        return HYBRID_ERROR;
    }

    g_free(sip_text);

    return HYBRID_OK;
}

static gchar*
generate_invite_buddy_body(const gchar *sipuri)
{
    const gchar *body;
    xmlnode     *root;
    xmlnode     *node;
    gchar       *res;

    g_return_val_if_fail(sipuri != NULL, NULL);

    body = "<args></args>";

    root = xmlnode_root(body, strlen(body));

    node = xmlnode_new_child(root, "contacts");
    node = xmlnode_new_child(node, "contact");
    xmlnode_new_prop(node, "uri", sipuri);

    res = xmlnode_to_string(root);

    xmlnode_free(root);

    return res;
}

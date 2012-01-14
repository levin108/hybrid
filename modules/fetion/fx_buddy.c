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

#include "blist.h"
#include "xmlnode.h"
#include "eventloop.h"
#include "util.h"
#include "gtkutils.h"

#include "fetion.h"
#include "fx_account.h"
#include "fx_buddy.h"

static gchar *generate_subscribe_body(void);
static gchar *generate_get_info_body(const gchar *userid);
static gchar *generate_buddy_move_body(const gchar *userid,
                const gchar *groupid);
static gchar *generate_remove_buddy_body(const gchar *userid);
static gchar *generate_rename_buddy_body(const gchar *userid,
                const gchar *name);
static gchar *generate_buddy_add_body(const gchar *no, const gchar *groupid,
                const gchar *localname, const gchar *desc);
static gchar *generate_handle_request_body(const gchar *sipuri,
        const gchar *userid, const gchar *alias, const gchar *groupid,
        gboolean accept);

fetion_buddy*
fetion_buddy_create(void)
{
    fetion_buddy *buddy;

    buddy = g_new0(fetion_buddy, 1);

    return buddy;
}

gint
fetion_buddy_scribe(fetion_account *ac)
{
    gchar      *res, *body;
    fetion_sip *sip;
    sip_header *eheader;

    sip = ac->sip;
    fetion_sip_set_type(sip, SIP_SUBSCRIPTION);

    eheader = sip_event_header_create(SIP_EVENT_PRESENCE);
    fetion_sip_add_header(sip, eheader);

    body = generate_subscribe_body();

    res = fetion_sip_to_string(sip, body);

    g_free(body);

    hybrid_debug_info("fetion", "send:\n%s", res);

    if (send(ac->sk, res, strlen(res), 0) == -1) {
        g_free(res);

        return HYBRID_ERROR;
    }

    g_free(res);

    return HYBRID_OK;
}

gint
fetion_buddy_get_info(fetion_account *ac, const gchar *userid,
        TransCallback callback, gpointer data)
{
    fetion_transaction *trans;
    fetion_sip         *sip;
    sip_header         *eheader;
    gchar              *body;
    gchar              *res;

    g_return_val_if_fail(ac != NULL, HYBRID_ERROR);
    g_return_val_if_fail(userid != NULL, HYBRID_ERROR);

    sip = ac->sip;

    fetion_sip_set_type(sip , SIP_SERVICE);
    eheader = sip_event_header_create(SIP_EVENT_GETCONTACTINFO);

    trans = transaction_create();
    transaction_set_callid(trans, sip->callid);
    transaction_set_userid(trans, userid);
    transaction_set_callback(trans, callback);
    transaction_set_data(trans, data);
    transaction_add(ac, trans);

    fetion_sip_add_header(sip, eheader);
    body = generate_get_info_body(userid);
    res = fetion_sip_to_string(sip, body);
    g_free(body);

    hybrid_debug_info("fetion", "send:\n%s", res);

    if (send(ac->sk, res, strlen(res), 0) == -1) {
        g_free(res);
        return HYBRID_ERROR;
    }

    g_free(res);

    return HYBRID_OK;
}

gint
fetion_buddy_remove(fetion_account *ac, const gchar *userid)
{
    fetion_sip *sip;
    sip_header *eheader;
    gchar      *res;
    gchar      *body;

    g_return_val_if_fail(ac != NULL, HYBRID_ERROR);
    g_return_val_if_fail(userid != NULL, HYBRID_ERROR);

    sip = ac->sip;

    fetion_sip_set_type(sip, SIP_SERVICE);
    eheader = sip_event_header_create(SIP_EVENT_DELETEBUDDY);
    fetion_sip_add_header(sip, eheader);

    body = generate_remove_buddy_body(userid);
    res = fetion_sip_to_string(sip, body);
    g_free(body);

    hybrid_debug_info("fetion", "remove buddy, send:\n%s", res);

    if (send(ac->sk, res, strlen(res), 0) == -1) {
        hybrid_debug_error("fetion", "remove buddy %s", userid);

        return HYBRID_ERROR;
    }

    g_free(res);

    return HYBRID_OK;
}

gint
fetion_buddy_rename(fetion_account *ac, const gchar *userid,
        const gchar *newname)
{
    fetion_sip *sip;
    sip_header *eheader;
    gchar      *res;
    gchar      *body;

    g_return_val_if_fail(ac != NULL, HYBRID_ERROR);
    g_return_val_if_fail(userid != NULL, HYBRID_ERROR);
    g_return_val_if_fail(newname != NULL, HYBRID_ERROR);

    sip = ac->sip;

    fetion_sip_set_type(sip, SIP_SERVICE);
    eheader = sip_event_header_create(SIP_EVENT_SETCONTACTINFO);
    fetion_sip_add_header(sip , eheader);

    body = generate_rename_buddy_body(userid, newname);
    res = fetion_sip_to_string(sip, body);
    g_free(body);

    if (send(ac->sk, res, strlen(res), 0) == -1) {
        hybrid_debug_error("fetion", "rename buddy %s", userid);

        return HYBRID_ERROR;
    }

    g_free(res);

    return HYBRID_OK;
}

gint
fetion_buddy_move_to(fetion_account *ac, const gchar *userid,
        const gchar *groupid)
{
    fetion_sip *sip;
    sip_header *eheader;
    gchar      *res, *body;

    g_return_val_if_fail(ac != NULL, HYBRID_ERROR);
    g_return_val_if_fail(userid != NULL, HYBRID_ERROR);
    g_return_val_if_fail(groupid != NULL, HYBRID_ERROR);

    sip = ac->sip;

    fetion_sip_set_type(sip, SIP_SERVICE);

    eheader = sip_event_header_create(SIP_EVENT_SETCONTACTINFO);

    fetion_sip_add_header(sip , eheader);
    body = generate_buddy_move_body(userid, groupid);
    res = fetion_sip_to_string(sip , body);
    g_free(body);

    hybrid_debug_info("fetion", "%s moved to group %s, send:\n%s",
                    userid, groupid, res);

    if (send(ac->sk, res, strlen(res), 0) == -1) {
        return HYBRID_ERROR;
    }

    g_free(res);

    return HYBRID_OK;
}

fetion_buddy*
fetion_buddy_parse_info(fetion_account *ac,
                        const gchar *userid, const gchar *sipmsg)
{
    xmlnode      *root;
    xmlnode      *node;
    gchar        *pos;
    gchar        *temp;
    gchar        *value;
    fetion_buddy *buddy;
    gint          code;

    code = fetion_sip_get_code(sipmsg);

    if (code != 200) {
        hybrid_debug_error("fetion", "get information with code:%d", code);
        return NULL;
    }

    if (!(pos = strstr(sipmsg, "\r\n\r\n"))) {
        goto get_info_error;
    }

    pos += 4;

    if (!(root = xmlnode_root(pos, strlen(pos)))) {
        goto get_info_error;
    }

    if (!(node = xmlnode_find(root, "contact"))) {
        xmlnode_free(root);
        goto get_info_error;
    }

    if (!(buddy = fetion_buddy_find_by_userid(ac, userid))) {
        xmlnode_free(root);
        goto get_info_error;
    }

    if (xmlnode_has_prop(node, "sid")) {
        value = xmlnode_prop(node, "sid");
        g_free(buddy->sid);
        buddy->sid = g_strdup(value);
        g_free(value);
    }

    if (xmlnode_has_prop(node, "mobile-no")) {
        value = xmlnode_prop(node, "mobile-no");
        g_free(buddy->mobileno);
        buddy->mobileno = g_strdup(value);
        g_free(value);
    }

    if (xmlnode_has_prop(node, "impresa")) {
        value = xmlnode_prop(node, "impresa");
        g_free(buddy->mood_phrase);
        buddy->mood_phrase = g_strdup(value);
        g_free(value);
    }

    if (xmlnode_has_prop(node, "nickname")) {
        value = xmlnode_prop(node, "nickname");
        g_free(buddy->nickname);
        buddy->nickname = g_strdup(value);
        g_free(value);
    }

    if (xmlnode_has_prop(node, "gender")) {
        value = xmlnode_prop(node, "gender");
        buddy->gender = atoi(value);
        g_free(value);
    }

    if (xmlnode_has_prop(node, "carrier-region")) {
        value = xmlnode_prop(node, "carrier-region");

        for (pos = value; *pos && *pos != '.'; pos ++);
        g_free(buddy->country);
        buddy->country = g_strndup(value, pos - value);

        for (pos ++, temp = pos; *pos && *pos != '.'; pos ++);
        g_free(buddy->province);
        buddy->province = g_strndup(temp, pos - temp);

        for (pos ++, temp = pos; *pos && *pos != '.'; pos ++);
        g_free(buddy->city);
        buddy->city = g_strndup(temp, pos - temp);

    }

    xmlnode_free(node);

    return buddy;

get_info_error:
    hybrid_debug_error("fetion", "invalid get-info response");
    return NULL;
}

fetion_buddy*
fetion_buddy_find_by_userid(fetion_account *ac, const gchar *userid)
{
    GSList       *pos;
    fetion_buddy *buddy;

    g_return_val_if_fail(ac != NULL, NULL);
    g_return_val_if_fail(userid != NULL, NULL);

    for (pos = ac->buddies; pos; pos = pos->next) {
        buddy = (fetion_buddy*)pos->data;

        if (g_strcmp0(buddy->userid, userid) == 0) {
            return buddy;
        }
    }

    return NULL;
}

fetion_buddy*
fetion_buddy_find_by_sid(fetion_account *ac, const gchar *sid)
{
    GSList       *pos;
    fetion_buddy *buddy;

    g_return_val_if_fail(ac != NULL, NULL);
    g_return_val_if_fail(sid != NULL, NULL);

    for (pos = ac->buddies; pos; pos = pos->next) {
        buddy = (fetion_buddy*)pos->data;

        if (g_strcmp0(buddy->sid, sid) == 0) {
            return buddy;
        }
    }

    return NULL;
}

static gchar*
generate_subscribe_body(void)
{
    xmlnode *root;
    xmlnode *node;
    gchar   *body;
    gchar xml_raw[] = "<args></args>";

    root = xmlnode_root(xml_raw, strlen(xml_raw));

    node = xmlnode_new_child(root, "subscription");
    xmlnode_new_prop(node, "self", "v4default;mail-count");
    xmlnode_new_prop(node, "buddy", "v4default");
    xmlnode_new_prop(node, "version", "0");

    body = xmlnode_to_string(root);

    xmlnode_free(root);

    return body;
}

void
fetion_buddy_destroy(fetion_buddy *buddy)
{
    if (buddy) {
        g_free(buddy->userid);
        g_free(buddy->sipuri);
        g_free(buddy->sid);
        g_free(buddy->mobileno);
        g_free(buddy->mood_phrase);
        g_free(buddy->carrier);
        g_free(buddy->localname);
        g_free(buddy->groups);
        g_free(buddy->portrait_crc);
        g_free(buddy->country);
        g_free(buddy->province);
        g_free(buddy->city);
        g_free(buddy);
    }
}

static gint
buddy_add_cb(fetion_account *account, const gchar *sipmsg,
            fetion_transaction *trans)
{
    gint          code;
    gchar        *pos;
    gchar        *value;
    gchar        *name;
    fetion_buddy *buddy;
    HybridGroup  *group;
    HybridBuddy  *bd;
    xmlnode      *root;
    xmlnode      *node;

    hybrid_debug_info("fetion", "add buddy, recv:\n%s", sipmsg);

    if ((code = fetion_sip_get_code(sipmsg)) != 200) {

        hybrid_message_box_show(HYBRID_MESSAGE_WARNING,
                "Add buddy error. Server response with %d", code);

        return HYBRID_ERROR;
    }

    if (!(pos = strstr(sipmsg, "\r\n\r\n"))) {
        goto add_buddy_unknown_err;
    }

    pos += 4;

    if (!(root = xmlnode_root(pos, strlen(pos)))) {
        goto add_buddy_unknown_err;
    }

    if (!(node = xmlnode_find(root, "buddy"))) {

        xmlnode_free(root);

        goto add_buddy_unknown_err;
    }

    if (xmlnode_has_prop(node, "status-code")) {

        value = xmlnode_prop(node, "status-code");

        code = atoi(value);

        g_free(value);

        if (code == 200) {
            goto add_buddy_ok;
        }

        xmlnode_free(node);

        if (code == 521) {

            hybrid_message_box_show(HYBRID_MESSAGE_WARNING,
                    "The buddy has already been in your buddy list,\n"
                    "Please don't add it duplicately.");

            return HYBRID_ERROR;
        }

        if (code == 404) {

            hybrid_message_box_show(HYBRID_MESSAGE_WARNING,
                    "The buddy you try to add doesn't exist.");

            return HYBRID_ERROR;
        }

        if (code == 486) {

            hybrid_message_box_show(HYBRID_MESSAGE_WARNING,
                    "You have reached the daily limit of adding buddies,\n"
                    "please try another day.");

            return HYBRID_ERROR;
        }

        goto add_buddy_unknown_err;
    }

add_buddy_ok:

    if (!xmlnode_has_prop(node, "user-id") ||
        !xmlnode_has_prop(node, "local-name") ||
        !xmlnode_has_prop(node, "uri") ||
        !xmlnode_has_prop(node, "buddy-lists")) {

        xmlnode_free(root);

        goto add_buddy_unknown_err;
    }

    buddy = fetion_buddy_create();

    buddy->userid    = xmlnode_prop(node, "user-id");
    buddy->localname = xmlnode_prop(node, "local-name");
    buddy->sipuri    = xmlnode_prop(node, "uri");
    buddy->groups    = xmlnode_prop(node, "buddy-lists");

    xmlnode_free(root);

    account->buddies = g_slist_append(account->buddies, buddy);

    if (!(group = hybrid_blist_find_group(account->account, buddy->groups))) {
        fetion_buddy_destroy(buddy);
        account->buddies = g_slist_remove(account->buddies, buddy);

        goto add_buddy_unknown_err;
    }

    if (buddy->localname && *(buddy->localname) != '\0')
	    name = g_strdup(buddy->localname);
    else if(buddy->nickname && *(buddy->nickname) != '\0')
	    name = g_strdup(buddy->nickname);
    else
	    name = get_sid_from_sipuri(buddy->sipuri);

    bd = hybrid_blist_add_buddy(account->account, group, buddy->userid, name);
    hybrid_blist_set_buddy_status(bd, FALSE);

    g_free(name);

    return HYBRID_OK;

add_buddy_unknown_err:
    hybrid_message_box_show(HYBRID_MESSAGE_WARNING,
            "Add buddy error. Unknown reason.");

    return HYBRID_ERROR;
}

gint
fetion_buddy_add(fetion_account *account, const gchar *groupid,
                    const gchar *no, const gchar *alias)
{
    fetion_sip         *sip;
    sip_header         *eheader;
    fetion_transaction *trans;
    gchar              *body;
    gchar              *sip_text;

    g_return_val_if_fail(account != NULL, HYBRID_ERROR);
    g_return_val_if_fail(groupid != NULL, HYBRID_ERROR);
    g_return_val_if_fail(no != NULL, HYBRID_ERROR);

    sip = account->sip;

    fetion_sip_set_type(sip, SIP_SERVICE);
    eheader = sip_event_header_create(SIP_EVENT_ADDBUDDY);
    fetion_sip_add_header(sip, eheader);

    trans = transaction_create();
    transaction_set_callid(trans, sip->callid);
    transaction_set_callback(trans, buddy_add_cb);
    transaction_add(account, trans);

    body = generate_buddy_add_body(no, groupid, alias, account->nickname);
    sip_text = fetion_sip_to_string(sip, body);
    g_free(body);

    hybrid_debug_info("fetion", "add buddy,send:\n%s", sip_text);

    if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {

        hybrid_debug_error("fetion", "add buddy failed");

        return HYBRID_ERROR;
    }

    g_free(sip_text);

    return HYBRID_OK;
}

void
fetion_buddies_init(fetion_account *ac)
{
    GSList       *pos;
    gchar        *start, *stop, *id;
    fetion_buddy *buddy;
    HybridGroup  *imgroup;
    HybridBuddy  *imbuddy;

    for (pos = ac->buddies; pos; pos = pos->next) {
        buddy = (fetion_buddy*)pos->data;

        start = buddy->groups;
        stop  = buddy->groups;

        while (*stop) {
            for (; *stop && *stop != ';'; stop ++);
            id = g_strndup(start, stop - start);

            imgroup = hybrid_blist_find_group(ac->account, id);
	    if (imgroup == NULL)
	    {
		    start =  ++stop;
		    for (; *stop && *stop != ';'; stop ++);
		    id = g_strndup(start, stop - start);

		    imgroup = hybrid_blist_find_group(ac->account, id);
	    }

            imbuddy = hybrid_blist_add_buddy(ac->account, imgroup,
                    buddy->userid, buddy->localname ? buddy->localname : buddy->nickname);

            if (*(imbuddy->name) == '\0') {
                hybrid_blist_set_buddy_name(imbuddy, buddy->sid);
            }

            hybrid_blist_set_buddy_status(imbuddy, buddy->status == 1 ? TRUE : FALSE);

            g_free(id);

            if (*stop) {
                stop ++;
                start = stop;

            } else {
                break;
            }
        }
    }
}

/**
 * Callback function to handle the portrait receive event.
 */
static gboolean
portrait_recv_cb(gint sk, gpointer user_data)
{
    gchar           buf[BUF_LENGTH];
    gint            n;
    gchar          *pos;
    HybridBuddy    *imbuddy;
    portrait_trans *trans = (portrait_trans*)user_data;

    if ((n = recv(sk, buf, sizeof(buf), 0)) == -1) {
        hybrid_debug_error("fetion", "get portrait for \'%s\':%s",
                trans->buddy ? trans->buddy->sid : trans->ac->sid,
                strerror(errno));
        return FALSE;
    }

    buf[n] = '\0';

    if (n == 0) {

        if (trans->portrait_type == PORTRAIT_TYPE_BUDDY) {
            imbuddy = hybrid_blist_find_buddy(trans->ac->account,
                    trans->buddy->userid);
        }

        if (hybrid_get_http_code(trans->data) != 200) {
            /*
             * Note that we got no portrait, but we still need
             * to set buddy icon, just for the portrait checksum, we
             * set it default to "0" instead of leaving it NULL,
             * so that in the next login, we just check the changes
             * of the buddy's checksum to determine whether to fetch a
             * portrait from the server.
             */
            if (trans->portrait_type == PORTRAIT_TYPE_BUDDY) {
                hybrid_blist_set_buddy_icon(imbuddy, NULL, 0,
                        trans->buddy->portrait_crc);

            } else {
                hybrid_account_set_icon(trans->ac->account, NULL,
                        0, trans->ac->portrait_crc);
            }

            goto pt_fin;
        }

        trans->data_len = hybrid_get_http_length(trans->data);

        if (!(pos = strstr(trans->data, "\r\n\r\n"))) {
            goto pt_fin;
        }

        pos += 4;

        if (trans->portrait_type == PORTRAIT_TYPE_BUDDY) { /**< buddy portrait */
            hybrid_blist_set_buddy_icon(imbuddy, (guchar*)pos,
                    trans->data_len, trans->buddy->portrait_crc);

        } else {
            hybrid_account_set_icon(trans->ac->account, (guchar*)pos,
                    trans->data_len, trans->ac->portrait_crc);
        }

        goto pt_fin;

    } else {
        trans->data = realloc(trans->data, trans->data_size + n);
        memcpy(trans->data + trans->data_size, buf, n);
        trans->data_size += n;
    }

    return TRUE;

pt_fin:
    g_free(trans->data);
    g_free(trans);

    return FALSE;
}

gboolean
portrait_conn_cb(gint sk, gpointer user_data)
{
    portrait_data  *data = (portrait_data*)user_data;
    portrait_trans *trans;
    gchar          *http_string;
    gchar          *encoded_sipuri;
    gchar          *encoded_ssic;
    gchar          *uri;

    trans                = g_new0(portrait_trans, 1);
    trans->buddy         = data->buddy;
    trans->ac            = data->ac;
    trans->portrait_type = data->portrait_type;

    g_free(data);

    if (trans->portrait_type == PORTRAIT_TYPE_BUDDY) {
        encoded_sipuri = g_uri_escape_string(trans->buddy->sipuri, NULL, TRUE);

    } else {
        encoded_sipuri = g_uri_escape_string(trans->ac->sipuri, NULL, TRUE);
    }

    encoded_ssic = g_uri_escape_string(trans->ac->ssic, NULL, TRUE);

    uri = g_strdup_printf("/%s/getportrait.aspx", trans->ac->portrait_host_path);

    http_string = g_strdup_printf("GET %s?Uri=%s"
              "&Size=120&c=%s HTTP/1.1\r\n"
              "User-Agent: IIC2.0/PC "PROTO_VERSION"\r\n"
              "Accept: image/pjpeg;image/jpeg;image/bmp;"
              "image/x-windows-bmp;image/png;image/gif\r\n"
              "Host: %s\r\nConnection: Close\r\n\r\n",
              uri, encoded_sipuri, encoded_ssic,
              trans->ac->portrait_host_name);

    g_free(encoded_sipuri);
    g_free(encoded_ssic);
    g_free(uri);

    if (send(sk, http_string, strlen(http_string), 0) == -1) {
        hybrid_debug_error("fetion", "download portrait for \'%s\':%s",
                trans->buddy->sid, strerror(errno));
        g_free(http_string);
        return FALSE;
    }

    g_free(http_string);

    hybrid_event_add(sk, HYBRID_EVENT_READ, portrait_recv_cb, trans);

    return FALSE;
}

void
fetion_update_portrait(fetion_account *ac, fetion_buddy *buddy)
{
    portrait_data *data;
    HybridBuddy   *hybrid_buddy;
    const gchar   *checksum;

    g_return_if_fail(ac != NULL);
    g_return_if_fail(buddy != NULL);

    data                = g_new0(portrait_data, 1);
    data->buddy         = buddy;
    data->ac            = ac;
    data->portrait_type = PORTRAIT_TYPE_BUDDY;

    if (!(hybrid_buddy = hybrid_blist_find_buddy(ac->account, buddy->userid))) {
        hybrid_debug_error("fetion", "FATAL, update portrait,"
                " unable to find a buddy.");
        return;
    }

    checksum = hybrid_blist_get_buddy_checksum(hybrid_buddy);

    if (checksum != NULL && g_strcmp0(checksum, buddy->portrait_crc) == 0) {
        hybrid_debug_info("fetion", "portrait for %s(%s) up to date",
        buddy->nickname && *(buddy->nickname) != '\0' ? buddy->nickname : buddy->userid,
        buddy->portrait_crc);
        return;
    }

    hybrid_proxy_connect(ac->portrait_host_name, 80, portrait_conn_cb, data);
}

static gint
handle_request_cb(fetion_account *account, const gchar *sipmsg,
            fetion_transaction *trans)
{
    gchar        *pos;
    gchar        *value;
    fetion_buddy *buddy;
    xmlnode      *root;
    xmlnode      *node;
    HybridGroup  *group;
    HybridBuddy  *bd;
    gchar        *name;

    if (!(pos = strstr(sipmsg, "\r\n\r\n"))) {
        return HYBRID_ERROR;
    }

    pos += 4;

    if (!(root = xmlnode_root(pos, strlen(pos)))) {
        return HYBRID_ERROR;
    }

    if (!(node = xmlnode_find(root, "buddy"))) {
        return HYBRID_ERROR;
    }

    if (!xmlnode_has_prop(node, "uri") ||
        !xmlnode_has_prop(node, "user-id")) {
        return HYBRID_ERROR;
    }

    buddy         = fetion_buddy_create();
    buddy->sipuri = xmlnode_prop(node, "uri");
    buddy->userid = xmlnode_prop(node, "user-id");
    buddy->sid    = get_sid_from_sipuri(buddy->sipuri);

    account->buddies = g_slist_append(account->buddies, buddy);

    if (xmlnode_has_prop(node, "local-name")) {
        buddy->localname = xmlnode_prop(node, "localname");
    }

    if (xmlnode_has_prop(node, "buddy-lists")) {
        buddy->groups = xmlnode_prop(node, "buddy-lists");

    } else {
        buddy->groups = "0";
    }

    if (xmlnode_has_prop(node, "relation-status")) {
        value = xmlnode_prop(node, "relation-status");
        buddy->status = atoi(value);
        g_free(value);

    } else {
        buddy->status = 0;
    }

    if (!(group = hybrid_blist_find_group(account->account, buddy->groups))) {
        account->buddies = g_slist_remove(account->buddies, buddy);
        fetion_buddy_destroy(buddy);
        return HYBRID_ERROR;
    }

    if (buddy->localname && *(buddy->localname) == '\0') {
        name = get_sid_from_sipuri(buddy->sipuri);

    } else {
        name = g_strdup(buddy->localname);
    }

    bd = hybrid_blist_add_buddy(account->account, group, buddy->userid, name);
    hybrid_blist_set_buddy_status(bd, buddy->status == 1 ? TRUE: FALSE);

    g_free(name);

    return HYBRID_OK;
}

void
fetion_buddy_handle_request(fetion_account *ac, const gchar *sipuri,
                            const gchar *userid, const gchar *alias, const gchar *groupid,
                            gboolean accept)
{
    fetion_sip         *sip;
    sip_header         *eheader;
    gchar              *body;
    gchar              *sip_text;
    fetion_transaction *trans;

    g_return_if_fail(ac != NULL);
    g_return_if_fail(sipuri != NULL);
    g_return_if_fail(userid != NULL);
    g_return_if_fail(groupid != NULL);

    sip = ac->sip;

    fetion_sip_set_type(sip, SIP_SERVICE);
    eheader = sip_event_header_create(SIP_EVENT_HANDLECONTACTREQUEST);

    if (accept) {
        trans = transaction_create();
        transaction_set_callid(trans, sip->callid);
        transaction_set_userid(trans, userid);
        transaction_set_callback(trans, handle_request_cb);
        transaction_add(ac, trans);
    }

    fetion_sip_add_header(sip, eheader);
    body = generate_handle_request_body(sipuri, userid, alias, groupid, accept);

    sip_text = fetion_sip_to_string(sip, body);
    g_free(body);

    if (send(ac->sk, sip_text, strlen(sip_text), 0) == -1) {

        hybrid_debug_error("fetion", "handle buddy request failed.");
        g_free(sip_text);

        return;
    }

    g_free(sip_text);
}

static gchar*
generate_get_info_body(const gchar *userid)
{
    xmlnode *root;
    xmlnode *node;
    gchar   *res;

    gchar body[] = "<args></args>";

    root = xmlnode_root(body, strlen(body));
    node = xmlnode_new_child(root, "contact");
    xmlnode_new_prop(node, "user-id", userid);

    res = xmlnode_to_string(root);

    xmlnode_free(root);

    return res;
}

static gchar*
generate_buddy_move_body(const gchar *userid, const gchar *groupid)
{
    const gchar *body;
    xmlnode     *root;
    xmlnode     *node;
    gchar       *res;

    body = "<args></args>";

    root = xmlnode_root(body, strlen(body));

    node = xmlnode_new_child(root, "contacts");
    node = xmlnode_new_child(node, "contact");

    xmlnode_new_prop(node, "user-id", userid);
    xmlnode_new_prop(node, "buddy-lists", groupid);

    res = xmlnode_to_string(root);

    xmlnode_free(root);

    return res;
}

static gchar*
generate_remove_buddy_body(const gchar *userid)
{
    const gchar *body;
    xmlnode     *root;
    xmlnode     *node;
    gchar       *res;

    body = "<args></args>";

    root = xmlnode_root(body, strlen(body));

    node = xmlnode_new_child(root, "contacts");
    node = xmlnode_new_child(node, "buddies");
    node = xmlnode_new_child(node, "buddy");
    xmlnode_new_prop(node, "user-id", userid);

    res = xmlnode_to_string(root);

    xmlnode_free(root);

    return res;
}


static gchar*
generate_rename_buddy_body(const gchar *userid, const gchar *name)
{
    const gchar *body;
    xmlnode     *root;
    xmlnode     *node;
    gchar       *res;

    body = "<args></args>";

    root = xmlnode_root(body, strlen(body));

    node = xmlnode_new_child(root, "contacts");
    node = xmlnode_new_child(node, "contact");
    xmlnode_new_prop(node, "user-id", userid);
    xmlnode_new_prop(node, "local-name", name);

    res = xmlnode_to_string(root);

    xmlnode_free(root);

    return res;
}

static gchar*
generate_buddy_add_body(const gchar *no, const gchar *groupid,
                        const gchar *localname, const gchar *desc)
{
    const gchar *body;
    gchar       *sipuri;
    xmlnode     *root;
    xmlnode     *node;
    gchar       *res;

    body = "<args></args>";

    root = xmlnode_root(body, strlen(body));

    node = xmlnode_new_child(root, "contacts");
    node = xmlnode_new_child(node, "buddies");
    node = xmlnode_new_child(node, "buddy");

    if (strlen(no) < 11) {
        sipuri = g_strdup_printf("sip:%s", no);

    } else {
        sipuri = g_strdup_printf("tel:%s", no);
    }

    xmlnode_new_prop(node, "uri", sipuri);
    xmlnode_new_prop(node, "local-name", localname);
    xmlnode_new_prop(node, "buddy-lists", groupid);
    xmlnode_new_prop(node, "desc", desc);
    xmlnode_new_prop(node, "expose-mobile-no", "1");
    xmlnode_new_prop(node, "expose-name", "1");
    xmlnode_new_prop(node, "addbuddy-phrase-id", "0");

    g_free(sipuri);

    res = xmlnode_to_string(root);

    xmlnode_free(root);

    return res;
}

static gchar*
generate_handle_request_body(const gchar *sipuri, const gchar *userid,
                             const gchar *alias, const gchar *groupid,
                             gboolean accept)
{
    xmlnode *root;
    xmlnode *node;
    gchar   *res;

    root = xmlnode_create("args");
    node = xmlnode_new_child(root, "contacts");
    node = xmlnode_new_child(node, "buddies");
    node = xmlnode_new_child(node, "buddy");

    xmlnode_new_prop(node, "user-id", userid);
    xmlnode_new_prop(node, "uri", sipuri);
    xmlnode_new_prop(node, "result", accept ? "1": "0");
    xmlnode_new_prop(node, "buddy-lists", groupid);
    xmlnode_new_prop(node, "expose-mobile-no", "1");
    xmlnode_new_prop(node, "expose-name", "1");
    xmlnode_new_prop(node, "local-name", alias);

    res = xmlnode_to_string(root);

    xmlnode_free(root);

    return res;
}

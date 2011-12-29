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
#include "xmlnode.h"

#include "fx_sip.h"
#include "fx_account.h"
#include "fx_buddy.h"

gint callid = 1; /**< The global call id */

fetion_sip*
fetion_sip_create(fetion_account *ac)
{
    g_return_val_if_fail(ac != NULL, NULL);

    fetion_sip *sip = g_new0(fetion_sip, 1);

    if (ac->sid) {
        sip->from = g_strdup(ac->sid);
    }

    sip->header = NULL;
    sip->sequence = 2;
    ac->sip = sip;

    return sip;
}

void
fetion_sip_set_type(fetion_sip *sip, gint sip_type)
{
    sip->type = sip_type;
    sip->callid = callid;
}

void
fetion_sip_set_from(fetion_sip *sip, const gchar *from)
{
    g_return_if_fail(sip != NULL);
    g_return_if_fail(from != NULL);

    if (sip->from) {
        g_free(sip->from);
    }

    sip->from = g_strdup(from);
}

gint
fetion_sip_get_msg_type(const gchar *sipmsg)
{
    gchar *res;
    gchar *pos;
    gint t;

    for (pos = (char*)sipmsg; *pos && *pos != ' '; pos ++);

    res = g_strndup(sipmsg, pos - sipmsg);

    if (strcmp(res, "I") == 0) {
        t = SIP_INVITATION;

    } else if (strcmp(res, "M") == 0) {
        t =  SIP_MESSAGE;

    } else if (strcmp(res, "BN") == 0) {
        t =  SIP_NOTIFICATION;

    } else if (strcmp(res, "SIP-C/4.0") == 0 ||
            strcmp(res , "SIP-C/1.0") == 0) {
        t = SIP_SIPC_4_0;

    } else if (strcmp(res, "IN") == 0) {
        t = SIP_INFO;

    } else if (strcmp(res, "O") == 0) {
        t =  SIP_OPTION;

    } else {
        t = SIP_UNKNOWN;
    }

    g_free(res);

    return t;
}

gint
fetion_sip_get_code(const gchar *sipmsg)
{
    gchar *pos;
    gchar *stop;
    gchar *temp;
    gint   res;

    g_return_val_if_fail(sipmsg != NULL, 0);

    for (pos = (gchar*)sipmsg; *pos && *pos != ' '; pos ++);

    if (*pos == '\0') {
        return 0;
    }

    pos ++;

    for (stop = pos; *stop && *stop != ' '; stop ++);

    if (*stop == '\0') {
        return 0;
    }

    temp = g_strndup(pos, stop - pos);
    res = atoi(temp);
    g_free(temp);

    return res;
}

gint
fetion_sip_get_length(const gchar *sipmsg)
{
    gint   length;
    gchar *temp;

    g_return_val_if_fail(sipmsg != NULL, 0);

    if(!(temp = sip_header_get_attr(sipmsg, "L"))) {
        return 0;
    }

    length = atoi(temp);
    g_free(temp);

    return length;
}

gchar*
get_sid_from_sipuri(const gchar *sipuri)
{
    gchar *pos, *pos1;
    gchar *res;

    g_return_val_if_fail(sipuri != NULL, NULL);

    for (pos = (gchar*)sipuri; *pos && *pos != ':'; pos ++);

    if (*pos == '\0') {
        return NULL;
    }

    pos ++;

    for (pos1 = pos; *pos1 && *pos1 != '@'; pos1 ++);

    res = g_strndup(pos, pos1 - pos);

    return res;
}

void
fetion_sip_destroy(fetion_sip *sip)
{
    if (sip) {
        g_free(sip->from);
        g_free(sip);
    }
}

gchar*
fetion_sip_to_string(fetion_sip *sip, const gchar *body)
{
    gchar      *type;
    gchar      *res;
    gchar      *header_value;
    gchar      *to_free;
    sip_header *pos;
    sip_header *temp;

    g_return_val_if_fail(sip != NULL, NULL);

    switch (sip->type) {

    case SIP_REGISTER:
      type = g_strdup("R");
      break;
    case SIP_SUBSCRIPTION:
      type = g_strdup("SUB");
      break;
    case SIP_SERVICE:
      type = g_strdup("S");
      break;
    case SIP_MESSAGE:
      type = g_strdup("M");
      break;
    case SIP_INFO:
      type = g_strdup("IN");
      break;
    case SIP_OPTION:
      type = g_strdup("O");
      break;
    case SIP_INVITATION:
      type = g_strdup("I");
      break;
    case SIP_ACKNOWLEDGE:
      type = g_strdup("A");
      break;
    default:
      type = NULL;
      break;
    };

    if(! type) {
        return NULL;
    }

    res = g_strdup_printf("%s fetion.com.cn SIP-C/4.0\r\n"
                          "F: %s\r\n"
                          "I: %d\r\n"
                          "Q: 2 %s\r\n",
                          type, sip->from, sip->callid, type);

    g_free(type);

    pos = sip->header;

    while (pos) {

        to_free = res;

        header_value = g_strdup_printf("%s: %s\r\n", pos->name, pos->value);
        res = g_strjoin("", res, header_value, NULL);

        g_free(to_free);
        g_free(header_value);

        temp = pos;
        pos  = pos->next;

        sip_header_destroy(temp);
    }

    if (body) {
        to_free = res;
        header_value = g_strdup_printf("L: %d\r\n\r\n", strlen(body));
        res = g_strjoin("", res, header_value, body, NULL);
        g_free(to_free);
        g_free(header_value);

    } else {
        to_free = res;
        res = g_strjoin("", res, "\r\n", NULL);
        g_free(to_free);
    }

    callid ++;
    sip->header = NULL;

    return res;
}

void
fetion_sip_add_header(fetion_sip *sip, sip_header *header)
{
    sip_header *pos = sip->header;

    if (!pos) {
        sip->header = header;
        return;
    }

    while (pos) {

        if(!pos->next) {
            pos->next = header;
            break;
        }

        pos = pos->next;
    }
}

sip_header*
sip_authentication_header_create(const gchar *response)
{
    gchar      *value;
    gchar start[] = "Digest response=\"";
    gchar end[]   = "\",algorithm=\"SHA1-sess-v4\"";
    sip_header *header;

    value = g_strjoin("", start, response, end, NULL);
    header = sip_header_create("A", value);
    g_free(value);

    return header;
}

sip_header*
sip_ack_header_create(const gchar *code, const gchar *algorithm, const gchar *type,
                      const gchar *guid)
{
    gchar ack[BUF_LENGTH];
    g_snprintf(ack, sizeof(ack) - 1, "Verify response=\"%s\",algorithm=\"%s\","
               "type=\"%s\",chid=\"%s\"", code, algorithm, type, guid);

    return sip_header_create("A" , ack);
}

sip_header*
sip_event_header_create(gint event_type)
{
    gchar      *event = NULL;
    sip_header *header;

    g_return_val_if_fail(event_type >= SIP_EVENT_PRESENCE, NULL);
    g_return_val_if_fail(event_type <= SIP_EVENT_PGPRESENCE, NULL);

    switch (event_type) {
    case SIP_EVENT_PRESENCE :
      event = g_strdup("PresenceV4");
      break;
    case SIP_EVENT_SETPRESENCE :
      event = g_strdup("SetPresenceV4");
      break;
    case SIP_EVENT_CATMESSAGE :
      event = g_strdup("CatMsg");
      break;
    case SIP_EVENT_SENDCATMESSAGE :
      event = g_strdup("SendCatSMS");
      break;
    case SIP_EVENT_STARTCHAT :
      event = g_strdup("StartChat");
      break;
    case SIP_EVENT_GETCONTACTINFO :
      event = g_strdup("GetContactInfoV4");
      break;
    case SIP_EVENT_CONVERSATION :
      event = g_strdup("Conversation");
      break;
    case SIP_EVENT_INVITEBUDDY :
      event = g_strdup("InviteBuddy");
      break;
    case SIP_EVENT_CREATEBUDDYLIST :
      event = g_strdup("CreateBuddyList");
      break;
    case SIP_EVENT_DELETEBUDDYLIST :
      event = g_strdup("DeleteBuddyList");
      break;
    case SIP_EVENT_SETCONTACTINFO :
      event = g_strdup("SetContactInfoV4");
      break;
    case SIP_EVENT_SETUSERINFO :
      event = g_strdup("SetUserInfoV4");
      break;
    case SIP_EVENT_SETBUDDYLISTINFO :
      event = g_strdup("SetBuddyListInfo");
      break;
    case SIP_EVENT_DELETEBUDDY :
      event = g_strdup("DeleteBuddyV4");
      break;
    case SIP_EVENT_ADDBUDDY :
      event = g_strdup("AddBuddyV4");
      break;
    case SIP_EVENT_KEEPALIVE :
      event = g_strdup("KeepAlive");
      break;
    case SIP_EVENT_DIRECTSMS :
      event = g_strdup("DirectSMS");
      break;
    case SIP_EVENT_HANDLECONTACTREQUEST :
      event = g_strdup("HandleContactRequestV4");
      break;
    case SIP_EVENT_SENDDIRECTCATSMS :
      event = g_strdup("SendDirectCatSMS");
      break;
    case SIP_EVENT_PGGETGROUPLIST:
      event = g_strdup("PGGetGroupList");
      break;
    case SIP_EVENT_PGGETGROUPINFO:
      event = g_strdup("PGGetGroupInfo");
      break;
    case SIP_EVENT_PGPRESENCE:
      event = g_strdup("PGPresence");
      break;
    case SIP_EVENT_PGGETGROUPMEMBERS:
      event = g_strdup("PGGetGroupMembers");
      break;
    case SIP_EVENT_PGSENDCATSMS:
      event = g_strdup("PGSendCatSMS");
      break;
    default:
      break;
    }

    header = sip_header_create("N", event);

    g_free(event);

    return header;
}

sip_header*
sip_credential_header_create(const gchar *credential)
{
    gchar value[BUF_LENGTH];

    g_return_val_if_fail(credential != NULL, NULL);

    g_snprintf(value, sizeof(value) - 1, "TICKS auth=\"%s\"", credential);

    return sip_header_create("A", value);
}

sip_header*
sip_header_create(const gchar *name, const gchar *value)
{
    sip_header *header;

    g_return_val_if_fail(name != NULL, NULL);
    g_return_val_if_fail(value != NULL, NULL);

    header = g_new0(sip_header, 1);
    header->name = g_strdup(name);
    header->value = g_strdup(value);
    header->next = NULL;

    return header;
}

gchar*
sip_header_get_attr(const gchar *header_string, const gchar *name)
{
    gchar *iter;
    gchar *pos, *stop;

    g_return_val_if_fail(header_string != NULL, NULL);
    g_return_val_if_fail(name != NULL, NULL);

    iter = g_strjoin("", name, ": ", NULL);

    if (!(pos = strstr(header_string, iter))) {
        return NULL;
    }

    pos += strlen(iter);

    g_free(iter);

    for (stop = pos; *stop && *stop != '\r' && *stop != '\n'; stop ++);

    return g_strndup(pos, stop - pos);
}

gint
sip_header_get_auth(const gchar *header_string, gchar **ip, gint *port,
                    gchar **credential)
{
    gchar *pos;
    gchar *temp;
    gchar *port_str;

    *ip         = NULL;
    *port       = 0;
    *credential = NULL;

    g_return_val_if_fail(header_string != NULL, HYBRID_ERROR);

    for (pos = (gchar*)header_string; *pos && *pos != '\"'; pos ++);

    if (*pos == '\0') {
        return HYBRID_ERROR;
    }

    pos ++;

    for (temp = pos; *pos && *pos != ':'; pos ++);

    if (*pos =='\0') {
        return HYBRID_ERROR;
    }

    *ip = g_strndup(temp, pos - temp);

    pos ++;

    for (temp = pos; *pos && *pos != ';'; pos ++);

    port_str = g_strndup(temp, pos - temp);
    *port = atoi(port_str);
    g_free(port_str);

    for (; *pos && *pos != '='; pos ++);
    for (; *pos && *pos != '\"'; pos ++);

    if (*pos =='\0') {
        return HYBRID_ERROR;
    }

    pos ++;

    for (temp = pos; *pos && *pos != '\"'; pos ++);

    if (*pos =='\0') {
        return HYBRID_ERROR;
    }

    *credential = g_strndup(temp, pos - temp);

    return HYBRID_OK;
}

void
sip_header_destroy(sip_header *header)
{
    if (header) {
        g_free(header->name);
        g_free(header->value);
        g_free(header);
    }
}

void
sip_parse_notify(const gchar *sipmsg, gint *notify_type, gint *event_type)
{
    gchar   *attr;
    gchar   *pos;
    gchar   *event;
    xmlnode *root = NULL;
    xmlnode *node;

    g_return_if_fail(sipmsg != NULL);

    if (!(attr = sip_header_get_attr(sipmsg, "N"))) {
        *notify_type = NOTIFICATION_TYPE_UNKNOWN;
        *event_type = NOTIFICATION_EVENT_UNKNOWN;
        return;
    }

    if (g_strcmp0(attr, "PresenceV4") == 0) {
        *notify_type = NOTIFICATION_TYPE_PRESENCE;

    } else if (g_strcmp0(attr, "Conversation") == 0) {
        *notify_type = NOTIFICATION_TYPE_CONVERSATION;

    } else if (g_strcmp0(attr, "contact") == 0) {
        *notify_type = NOTIFICATION_TYPE_CONTACT;

    } else if (g_strcmp0(attr, "registration") == 0) {
        *notify_type = NOTIFICATION_TYPE_REGISTRATION;

    } else if (g_strcmp0(attr, "SyncUserInfoV4") == 0) {
        *notify_type = NOTIFICATION_TYPE_SYNCUSERINFO;

    } else if (g_strcmp0(attr, "PGGroup") == 0) {
        *notify_type = NOTIFICATION_TYPE_PGGROUP;

    } else {
        *notify_type = NOTIFICATION_TYPE_UNKNOWN;
    }

    g_free(attr);

    if (!(pos = strstr(sipmsg, "\r\n\r\n"))) {
        goto notify_err;
    }

    pos += 4;

    if (!(root = xmlnode_root(pos, strlen(pos)))) {
        goto notify_err;
    }

    if (!(node = xmlnode_find(root, "event"))) {
        goto notify_err;
    }

    if (!(event = xmlnode_prop(node, "type"))) {
        goto notify_err;
    }

    if (g_strcmp0(event, "Support") == 0) {
        if (!(node = xmlnode_next(node))) {
            goto notify_err;
        }

        if (g_strcmp0(node->name, "event") != 0) {
            goto notify_err;
        }

        if (!(event = xmlnode_prop(node, "type"))) {
            goto notify_err;
        }

    }

    if (g_strcmp0(event, "PresenceChanged") == 0) {
        *event_type = NOTIFICATION_EVENT_PRESENCECHANGED;

    } else if (g_strcmp0(event, "UserEntered") == 0) {
        *event_type = NOTIFICATION_EVENT_USERENTER;

    } else if (g_strcmp0(event, "UserLeft") == 0) {
        *event_type = NOTIFICATION_EVENT_USERLEFT;

    } else if (g_strcmp0(event, "deregistered") == 0) {
        *event_type = NOTIFICATION_EVENT_DEREGISTRATION;

    } else if (g_strcmp0(event, "SyncUserInfo") == 0) {
        *event_type = NOTIFICATION_EVENT_SYNCUSERINFO;

    } else if (g_strcmp0(event, "AddBuddyApplication") == 0) {
        *event_type = NOTIFICATION_EVENT_ADDBUDDYAPPLICATION;

    } else if (g_strcmp0(event, "PGGetGroupInfo") == 0) {
        *event_type = NOTIFICATION_EVENT_PGGETGROUPINFO;

    } else {
        *event_type = NOTIFICATION_EVENT_UNKNOWN;
    }

    xmlnode_free(root);
    return;

notify_err:
    xmlnode_free(root);
    *event_type = NOTIFICATION_EVENT_UNKNOWN;
    return;
}

GSList*
sip_parse_sync(fetion_account *account, const gchar *sipmsg)
{
    gchar        *pos;
    gchar        *action;
    gchar        *userid;
    gchar        *status;
    xmlnode      *root;
    xmlnode      *node;
    fetion_buddy *buddy;
    GSList       *list = NULL;

    if (!(pos = strstr(sipmsg, "\r\n\r\n"))) {
        goto sync_info_err;
    }

    pos += 4;

    if (!(root = xmlnode_root(pos, strlen(pos)))) {
        goto sync_info_err;
    }

    if (!(node = xmlnode_find(root, "buddies"))) {

        xmlnode_free(root);

        return list;
    }

    node = xmlnode_child(node);

    while (node) {
        if (!xmlnode_has_prop(node, "action")) {
            goto next;
        }

        action = xmlnode_prop(node, "action");

        if (g_strcmp0(action, "update") == 0) {

            if (!xmlnode_has_prop(node, "user-id") ||
                !xmlnode_has_prop(node, "relation-status")) {

                g_free(action);

                goto next;
            }

            userid = xmlnode_prop(node, "user-id");
            status = xmlnode_prop(node, "relation-status");

            if (!(buddy = fetion_buddy_find_by_userid(account, userid))) {

                g_free(action);
                g_free(userid);
                g_free(status);

                goto next;
            }

            buddy->status = atoi(status);

            list = g_slist_append(list, buddy);

            g_free(status);
            g_free(userid);
        }

        g_free(action);
next:
        node = xmlnode_next(node);
    }

    return list;

sync_info_err:
    hybrid_debug_error("fetion", "invalid sync info");

    return list;
}

GSList*
sip_parse_presence(fetion_account *ac, const gchar *sipmsg)
{
    gchar        *pos;
    gchar        *temp;
    xmlnode      *root;
    xmlnode      *node;
    xmlnode      *pnode;
    GSList       *list = NULL;
    fetion_buddy *buddy;

    if (!(pos = strstr(sipmsg, "\r\n\r\n"))) {
        return list;
    }

    pos += 4;

    root = xmlnode_root(pos, strlen(pos));
    node = xmlnode_find(root, "contacts");
    node = xmlnode_child(node);

    while (node) {

        temp = xmlnode_prop(node, "id");

        if (!(buddy = fetion_buddy_find_by_userid(ac, temp))) {
            /* Maybe yourself's presence, we just ignore it. */
            g_free(temp);
            node = node->next;
            continue;
        }

        g_free(temp);

        list = g_slist_append(list, buddy);

        if ((pnode = xmlnode_find(node, "p"))) {

            if (xmlnode_has_prop(pnode, "m")) {
                temp = xmlnode_prop(pnode, "m");
                g_free(buddy->mobileno);
                buddy->mobileno = g_strdup(temp);
                g_free(temp);
            }

            if (xmlnode_has_prop(pnode, "n")) {
                temp = xmlnode_prop(pnode, "n");
                g_free(buddy->nickname);
                buddy->nickname = g_strdup(temp);
                g_free(temp);
            }

            if (xmlnode_has_prop(pnode, "i")) {
                temp = xmlnode_prop(pnode, "i");
                g_free(buddy->mood_phrase);
                buddy->mood_phrase = g_strdup(temp);
                g_free(temp);
            }

            if (xmlnode_has_prop(pnode, "c")) {
                temp = xmlnode_prop(pnode, "c");
                g_free(buddy->carrier);
                buddy->carrier = g_strdup(temp);
                g_free(temp);
            }

            if (xmlnode_has_prop(pnode, "p")) {
                temp = xmlnode_prop(pnode, "p");
                g_free(buddy->portrait_crc);

                if (*temp == '\0') {
                    g_free(temp);
                    temp = g_strdup("0");
                }
                buddy->portrait_crc = temp;
            } else {
                g_free(buddy->portrait_crc);
                buddy->portrait_crc = g_strdup("0");
            }

            if (xmlnode_has_prop(pnode, "cs")) {
                temp = xmlnode_prop(pnode, "cs");
                buddy->carrier_status = atoi(temp);
                g_free(temp);
            }
        }

        if ((pnode = xmlnode_find(node, "pr"))) {

            if (xmlnode_has_prop(pnode, "b")) {
                temp = xmlnode_prop(pnode, "b");
                buddy->state = atoi(temp);
                g_free(temp);
            }
        }

        node = node->next;
    }

    xmlnode_free(root);

    return list;
}

gint
sip_parse_appbuddy(const gchar *sipmsg, gchar **userid,
                   gchar **sipuri, gchar **desc)
{
    gchar   *pos;
    xmlnode *root;
    xmlnode *node;

    if (!(pos = strstr(sipmsg, "\r\n\r\n"))) {
        return HYBRID_ERROR;
    }

    pos += 4;

    if (!(root = xmlnode_root(pos, strlen(pos)))) {
        return HYBRID_ERROR;
    }

    if (!(node = xmlnode_find(root, "application"))) {
        return HYBRID_ERROR;
    }

    if (xmlnode_has_prop(node, "uri") && sipuri != NULL) {
        *sipuri = xmlnode_prop(node, "uri");
    }

    if (xmlnode_has_prop(node, "user-id") && userid != NULL) {
        *userid = xmlnode_prop(node, "user-id");
    }

    if (xmlnode_has_prop(node, "desc") && desc != NULL) {
        *desc = xmlnode_prop(node, "desc");
    }

    return HYBRID_OK;
}

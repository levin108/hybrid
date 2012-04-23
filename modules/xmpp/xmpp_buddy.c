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

#include "notify.h"

#include "xmpp_buddy.h"
#include "xmpp_iq.h"

XmppPresence*
xmpp_presence_create(const gchar *jid, const gchar *status, gint show)
{
    XmppPresence *presence;

    g_return_val_if_fail(jid != NULL, NULL);

    presence = g_new0(XmppPresence, 1);
    presence->full_jid = g_strdup(jid);
    presence->status = g_strdup(status);
    presence->show = show;

    return presence;
}

void
xmpp_presence_destroy(XmppPresence *presence)
{
    XmppBuddy *buddy;

    if (!presence) {
        return;
    }

    buddy = presence->buddy;

    buddy->presence_list = g_slist_remove(buddy->presence_list, presence);

    g_free(presence->full_jid);
    g_free(presence->status);
    g_free(presence);
}

XmppPresence*
xmpp_buddy_find_presence(XmppBuddy *buddy, const gchar *full_jid)
{
    XmppPresence *presence;
    GSList       *pos;

    g_return_val_if_fail(buddy != NULL, NULL);
    g_return_val_if_fail(full_jid != NULL, NULL);

    if (g_ascii_strncasecmp(buddy->jid, full_jid, strlen(buddy->jid)) != 0) {
        return NULL;
    }

    for (pos = buddy->presence_list; pos; pos = pos->next) {
        presence = (XmppPresence*)pos->data;

        if (g_strcmp0(presence->full_jid, full_jid) == 0) {
            return presence;
        }
    }

    return NULL;
}

/**
 * Scribe the presence information of the roster by
 * sending a <presence/> label to the server.
 */
static void
xmpp_buddy_presence(XmppStream *stream)
{
    xmlnode       *node;
    gchar         *xml_string;
    HybridAccount *account;

    g_return_if_fail(stream != NULL);

    account = stream->account->account;

    node = xmlnode_create("presence");

    xml_string = xmlnode_to_string(node);

    hybrid_debug_info("xmpp", "subscribe presence,send:\n%s", xml_string);

    if (hybrid_ssl_write(stream->ssl, xml_string, strlen(xml_string)) == -1) {

        hybrid_account_error_reason(stream->account->account,
                                    _("Failed to subscribe presence"));
        g_free(xml_string);

        return;
    }

    g_free(xml_string);

    xmpp_account_modify_status(stream, account->state,
                               account->status_text);
}

void
xmpp_buddy_process_roster(XmppStream *stream, xmlnode *root)
{
    gchar         *value;
    gchar         *jid, *name, *scribe;
    gchar         *group_name;
    xmlnode       *node;
    xmlnode       *group_node;
    xmlnode       *item_nodes;
    HybridGroup   *group;
    HybridAccount *account;
    HybridBuddy   *hd;
    XmppBuddy     *buddy;

    g_return_if_fail(stream != NULL);
    g_return_if_fail(root != NULL);

    account = stream->account->account;

    if (!xmlnode_has_prop(root, "type")) {
        goto roster_err;
    }

    value = xmlnode_prop(root, "type");
    if (g_strcmp0(value, "result") != 0) {
        goto roster_err;
    }
    g_free(value);

    if (!(node = xmlnode_find(root, "query"))) {
        goto roster_err;
    }

    item_nodes = xmlnode_child(node);

    for (node = item_nodes; node; node = node->next) {

        jid = xmlnode_prop(node, "jid");

        /*
         * If the item has no property named 'name', then we make the
         * prefix name of the jid as the buddy's name as default.
         */
        if (xmlnode_has_prop(node, "name")) {
            name   = xmlnode_prop(node, "name");

        } else {
            name = NULL;
        }
        scribe = xmlnode_prop(node, "subscription");

        /*
         * If the presence node doesn't has a group node,
         * so we make it belongs to default group 'Buddies'.
         */
        if (!(group_node = xmlnode_find(node, "group"))) {
            group_name = g_strdup(_("Buddies"));

        } else {
            group_name = xmlnode_content(group_node);
        }

        /* add this buddy's group to the buddy list. */
        group = hybrid_blist_add_group(account, group_name, group_name);
        g_free(group_name);

        /* add this buddy to the buddy list. */
        hd = hybrid_blist_add_buddy(account, group, jid, name);
        g_free(jid);

        /*
         * Maybe the buddy with the specified jabber id already exists,
         * then hybrid_blist_add_buddy() will not set the buddy name even
         * if the name has changed, so we must set the name manually by
         * hybrid_blist_set_buddy_name() in case that the name changed.
         */
        buddy = xmpp_buddy_create(stream, hd);
        buddy->subscription = g_strdup(scribe);

        xmpp_buddy_set_name(buddy, name);
        g_free(name);
    }

    /* subsribe the presence of the roster. */
    xmpp_buddy_presence(stream);

    return;

roster_err:
    hybrid_account_error_reason(stream->account->account,
                                _("Failed to request roster."));
}

XmppBuddy*
xmpp_buddy_create(XmppStream *stream, HybridBuddy *hybrid_buddy)
{
    XmppBuddy   *buddy;
    XmppAccount *account;
    gchar       *pos;

    g_return_val_if_fail(hybrid_buddy != NULL, NULL);

    account = stream->account;

    if (!account->buddies) {
        account->buddies = g_hash_table_new_full(g_str_hash, g_str_equal,
                NULL, (GDestroyNotify)xmpp_buddy_destroy);
    }

    if ((buddy = g_hash_table_lookup(account->buddies, hybrid_buddy->id))) {
        return buddy;
    }

    for (pos = hybrid_buddy->id; *pos; pos ++) {
        *pos = g_ascii_tolower(*pos);
    }

    buddy = g_new0(XmppBuddy, 1);
    buddy->jid = g_strdup(hybrid_buddy->id);
    buddy->buddy = hybrid_buddy;
    buddy->stream = stream;
    buddy->name = g_strdup(hybrid_buddy->name);

    xmpp_buddy_set_group_name(buddy, hybrid_buddy->parent->name);
    xmpp_buddy_set_photo(buddy, hybrid_blist_get_buddy_checksum(hybrid_buddy));

    g_hash_table_insert(account->buddies, buddy->jid, buddy);

    return buddy;
}

void
xmpp_buddy_set_name(XmppBuddy *buddy, const gchar *name)
{
    gchar *tmp;
    gchar *pos;

    g_return_if_fail(buddy != NULL);

    tmp         = buddy->name;
    buddy->name = g_strdup(name);
    g_free(tmp);

    if (!buddy->name) {
        for (pos = buddy->jid; *pos && *pos != '@'; pos ++);
        buddy->name = g_strndup(buddy->jid, pos - buddy->jid);
    }

    hybrid_blist_set_buddy_name(buddy->buddy, buddy->name);
}

void
xmpp_buddy_set_subscription(XmppBuddy *buddy, const gchar *sub)
{
    gchar *tmp;

    g_return_if_fail(buddy != NULL);

    if (g_strcmp0(buddy->subscription, sub) != 0) {

        if (g_strcmp0(sub, "both") == 0) {
            hybrid_blist_set_buddy_status(buddy->buddy, TRUE);

        } else {
            hybrid_blist_set_buddy_status(buddy->buddy, FALSE);
        }
    }

    tmp = buddy->subscription;
    buddy->subscription = g_strdup(sub);
    g_free(tmp);
}

void
xmpp_buddy_set_photo(XmppBuddy *buddy, const gchar *photo)
{
    gchar *tmp;

    g_return_if_fail(buddy != NULL);

    tmp = buddy->photo;
    buddy->photo = g_strdup(photo);
    g_free(tmp);
}

void
xmpp_buddy_set_status(XmppBuddy *buddy, const gchar *jid, const gchar *status)
{
    gchar        *tmp;
    XmppPresence *presence;

    g_return_if_fail(buddy != NULL);

    if (!(presence = xmpp_buddy_find_presence(buddy, jid))) {
        presence = xmpp_presence_create(jid, status, HYBRID_STATE_ONLINE);
        xmpp_buddy_add_presence(buddy, presence);

    } else {

        tmp = presence->status;
        presence->status = g_strdup(status);
        g_free(tmp);
    }

    hybrid_blist_set_buddy_mood(buddy->buddy, status);
}

void
xmpp_buddy_set_show(XmppBuddy *buddy, const gchar *full_jid, const gchar *show)
{
    gint          state = 0;
    XmppPresence *presence;

    g_return_if_fail(buddy != NULL);
    g_return_if_fail(show != NULL);

    if (g_ascii_strcasecmp(show, "away") == 0) {
        state = HYBRID_STATE_AWAY;

    } else if (g_ascii_strcasecmp(show, "avaiable") == 0) {
        state = HYBRID_STATE_ONLINE;

    } else if (g_ascii_strcasecmp(show, "dnd") == 0) {
        state = HYBRID_STATE_BUSY;

    } else if (g_ascii_strcasecmp(show, "unavailable") == 0) {
        state = HYBRID_STATE_OFFLINE;

    } else {
        state = HYBRID_STATE_ONLINE;
    }

    if (!(presence = xmpp_buddy_find_presence(buddy, full_jid))) {
        if (state != HYBRID_STATE_OFFLINE) {
            presence = xmpp_presence_create(full_jid, NULL, state);
            xmpp_buddy_add_presence(buddy, presence);

        } else {
            hybrid_debug_error("xmpp", "FATAL, received a unavailable presence"
                    " that didn't exist in the buddy's presence list.");
            return;
        }
    }

    if (state == HYBRID_STATE_OFFLINE) {
        xmpp_presence_destroy(presence);

        if (!buddy->presence_list) {
            hybrid_blist_set_buddy_state(buddy->buddy, state);

        } else {
            presence = (XmppPresence*)buddy->presence_list->data;
            hybrid_blist_set_buddy_state(buddy->buddy, presence->show);
            hybrid_blist_set_buddy_mood(buddy->buddy, presence->status);
        }

    } else {
        hybrid_blist_set_buddy_state(buddy->buddy, state);
    }
}

void
xmpp_buddy_set_group_name(XmppBuddy *buddy, const gchar *group)
{
    gchar *tmp;

    g_return_if_fail(buddy != NULL);

    tmp = buddy->group;
    buddy->group = g_strdup(group);
    g_free(tmp);
}

void
xmpp_buddy_add_presence(XmppBuddy *buddy, XmppPresence *presence)
{
    g_return_if_fail(buddy != NULL);
    g_return_if_fail(presence != NULL);

    buddy->presence_list = g_slist_append(buddy->presence_list, presence);
    presence->buddy = buddy;
}

gint
xmpp_buddy_set_group(XmppBuddy *buddy, const gchar *group)
{
    IqRequest *iq;
    xmlnode   *node;

    g_return_val_if_fail(buddy != NULL, HYBRID_ERROR);
    g_return_val_if_fail(group != NULL, HYBRID_ERROR);

    iq = iq_request_create(buddy->stream, IQ_TYPE_SET);

    node = xmlnode_new_child(iq->node, "query");
    xmlnode_new_namespace(node, NULL, NS_IQ_ROSTER);
    node = xmlnode_new_child(node, "item");
    xmlnode_new_prop(node, "jid", buddy->jid);
    node = xmlnode_new_child(node, "group");
    xmlnode_set_content(node, group);

    if (iq_request_send(iq) != HYBRID_OK) {

        iq_request_destroy(iq);
        return HYBRID_ERROR;
    }

    iq_request_destroy(iq);

    return HYBRID_OK;
}

gint
xmpp_buddy_alias(XmppBuddy *buddy, const gchar *alias)
{
    IqRequest *iq;
    xmlnode   *node;

    g_return_val_if_fail(buddy != NULL, HYBRID_ERROR);
    g_return_val_if_fail(alias != NULL, HYBRID_ERROR);

    iq = iq_request_create(buddy->stream, IQ_TYPE_SET);

    node = xmlnode_new_child(iq->node, "query");
    xmlnode_new_namespace(node, NULL, NS_IQ_ROSTER);
    node = xmlnode_new_child(node, "item");
    xmlnode_new_prop(node, "jid", buddy->jid);
    xmlnode_new_prop(node, "name", alias);

    if (iq_request_send(iq) != HYBRID_OK) {

        iq_request_destroy(iq);
        return HYBRID_ERROR;
    }

    iq_request_destroy(iq);

    return HYBRID_OK;
}

gint
xmpp_buddy_send_presence(XmppStream *stream, const gchar *jid, gint type)
{
    xmlnode     *root;
    gchar       *xml_string;
    const gchar *type_str = NULL;

    g_return_val_if_fail(stream != NULL, HYBRID_ERROR);
    g_return_val_if_fail(jid != NULL, HYBRID_ERROR);

    switch (type) {
    case XMPP_PRESENCE_SUBSCRIBE:
      type_str = "subscribe";
      break;
    case XMPP_PRESENCE_SUBSCRIBED:
      type_str = "subscribed";
      break;
    case XMPP_PRESENCE_UNSUBSCRIBE:
      type_str = "unsubscribe";
      break;
    case XMPP_PRESENCE_UNSUBSCRIBED:
      type_str = "unsubscribed";
      break;
    default:
      break;
    }

    root = xmlnode_create("presence");
    xmlnode_new_prop(root, "from", stream->jid);
    xmlnode_new_prop(root, "to", jid);
    xmlnode_new_prop(root, "type", type_str);

    xml_string = xmlnode_to_string(root);
    xmlnode_free(root);

    hybrid_debug_info("xmpp", "send presence:\n%s", xml_string);

    if (hybrid_ssl_write(stream->ssl, xml_string,
                strlen(xml_string)) == -1) {

        hybrid_debug_error("xmpp", "send presence failed");
        g_free(xml_string);

        return HYBRID_ERROR;
    }

    g_free(xml_string);

    return HYBRID_OK;
}

gint
xmpp_buddy_delete(XmppBuddy *buddy)
{
    IqRequest  *iq;
    xmlnode    *node;
    XmppStream *stream;

    g_return_val_if_fail(buddy != NULL, HYBRID_ERROR);

    stream = buddy->stream;

    iq = iq_request_create(stream, IQ_TYPE_SET);

    node = xmlnode_new_child(iq->node, "query");
    xmlnode_new_prop(node, "from", stream->jid);
    xmlnode_new_namespace(node, NULL, NS_IQ_ROSTER);

    node = xmlnode_new_child(node, "item");
    xmlnode_new_prop(node, "jid", buddy->jid);
    xmlnode_new_prop(node, "subscription", "remove");

    g_hash_table_remove(stream->account->buddies, buddy->jid);

    if (iq_request_send(iq) != HYBRID_OK) {

        hybrid_debug_error("xmpp", "remove buddy failed.");
        iq_request_destroy(iq);

        return HYBRID_ERROR;
    }

    iq_request_destroy(iq);

    return HYBRID_OK;
}

typedef struct  {
    gchar *id;
    gchar *name;
    gchar *group;
} buddy_add_data;

static gboolean
buddy_add_cb(XmppStream *stream, xmlnode *root, buddy_add_data *data)
{
    xmlnode       *node;
    gchar         *value;
    HybridNotify  *notify;
    HybridAccount *account;
    HybridBuddy   *buddy;
    HybridGroup   *group;
    XmppBuddy     *xbuddy;

    account = stream->account->account;

    if (!xmlnode_has_prop(root, "type")) {

        notify = hybrid_notify_create(account, _("Warning"));
        hybrid_notify_set_text(notify, _("Failed to add buddy.\n"
                    "Invalid response message."));

        goto buddy_add_err;
    }

    value = xmlnode_prop(root, "type");

    if (g_strcmp0(value, "result") == 0) {

        if (!(group = hybrid_blist_find_group(account, data->group))) {
            hybrid_blist_add_group(account, data->group, data->group);
        }

        buddy = hybrid_blist_add_buddy(account, group, data->id, data->name);

        xbuddy = xmpp_buddy_create(stream, buddy);

        if (data->name && *data->name) {
            xmpp_buddy_set_name(xbuddy, data->name);

        } else {
            /* set the default name. */
            gchar *name;
            gchar *pos;

            for (pos = xbuddy->jid; *pos && *pos != '@'; pos ++);
            name = g_strndup(xbuddy->jid, pos - xbuddy->jid);
            xmpp_buddy_set_name(xbuddy, name);
            g_free(name);
        }

        if (!xbuddy->subscription) {
            xmpp_buddy_set_subscription(xbuddy, "none");
        }

        /* OK, add buddy success, subscribe buddy's presence. */
        xmpp_buddy_send_presence(stream, xbuddy->jid, XMPP_PRESENCE_SUBSCRIBE);

    } else {

        if ((node = xmlnode_find(root, "error"))) {
            notify = hybrid_notify_create(account, _("Warning"));

            if ((node = xmlnode_find(node, "text"))) {

                value = xmlnode_content(node);
                hybrid_notify_set_text(notify, value);
                g_free(value);

            } else {
                hybrid_notify_set_text(notify, _("Failed to add buddy."));
            }
        }
    }

buddy_add_err:

    g_free(data->id);
    g_free(data->name);
    g_free(data->group);
    g_free(data);

    return FALSE;
}

gint
xmpp_roster_add_item(XmppStream *stream, const gchar *jid, const gchar *name,
                     const gchar *group)
{
    IqRequest      *iq;
    xmlnode        *node;
    HybridAccount  *account;
    buddy_add_data *data;

    g_return_val_if_fail(stream != NULL, HYBRID_ERROR);
    g_return_val_if_fail(jid != NULL, HYBRID_ERROR);

    account = stream->account->account;

    iq = iq_request_create(stream, IQ_TYPE_SET);
    xmlnode_new_prop(iq->node, "from", stream->jid);

    node = xmlnode_new_child(iq->node, "query");
    xmlnode_new_namespace(node, NULL, NS_IQ_ROSTER);

    node = xmlnode_new_child(node, "item");
    xmlnode_new_prop(node, "jid", jid);

    if (name && *name) {
        xmlnode_new_prop(node, "name", name);
    }

    if (group) {
        node = xmlnode_new_child(node, "group");
        xmlnode_set_content(node, group);
    }

    data = g_new0(buddy_add_data, 1);
    data->id    = g_strdup(jid);
    data->name  = g_strdup(name);
    data->group = g_strdup(group);

    iq_request_set_callback(iq, (trans_callback)buddy_add_cb, data);

    if (iq_request_send(iq) != HYBRID_OK) {
        iq_request_destroy(iq);
        hybrid_account_error_reason(account, _("Connection Error."));
        return HYBRID_ERROR;
    }

    iq_request_destroy(iq);


    return HYBRID_OK;
}

gint
xmpp_buddy_get_info(XmppStream *stream, const gchar *jid,
                    trans_callback callback, gpointer user_data)
{
    IqRequest *iq;
    xmlnode   *node;

    g_return_val_if_fail(stream != NULL, HYBRID_ERROR);
    g_return_val_if_fail(jid != NULL, HYBRID_ERROR);

    iq = iq_request_create(stream, IQ_TYPE_GET);

    if (strcmp(stream->account->username, jid) == 0) {
        gchar *myjid = g_strdup_printf("%s@%s", jid, stream->account->to);
        xmlnode_new_prop(iq->node, "to", myjid);
        g_free(myjid);
    } else {
        xmlnode_new_prop(iq->node, "to", jid);
    }

    node = xmlnode_new_child(iq->node, "vCard");
    xmlnode_new_namespace(node, NULL, "vcard-temp");

    iq_request_set_callback(iq, callback, user_data);

    if (iq_request_send(iq) != HYBRID_OK) {

        hybrid_debug_error("xmpp", "get buddy info failed");
        iq_request_destroy(iq);

        return HYBRID_ERROR;
    }

    iq_request_destroy(iq);

    return HYBRID_OK;
}

XmppBuddy*
xmpp_buddy_find(XmppAccount *account, const gchar *jid)
{
    XmppBuddy  *buddy;
    GHashTable *xmpp_buddies;
    gchar      *tmp, *pos;

    g_return_val_if_fail(account != NULL, NULL);
    g_return_val_if_fail(jid != NULL, NULL);

    xmpp_buddies = account->buddies;

    g_return_val_if_fail(jid != NULL, NULL);

    if (!xmpp_buddies) {
        return NULL;
    }

    tmp = g_strdup(jid);

    for (pos = tmp; *pos; pos ++) {
        *pos = g_ascii_tolower(*pos);
    }

    buddy = g_hash_table_lookup(xmpp_buddies, tmp);
    g_free(tmp);

    return buddy;
}

void
xmpp_buddy_destroy(XmppBuddy *buddy)
{
    if (buddy) {
        g_free(buddy->jid);
        g_free(buddy->name);
        g_free(buddy->group);

        g_free(buddy);
    }
}

void
xmpp_buddy_clear(XmppStream *stream)
{
    XmppAccount *account;

    g_return_if_fail(stream != NULL);

    account = stream->account;

    if (!account->buddies) {
        return;
    }

    g_hash_table_remove_all(account->buddies);
}


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

static gchar*
get_resource(const gchar *full_jid)
{
    gchar *pos;

    for (pos = (gchar *)full_jid; *pos && *pos != '/'; pos ++);

    if (*pos == '\0') {
        return g_strdup(full_jid);
    }

    pos ++;

    return g_strndup(pos, full_jid + strlen(full_jid) - pos);
}


static gboolean
xmpp_login(HybridAccount *account)
{
    XmppAccount *ac;
    XmppStream  *stream;

    ac = xmpp_account_create(account, account->username,
                             account->password, "gmail.com");

    stream = xmpp_stream_create(ac);

    hybrid_account_set_protocol_data(account, stream);

    hybrid_proxy_connect(jabber_server, 5222,
                         (connect_callback)xmpp_stream_init, stream);

    return FALSE;
}

static gboolean
get_info_cb(XmppStream *stream, xmlnode *root, XmppBuddy *buddy)
{
    xmlnode      *node;
    gchar        *type;
    gchar        *name;
    gchar        *photo_bin;
    guchar       *photo;
    gchar        *resource;
    gchar        *status;
    GdkPixbuf    *pixbuf;
    gint          photo_len;
    GSList       *pos;
    XmppPresence *presence;

    HybridNotifyInfo *info;

    if (xmlnode_has_prop(root, "type")) {
        type = xmlnode_prop(root, "type");

        if (g_strcmp0(type, "result") != 0) {

            hybrid_debug_error("xmpp", "get buddy info error.");
            g_free(type);

            return FALSE;
        }

        g_free(type);
    }

    info = hybrid_notify_info_create();

    for (pos = buddy->presence_list; pos; pos = pos->next) {
        presence = (XmppPresence*)pos->data;

        resource = get_resource(presence->full_jid);
        hybrid_info_add_pair(info, _("Resource"), resource);
        g_free(resource);

        status = g_strdup_printf("[%s] %s",
                                 hybrid_get_presence_name(presence->show),
                                 presence->status ? presence->status : "");
        hybrid_info_add_pair(info, _("Status"), status);
        g_free(status);
    }

    if ((node = xmlnode_find(root, "FN"))) {
        name = xmlnode_content(node);

        hybrid_info_add_pair(info, _("Name"), name);
        g_free(name);
    }

    if ((node = xmlnode_find(root, "PHOTO"))) {

        if ((node = xmlnode_find(root, "BINVAL"))) {

            photo_bin = xmlnode_content(node);

            /* decode the base64-encoded photo string. */
            photo = hybrid_base64_decode(photo_bin, &photo_len);

            pixbuf = hybrid_create_pixbuf(photo, photo_len);

            if (pixbuf) {
                hybrid_info_add_pixbuf_pair(info, _("Photo"), pixbuf);
                g_object_unref(pixbuf);
            }

            g_free(photo_bin);
            g_free(photo);
        }
    }

    hybrid_info_notify(stream->account->account, info, buddy->jid);

    return TRUE;
}

static void
xmpp_get_info(HybridAccount *account, HybridBuddy *buddy)
{
    XmppStream *stream;
    XmppBuddy  *xbuddy;

    stream = hybrid_account_get_protocol_data(account);

    if (!(xbuddy = xmpp_buddy_find(stream->account, buddy->id))) {
        return;
    }

    xmpp_buddy_get_info(stream, buddy->id, (trans_callback)get_info_cb, xbuddy);
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
xmpp_keep_alive(HybridAccount *account)
{
    XmppStream *stream;

    stream = hybrid_account_get_protocol_data(account);

    if (xmpp_stream_ping(stream) != HYBRID_OK) {
        printf("%s\n", "error");
        return FALSE;
    }

    return TRUE;
}

static gboolean
xmpp_account_tooltip(HybridAccount *account, HybridTooltipData *tip_data)
{
    XmppStream *stream;
    gchar      *status;

    stream = hybrid_account_get_protocol_data(account);

    status = g_strdup_printf("[%s] %s",
                             hybrid_get_presence_name(account->state),
                             account->status_text ? account->status_text : "");

    hybrid_tooltip_data_add_title(tip_data, account->username);
    if (account->nickname) {
        hybrid_tooltip_data_add_pair(tip_data, _("Name"), account->nickname);
    }
    hybrid_tooltip_data_add_pair(tip_data, _("Status"), status);
    /* add _() anyway LOL */
    //hybrid_tooltip_data_add_pair(tip_data, _("Resource"), bd->resource);

    return TRUE;
}

static gboolean
xmpp_buddy_tooltip(HybridAccount *account, HybridBuddy *buddy,
        HybridTooltipData *tip_data)
{
    XmppBuddy    *bd;
    XmppPresence *presence;
    XmppStream   *stream;
    gchar        *status;
    gchar        *resource;
    gchar        *name;
    GSList       *pos;

    stream = hybrid_account_get_protocol_data(account);

    if (!(bd = xmpp_buddy_find(stream->account, buddy->id))) {
        return FALSE;
    }

    hybrid_tooltip_data_add_title(tip_data, bd->jid);
    hybrid_tooltip_data_add_pair(tip_data, _("Name"), bd->name);

    for (pos = bd->presence_list; pos; pos = pos->next) {
        presence = (XmppPresence *)pos->data;

        resource = get_resource(presence->full_jid);
        status = g_strdup_printf("[<b>%s</b>] %s",
                                 hybrid_get_presence_name(presence->show),
                                 presence->status ? presence->status : "");


        name = g_strdup_printf(_("Status (%s)"), resource);
        g_free(resource);

        hybrid_tooltip_data_add_pair_markup(tip_data, name, status);

        g_free(status);
        g_free(name);
    }

    hybrid_tooltip_data_add_pair(tip_data, _("Subscription"), bd->subscription);
    return TRUE;
}

static gboolean
xmpp_buddy_remove(HybridAccount *account, HybridBuddy *buddy)
{
    XmppBuddy  *xbuddy;
    XmppStream *stream;

    stream = hybrid_account_get_protocol_data(account);

    if (!(xbuddy = xmpp_buddy_find(stream->account, buddy->id))) {
        return TRUE;
    }

    if (xmpp_buddy_delete(xbuddy) != HYBRID_OK) {
        return FALSE;
    }

    return TRUE;
}

static gboolean
xmpp_buddy_rename(HybridAccount *account, HybridBuddy *buddy, const gchar *text)
{
    XmppBuddy  *xbuddy;
    XmppStream *stream;

    stream = hybrid_account_get_protocol_data(account);

    if (!(xbuddy = xmpp_buddy_find(stream->account, buddy->id))) {
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

static gboolean
xmpp_buddy_req(HybridAccount *account, HybridGroup *group,
               const gchar *id, const gchar *alias,
               gboolean accept, const gpointer user_data)
{
    XmppStream  *stream;
    XmppBuddy   *buddy;
    HybridBuddy *bd;

    stream = hybrid_account_get_protocol_data(account);

    if (accept) {
        xmpp_buddy_send_presence(stream, id, XMPP_PRESENCE_SUBSCRIBED);

        bd = hybrid_blist_add_buddy(account, group, id, alias);
        buddy = xmpp_buddy_create(stream, bd);

        if (alias && *alias) {
            xmpp_buddy_set_name(buddy, alias);

        } else {
            /* set the default name. */
            gchar *name;
            gchar *pos;

            for (pos = (gchar *)id; *pos && *pos != '@'; pos ++);
            name = g_strndup(id, pos - id);
            xmpp_buddy_set_name(buddy, name);
            g_free(name);
        }


    } else {
        xmpp_buddy_send_presence(stream, id, XMPP_PRESENCE_UNSUBSCRIBED);
    }


    return FALSE;
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
    XmppBuddy  *xbuddy;
    XmppStream *stream;

    stream = hybrid_account_get_protocol_data(account);

    if (!(xbuddy = xmpp_buddy_find(stream->account, buddy->id))) {
        return FALSE;
    }

    if (xmpp_buddy_set_group(xbuddy, new_group->name) != HYBRID_OK) {
        return FALSE;
    }

    return TRUE;
}

static void
xmpp_send_typing(HybridAccount *account, HybridBuddy *buddy, HybridInputState state)
{
    XmppStream *stream;

    stream = hybrid_account_get_protocol_data(account);

    xmpp_message_send_typing(stream, buddy->id, state);
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

    if (stream->source > 0) {
        g_source_remove(stream->source);
    }

    if (stream->keepalive_source > 0) {
        g_source_remove(stream->keepalive_source);
    }

    close(stream->sk);

    xmpp_stream_destroy(stream);
    xmpp_buddy_clear(stream);
}

HybridIMOps im_ops = {
    xmpp_login,                 /**< login */
    xmpp_get_info,              /**< get_info */
    xmpp_modify_name,           /**< modify_name */
    xmpp_modify_status,         /**< modify_status */
    xmpp_modify_photo,          /**< modify_photo */
    xmpp_change_state,          /**< change_state */
    xmpp_keep_alive,            /**< keep_alive */
    xmpp_account_tooltip,       /**< account_tooltip */
    xmpp_buddy_tooltip,         /**< buddy_tooltip */
    xmpp_buddy_move,            /**< buddy_move */
    xmpp_buddy_remove,          /**< buddy_remove */
    xmpp_buddy_rename,          /**< buddy_rename */
    xmpp_buddy_add,             /**< buddy_add */
    xmpp_buddy_req,             /**< buddy_req */
    NULL,                       /**< group_rename */
    NULL,                       /**< group_remove */
    xmpp_group_add,             /**< group_add */
    NULL,                       /**< chat_word_limit */
    NULL,                       /**< chat_start */
    xmpp_send_typing,           /**< chat_send_typing */
    xmpp_chat_send,             /**< chat_send */
    xmpp_close,                 /**< close */
};

HybridModuleInfo module_info = {
    "xmpp",                     /**< name */
    "levin108",                 /**< author */
    N_("jabber client"),        /**< summary */
    /* description */
    N_("implement xmpp protocol"),
    "http://basiccoder.com",      /**< homepage */
    "0","1",                    /**< major version, minor version */
    "xmpp",                     /**< icon name */
    MODULE_TYPE_IM,

    &im_ops,
    NULL,
    NULL,
    NULL, /**< actions */
};

void
xmpp_module_init(HybridModule *module)
{

}

HYBRID_MODULE_INIT(xmpp_module_init, &module_info);

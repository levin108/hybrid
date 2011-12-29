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

#include "xmpp_account.h"
#include "xmpp_stream.h"
#include "xmpp_buddy.h"
#include "xmpp_iq.h"

XmppAccount*
xmpp_account_create(HybridAccount *account, const gchar *username,
                    const gchar *password, const gchar *to)
{
    XmppAccount *ac;

    g_return_val_if_fail(account != NULL, NULL);
    g_return_val_if_fail(username != NULL, NULL);
    g_return_val_if_fail(password != NULL, NULL);
    g_return_val_if_fail(to != NULL, NULL);

    ac = g_new0(XmppAccount, 1);

    ac->account  = account;
    ac->username = g_strdup(username);
    ac->password = g_strdup(password);
    ac->to       = g_strdup(to);

    return ac;
}

void
xmpp_account_destroy(XmppAccount *account)
{
    if (account) {
        g_free(account->username);
        g_free(account->password);
        g_free(account->to);

        g_free(account);
    }
}

gint
xmpp_account_process_info(XmppStream *stream, xmlnode *root)
{
    xmlnode *node;
    gchar   *value;
    guchar  *photo;
    gint     photo_len;

    HybridAccount *account;

    account = stream->account->account;

    if (xmlnode_has_prop(root, "type")) {
        value = xmlnode_prop(root, "type");

        if (g_strcmp0(value, "result") != 0) {
            g_free(value);
            return HYBRID_ERROR;
        }

        g_free(value);
    }

    if ((node = xmlnode_find(root, "FN"))) {

        value = xmlnode_content(node);
        hybrid_account_set_nickname(account, value);
        g_free(value);
    }

    if ((node = xmlnode_find(root, "PHOTO"))) {

        if ((node = xmlnode_find(root, "BINVAL"))) {

            value = xmlnode_content(node);

            /* decode the base64-encoded photo string. */
            photo = hybrid_base64_decode(value, &photo_len);

            /* set icon for the buddy. */
            hybrid_account_set_icon(account, photo,    photo_len, "");

            g_free(value);
            g_free(photo);
        }
    }

    return HYBRID_OK;
}

gint
xmpp_account_modify_status(XmppStream *stream, gint state, const gchar *status)
{
    xmlnode       *root;
    xmlnode       *node;
    gchar         *xml_string;
    HybridAccount *account;

    g_return_val_if_fail(stream != NULL, HYBRID_ERROR);

    account = stream->account->account;

    if (state == HYBRID_STATE_OFFLINE) {
        state = HYBRID_STATE_ONLINE;
    }

    root = xmlnode_create("presence");
    xmlnode_new_prop(root, "from", stream->jid);

    switch (state) {
    case HYBRID_STATE_AWAY:
      node = xmlnode_new_child(root, "show");
      xmlnode_set_content(node, "away");
      break;
    case HYBRID_STATE_BUSY:
      node = xmlnode_new_child(root, "show");
      xmlnode_set_content(node, "dnd");
      break;
    default:
      break;
    };

    if (status) {
        node = xmlnode_new_child(root, "status");
        xmlnode_set_content(node, status);
    }

    xml_string = xmlnode_to_string(root);
    xmlnode_free(root);

    hybrid_debug_info("xmpp", "modify status,send:\n%s", xml_string);

    if (hybrid_ssl_write(stream->ssl, xml_string,
                strlen(xml_string)) == -1) {

        hybrid_debug_error("xmpp", "modify status failed.");
        g_free(xml_string);

        return HYBRID_ERROR;
    }

    g_free(xml_string);

    /* we store the status. */
    if (status && g_strcmp0(account->status_text, status)) {
        hybrid_account_set_status_text(account, status);
        hybrid_account_update(account);
    }

    /* set account's presence state. */
    hybrid_account_set_state(account, state);

    return HYBRID_OK;
}

gboolean
modify_name_cb(XmppStream *stream, xmlnode *root, gpointer user_data)
{
    gchar        *value;
    HybridNotify *notify;

    if (!xmlnode_has_prop(root, "type")) {
        goto modify_name_err;
    }

    value = xmlnode_prop(root, "type");

    if (g_strcmp0(value, "result") != 0) {
        g_free(value);
        goto modify_name_err;
    }

    g_free(value);

    return TRUE;

modify_name_err:

    notify = hybrid_notify_create(stream->account->account, _("System Message"));
    hybrid_notify_set_text(notify, _("Modify full name failed."));

    return FALSE;
}

static gboolean
account_get_info_cb(XmppStream *stream, xmlnode *root, gpointer user_data)
{
    xmpp_account_process_info(stream, root);

    return TRUE;
}

static gboolean
modify_photo_cb(XmppStream *stream, xmlnode *root, gpointer user_data)
{
    gchar        *value;
    HybridNotify *notify;

    if (!xmlnode_has_prop(root, "type")) {
        goto modify_name_err;
    }

    value = xmlnode_prop(root, "type");

    if (g_strcmp0(value, "result") != 0) {
        g_free(value);
        goto modify_name_err;
    }

    g_free(value);

    /*
     * modify photo success, we fetch the lastest account
     * information from the server.
     */
    xmpp_buddy_get_info(stream, stream->account->username,
                (trans_callback)account_get_info_cb, NULL);

    return TRUE;

modify_name_err:

    notify = hybrid_notify_create(stream->account->account, _("System Message"));
    hybrid_notify_set_text(notify, _("Modify photo failed."));

    return FALSE;
}

gint
xmpp_account_modify_name(XmppStream *stream, const gchar *name)
{
    IqRequest *iq;
    xmlnode   *node;

    g_return_val_if_fail(stream != NULL, HYBRID_ERROR);
    g_return_val_if_fail(name != NULL, HYBRID_ERROR);

    iq = iq_request_create(stream, IQ_TYPE_SET);

    node = xmlnode_new_child(iq->node, "vCard");
    xmlnode_new_namespace(node, NULL, "vcard-temp");

    node = xmlnode_new_child(node, "FN");
    xmlnode_set_content(node, name);

    iq_request_set_callback(iq, modify_name_cb, NULL);

    if (iq_request_send(iq) != HYBRID_OK) {

        hybrid_debug_error("xmpp", "modify photo failed.");
        iq_request_destroy(iq);

        return HYBRID_ERROR;
    }

    iq_request_destroy(iq);

    return HYBRID_OK;
}


gint
xmpp_account_modify_photo(XmppStream *stream, const gchar *filename)
{
    IqRequest *iq;
    xmlnode   *node;
    xmlnode   *photo_node;
    guchar    *file_bin;
    gsize      file_size;
    gchar     *file_base64;

    g_return_val_if_fail(stream != NULL, HYBRID_ERROR);
    g_return_val_if_fail(filename != NULL, HYBRID_ERROR);

    if (!g_file_get_contents(filename, (gchar**)&file_bin, &file_size, NULL)) {
        hybrid_debug_error("xmpp", "file %s doesn't exist.", filename);
        return HYBRID_ERROR;
    }

    iq = iq_request_create(stream, IQ_TYPE_SET);

    node = xmlnode_new_child(iq->node, "vCard");
    xmlnode_new_namespace(node, NULL, "vcard-temp");

    photo_node = xmlnode_new_child(node, "PHOTO");
    node = xmlnode_new_child(photo_node, "TYPE");
    xmlnode_set_content(node, "image/png");

    file_base64 = hybrid_base64_encode(file_bin, file_size);
    g_free(file_bin);

    node = xmlnode_new_child(photo_node, "BINVAL");
    xmlnode_set_content(node, file_base64);

    iq_request_set_callback(iq, modify_photo_cb, NULL);

    if (iq_request_send(iq) != HYBRID_OK) {

        hybrid_debug_error("xmpp", "modify photo failed.");
        iq_request_destroy(iq);

        return HYBRID_ERROR;
    }

    iq_request_destroy(iq);

    return HYBRID_OK;
}

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
nn *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 ***************************************************************************/

#include "util.h"
#include "gtkutils.h"
#include "connect.h"
#include "buddyreq.h"
#include "conv.h"

#include "xmpp_util.h"
#include "xmpp_stream.h"
#include "xmpp_parser.h"
#include "xmpp_buddy.h"
#include "xmpp_iq.h"

static gchar *generate_starttls_body(XmppStream *stream);
static gchar *create_initiate_stream(XmppStream *xs);
static void   xmpp_stream_get_roster(XmppStream *stream);
static void   xmpp_stream_get_vcard(XmppStream *stream);

static gchar*
get_bare_jid(const gchar *full_jid)
{
    gchar *pos;

    for (pos = (gchar *)full_jid; *pos && *pos != '/'; pos ++);

    return g_strndup(full_jid, pos - full_jid);
}

XmppStream*
xmpp_stream_create(XmppAccount *account)
{
    XmppStream *stream;

    g_return_val_if_fail(account != NULL, NULL);

    stream = g_new0(XmppStream, 1);

    stream->account       = account;
    stream->major_version = 1;
    stream->miner_version = 0;

    return stream;
}

void
xmpp_stream_set_id(XmppStream *stream, const gchar *id)
{
    g_return_if_fail(stream != NULL);

    g_free(stream->stream_id);

    stream->stream_id = g_strdup(id);
}

void
xmpp_stream_set_jid(XmppStream *stream, const gchar *jid)
{
    g_return_if_fail(stream != NULL);

    g_free(stream->jid);

    stream->jid = g_strdup(jid);
}

void
xmpp_stream_iqid_increase(XmppStream *stream)
{
    g_return_if_fail(stream != NULL);

    stream->current_iq_id ++;
}

gchar*
xmpp_stream_get_iqid(XmppStream *stream)
{
    gchar *id;

    g_return_val_if_fail(stream != NULL, NULL);

    id = g_strdup_printf("%d", stream->current_iq_id);

    return id;
}

void
xmpp_stream_set_state(XmppStream *stream, gint state)
{
    g_return_if_fail(stream != NULL);

    stream->state = state;
}

void
xmpp_stream_destroy(XmppStream *stream)
{
    IqTransaction *trans;

    if (stream) {

        g_free(stream->jid);
        g_free(stream->stream_id);

        xmpp_account_destroy(stream->account);

        while (stream->pending_trans) {
            trans = (IqTransaction*)stream->pending_trans->data;
            iq_transaction_remove(stream, trans);
        }

        hybrid_connection_destroy(stream->conn);
        hybrid_ssl_connection_destory(stream->ssl);

        xmlnode_free(stream->node);
        stream->node = NULL;

        g_free(stream);
        stream = NULL;
    }
}

/**
 * Send the starttls request:
 * <starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>
 */
static void
xmpp_stream_starttls(XmppStream *stream)
{
    gchar *body;

    g_return_if_fail(stream != NULL);

    xmpp_stream_set_state(stream, XMPP_STATE_TLS_AUTHENTICATING);

    body = generate_starttls_body(stream);

    if (send(stream->sk, body, strlen(body), 0) == -1) {

        hybrid_debug_error("stream", "start tls error.");

        return;
    }

    g_free(body);
}

/**
 * Start sasl authentication.
 */
static void
xmpp_stream_startsasl(XmppStream *stream)
{
    guchar  *auth;
    gchar   *auth_encoded;
    gchar   *xml_string;
    gint     username_len;
    gint     password_len;
    xmlnode *node;

    g_return_if_fail(stream != NULL);

    hybrid_debug_info("xmpp", "start sasl authentication.");

    hybrid_account_set_connection_string(stream->account->account,
                                         _("Start sasl authentication..."));

    xmpp_stream_set_state(stream, XMPP_STATE_SASL_AUTHENTICATING);
    /*
     * construct the authentication string to be base64 encoded,
     * which is in form of '\0 + username + \0 + password '
     */
    username_len = strlen(stream->account->username);
    password_len = strlen(stream->account->password);

    auth = g_malloc0(username_len + password_len + 2);

    auth[0] = '\0';
    memcpy(auth + 1, stream->account->username, username_len);

    auth[username_len + 1] = '\0';
    memcpy(auth + 2 + username_len, stream->account->password,
            password_len);

    auth_encoded = hybrid_base64_encode(auth, username_len + password_len + 2);

    g_free(auth);

    /* construct the xml string, which is in form of:
     *
     * <auth xmlns="urn:ietf:params:xml:ns:xmpp-sasl"
     * mechanism="PLAIN">encoded sasl string</auth>
     */
    node = xmlnode_create("auth");

    xmlnode_new_namespace(node, NULL, NS_XMPP_SASL);
    xmlnode_new_namespace(node, "ga", "http://www.google.com/talk/protocol/auth");
    xmlnode_new_prop(node, "ga:client-uses-full-bind-result", "true");
    xmlnode_new_prop(node, "mechanism", "PLAIN");
    xmlnode_set_content(node, auth_encoded);

    g_free(auth_encoded);

    xml_string = xmlnode_to_string(node);
    xmlnode_free(node);

    hybrid_debug_info("xmpp", "sasl send:\n%s", xml_string);

    if (hybrid_ssl_write(stream->ssl, xml_string,
                         strlen(xml_string)) == -1) {

        hybrid_account_error_reason(stream->account->account,
                                    _("SASL authentication error."));

        return;
    }

    g_free(xml_string);

}

static gboolean
stream_recv_cb(gint sk, XmppStream *stream)
{
    gchar buf[BUF_LENGTH];
    gint  n;

    if (!stream->ssl) {
        if ((n = recv(sk, buf, sizeof(buf) - 1, 0)) == -1) {

            hybrid_debug_error("xmpp", "init stream error.");
            return FALSE;
        }
    } else {

        if ((n = hybrid_ssl_read(stream->ssl, buf, sizeof(buf) - 1)) == -1) {

            hybrid_debug_error("xmpp", "stream read io error.");
            return TRUE;
        } else if (0 == n) {

            hybrid_debug_error("xmpp", "connection closed by server.");
            return FALSE;
        }
    }

    buf[n] = '\0';

    if (n > 0) {
        xmpp_process_pushed(stream, buf, n);
    }
    return TRUE;
}

gboolean
xmpp_stream_init(gint sk, XmppStream *stream)
{
    gchar *msg;

    stream->sk = sk;

    /* send version. */
    msg = "<?xml version='1.0' ?>";

    if (send(sk, msg, strlen(msg), 0) == -1) {

        hybrid_account_error_reason(stream->account->account,
                                    _("send initial jabber request failed"));

        return FALSE;
    }

    /* send initiate stream request. */
    msg = create_initiate_stream(stream);

    hybrid_debug_info("xmpp", "send initite jabber stream:\n%s", msg);

    if (send(sk, msg, strlen(msg), 0) == -1) {

        hybrid_account_error_reason(stream->account->account,
                                    _("send initial jabber request failed"));
        g_free(msg);

        return FALSE;
    }

    g_free(msg);

    stream->source = hybrid_event_add(sk, HYBRID_EVENT_READ,
                                      (input_func)stream_recv_cb, stream);

    return FALSE;
}

static gboolean
ping_cb(XmppStream *stream, xmlnode *node, gpointer user_data)
{
    gchar *value;

    if (xmlnode_has_prop(node, "type")) {
        value = xmlnode_prop(node, "type");

        if (g_strcmp0(value, "result") != 0) {

            hybrid_debug_error("xmpp", "ping xmpp server refused.");

            hybrid_account_error_reason(stream->account->account,
                                        _("Connection Closed."));
        }
    }

    if (stream->keepalive_source) {
        g_source_remove(stream->keepalive_source);
        stream->keepalive_source = 0;
    }

    return FALSE;
}

static gboolean
ping_timeout_cb(XmppStream *stream)
{
    hybrid_debug_error("xmpp", "ping timeout");

    return FALSE;
}

gint
xmpp_stream_ping(XmppStream *stream)
{
    IqRequest *iq;
    xmlnode   *node;

    g_return_val_if_fail(stream != NULL, HYBRID_ERROR);

    iq = iq_request_create(stream, IQ_TYPE_GET);

    node = xmlnode_new_child(iq->node, "ping");
    xmlnode_new_namespace(node, NULL, NS_XMPP_PING);

    iq_request_set_callback(iq, ping_cb, NULL);

    if (stream->keepalive_source) {
        g_source_remove(stream->keepalive_source);
    }

    stream->keepalive_source =
        g_timeout_add_seconds(60, (GSourceFunc)ping_timeout_cb, stream);

    if (iq_request_send(iq) != HYBRID_OK) {

        hybrid_debug_error("xmpp", "ping xmpp server failed.");

        iq_request_destroy(iq);

        hybrid_account_error_reason(stream->account->account,
                                    _("Connection Closed."));

        return HYBRID_ERROR;
    }

    iq_request_destroy(iq);

    return HYBRID_OK;
}

/**
 * Ok, TLS handshake success, we will continue the authtication
 * through the established TLS channel.
 */
static gboolean
tls_conn_cb(HybridSslConnection *ssl, XmppStream *stream)
{
    gchar *msg;

    hybrid_account_set_connection_string(stream->account->account,
                                         _("TLS connection established."));

    msg = create_initiate_stream(stream);

    /*
     * Reset the stream id, we will start a new stream through
     * the TLS channel, whether stream id is NULL is the flag
     * of a new stream.
     */
    xmpp_stream_set_id(stream, NULL);
    xmpp_stream_set_state(stream, XMPP_STATE_TLS_STREAM_STARTING);

    hybrid_debug_info("stream", "send start stream request:\n%s", msg);

    if (hybrid_ssl_write(ssl, msg, strlen(msg)) == -1) {

        hybrid_account_error_reason(stream->account->account,
                                    _("send initial jabber request failed"));
        g_free(msg);

        return FALSE;
    }

    g_free(msg);

    return FALSE;
}

/**
 * Start the TLS handshake, if success, jump to the
 * tls_conn_cb() callback function to send the
 * stream start request through the TLS channel.
 */
static void
xmpp_stream_performtls(XmppStream *stream)
{

    g_return_if_fail(stream != NULL);

    hybrid_account_set_connection_string(stream->account->account,
                                         _("Start performing tls..."));

    if (!(stream->ssl = hybrid_ssl_connect_with_fd(
              stream->sk, (ssl_callback)tls_conn_cb, stream))) {

        hybrid_account_error_reason(stream->account->account,
                                    _("TLS hand-shake failed."));

        return;
    }
}

/**
 * What should be done after authenticating successfully,
 * set connection status, set account state, and then fetch
 * the roster.
 */
static void
auth_success(XmppStream *stream)
{
    HybridAccount *account;

    g_return_if_fail(stream != NULL);

    account = stream->account->account;

    if (account->state == HYBRID_STATE_OFFLINE ||
        account->state == HYBRID_STATE_INVISIBLE) {
        account->state = HYBRID_STATE_ONLINE;
    }

    /* set account's presence state. */
    hybrid_account_set_state(account, account->state);

    /*
     * Remember to do this before adding any buddies to the blist,
     * we should set the account to be connected first.
     */
    hybrid_account_set_connection_status(account,
            HYBRID_CONNECTION_CONNECTED);

    /* OK, we request the roster from the server. */
    xmpp_stream_get_vcard(stream);
    xmpp_stream_get_roster(stream);

}

static gboolean
account_get_info_cb(XmppStream *stream, xmlnode *root, gpointer user_data)
{
    xmpp_account_process_info(stream, root);

    auth_success(stream);

    return FALSE;
}

/**
 * Callback function for the start session request to process the response.
 */
static gboolean
start_session_cb(XmppStream *stream, xmlnode *root, gpointer user_data)
{
    HybridAccount *account;

    account = stream->account->account;

    xmpp_buddy_get_info(stream, stream->account->username,
                (trans_callback)account_get_info_cb, NULL);

    return TRUE;
}

/**
 * Callback function for the resource bind request to process the response.
 */
static gboolean
resource_bind_cb(XmppStream *stream, xmlnode *root, gpointer user_data)
{
    gchar   *type;
    gchar   *jid;
    xmlnode *node;

    type = xmlnode_prop(root, "type");

    if (g_strcmp0(type, "result")) {
        hybrid_account_error_reason(stream->account->account,
                                    _("resource bind error."));
        return FALSE;
    }

    if (!(node = xmlnode_find(root, "jid"))) {
        hybrid_account_error_reason(stream->account->account,
                                    _("resource bind error: "
                                      "no jabber id returned."));
        return FALSE;
    }

    jid = xmlnode_content(node);
    xmpp_stream_set_jid(stream, jid);
    g_free(jid);

    /* request to start a session. */
    IqRequest *iq;

    iq = iq_request_create(stream, IQ_TYPE_SET);

    node = xmlnode_new_child(iq->node, "session");
    xmlnode_new_namespace(node, NULL, NS_XMPP_SESSION);

    iq_request_set_callback(iq, start_session_cb, NULL);

    if (iq_request_send(iq) != HYBRID_OK) {

        hybrid_account_error_reason(stream->account->account,
                                    _("start session error."));

        iq_request_destroy(iq);

        return TRUE;
    }

    iq_request_destroy(iq);

    return TRUE;
}

/**
 * Client binds a resource.
 */
static void
xmpp_stream_bind(XmppStream *stream)
{
    xmlnode   *node;
    IqRequest *iq;

    g_return_if_fail(stream != NULL);

    iq = iq_request_create(stream, IQ_TYPE_SET);

    node = xmlnode_new_child(iq->node, "bind");
    xmlnode_new_namespace(node, NULL, NS_XMPP_BIND);

    node = xmlnode_new_child(node, "resource");
    xmlnode_set_content(node, "Hybrid.");

    iq_request_set_callback(iq, resource_bind_cb, NULL);

    if (iq_request_send(iq) != HYBRID_OK) {

        hybrid_account_error_reason(stream->account->account,
                                    _("binds a resource error."));

        iq_request_destroy(iq);

        return;
    }

    iq_request_destroy(iq);
}

/**
 * Callback function to handle the roster response.
 */
static gboolean
request_roster_cb(XmppStream *stream, xmlnode *root, gpointer user_data)
{
    xmpp_buddy_process_roster(stream, root);
    return TRUE;
}

/**
 * Request the roster from the server.
 */
static void
xmpp_stream_get_roster(XmppStream *stream)
{
    xmlnode   *node;
    IqRequest *iq;

    g_return_if_fail(stream != NULL);

    xmpp_stream_iqid_increase(stream);

    iq = iq_request_create(stream, IQ_TYPE_GET);
    iq_request_set_callback(iq, request_roster_cb, NULL);

    node = xmlnode_new_child(iq->node, "query");
    xmlnode_new_namespace(node, NULL, NS_IQ_ROSTER);

#if 0
    xmlnode_new_namespace(node, "gr", NS_GOOGLE_ROSTER);
    xmlnode_new_prop(node, "gr:ext", "2");
#endif

    if (iq_request_send(iq) != HYBRID_OK) {

        hybrid_account_error_reason(stream->account->account,
                                    _("request roster error."));
        iq_request_destroy(iq);

        return;
    }

    iq_request_destroy(iq);
}
/**
 * Request the vcard from the server.
 */
static void
xmpp_stream_get_vcard(XmppStream *stream)
{
    xmlnode   *node;
    IqRequest *iq;

    g_return_if_fail(stream != NULL);

    xmpp_stream_iqid_increase(stream);

    iq = iq_request_create(stream, IQ_TYPE_GET);
    //iq_request_set_callback(iq, request_roster_cb, NULL);

    node = xmlnode_new_child(iq->node, "vCard");
    xmlnode_new_namespace(node, NULL, "vcard-temp");

    if (iq_request_send(iq) != HYBRID_OK) {

        hybrid_account_error_reason(stream->account->account,
                                    _("request vcard error."));
        iq_request_destroy(iq);

        return;
    }

    iq_request_destroy(iq);
}

/**
 * Process the <steam:features> messages.
 */
static void
xmpp_process_feature(XmppStream *stream, xmlnode *root)
{
    xmlnode *node;

    g_return_if_fail(stream != NULL);
    g_return_if_fail(root != NULL);

    if ((node = xmlnode_find(root, "starttls"))) {
        xmpp_stream_starttls(stream);

    } else {

        if (stream->state == XMPP_STATE_SASL_STREAM_STARTING) {
            /*
             * After starting a new stream on the sasl layer, server request
             * client to bind resource and start a new session.
             */
            if ((node = xmlnode_find(root, "bind"))) {
                xmpp_stream_bind(stream);
            }

            if ((node = xmlnode_find(root, "session"))) {

            }

        } else {
            xmpp_stream_startsasl(stream);
        }
    }
}

/**
 * Start a new stream on the sasl layer.
 */
static void
xmpp_stream_new_on_sasl(XmppStream *stream)
{
    gchar *msg;

    g_return_if_fail(stream != NULL);

    msg = create_initiate_stream(stream);

    xmpp_stream_set_id(stream, NULL);
    xmpp_stream_set_state(stream, XMPP_STATE_SASL_STREAM_STARTING);

    hybrid_debug_info("xmpp", "new stream on sasl,send:\n%s", msg);

    if (hybrid_ssl_write(stream->ssl, msg, strlen(msg)) == -1) {
        hybrid_account_error_reason(stream->account->account,
                                    _("start new stream on sasl layer error"));
        return;
    }

    g_free(msg);
}

/**
 * Process the roster set request.
 */
static void
xmpp_stream_process_set_roster(XmppStream *stream, xmlnode *query)
{
    xmlnode   *node;
    gchar     *value;
    XmppBuddy *buddy;

    g_return_if_fail(stream != NULL);
    g_return_if_fail(query != NULL);

    if (!(node = xmlnode_child(query))) {
        return;
    }

    while (node) {

        if (g_strcmp0(node->name, "item") != 0) {
            node = node->next;
            continue;
        }

        if (!xmlnode_has_prop(node, "jid")) {
            node = node->next;
            continue;
        }

        value = xmlnode_prop(node, "jid");

        if (!(buddy = xmpp_buddy_find(stream->account, value))) {
            g_free(value);
            node = node->next;
            continue;
        }
        g_free(value);

        if (xmlnode_has_prop(node, "subscription")) {
            value = xmlnode_prop(node, "subscription");
            xmpp_buddy_set_subscription(buddy, value);
            g_free(value);
        }

        if (xmlnode_has_prop(node, "name")) {
            value = xmlnode_prop(node, "name");
            xmpp_buddy_set_name(buddy, value);
            g_free(value);
        }

        node = node->next;
    }
}

/**
 * Process the iq set request.
 */
static void
xmpp_stream_process_iq_set(XmppStream *stream, xmlnode *root)
{
    gchar   *id;
    gchar   *value;
    xmlnode *node;
    gint     count = 0;

    g_return_if_fail(stream != NULL);
    g_return_if_fail(root != NULL);

    node = xmlnode_child(root);

    for (; node; node = node->next, count ++);

    /*
     * An IQ stanza of type "get" or "set" MUST contain exactly one
     * child element, which specifies the semantics of the particular
     *  request.
     */
    if (count != 1) {
        /* TODO send error stanza. */
        return;
    }

    node = xmlnode_child(root);

    if (g_strcmp0(node->name, "query") == 0) {
        value = xmlnode_get_namespace(node);

        if (g_strcmp0(value, NS_IQ_ROSTER) == 0) {
            xmpp_stream_process_set_roster(stream, node);
        }

        g_free(value);
    }

    /* send a result response. */
    IqRequest *iq;

    iq = iq_request_create(stream, IQ_TYPE_RESULT);

    xmlnode_new_prop(iq->node, "from", stream->jid);

    if (xmlnode_has_prop(root, "id")) {

        id = xmlnode_prop(root, "id");
        xmlnode_set_prop(iq->node, "id", id);
        g_free(id);
    }

    iq_request_send(iq);
    iq_request_destroy(iq);
}

/**
 * Process the iq messages.
 */
static void
xmpp_stream_process_iq(XmppStream *stream, xmlnode *node)
{
    gchar         *id;
    gint           id_int;
    GSList        *pos;
    IqTransaction *trans;
    gchar         *value;

    g_return_if_fail(stream != NULL);
    g_return_if_fail(node != NULL);

    if (!xmlnode_has_prop(node, "type") ||
        !xmlnode_has_prop(node, "id")) {
        hybrid_debug_error("xmpp", "invalid iq message.");

        return;
    }

    value = xmlnode_prop(node, "type");

    /* response to a get or set request. */
    if (g_strcmp0(value, "result") == 0 ||
        g_strcmp0(value, "error") == 0) {

        id = xmlnode_prop(node, "id");
        id_int = atoi(id);
        g_free(id);

        for (pos = stream->pending_trans; pos; pos = pos->next) {
            trans = (IqTransaction*)pos->data;

            if (trans->iq_id == id_int) {

                if (trans->callback) {
                    trans->callback(stream, node, trans->user_data);
                }

                iq_transaction_remove(stream, trans);
            }
        }

    /*
     * An entity that receives an IQ request of type "get" or "set" MUST
       reply with an IQ response of type "result" or "error".
     */
    } else if (g_strcmp0(value, "get") == 0) { /* process later. */
    } else if (g_strcmp0(value, "set") == 0) {
        xmpp_stream_process_iq_set(stream, node);
    }

    g_free(value);
}

/**
 * Callback function to process the buddy's get-info response.
 */
static gboolean
buddy_get_info_cb(XmppStream *stream, xmlnode *root, XmppBuddy *buddy)
{
    xmlnode *node;
    gchar   *type;
    gchar   *photo_bin;
    guchar  *photo;
    gint     photo_len;

    if (xmlnode_has_prop(root, "type")) {
        type = xmlnode_prop(root, "type");

        if (g_strcmp0(type, "result") != 0) {

            hybrid_debug_error("xmpp", "get buddy info error.");
            g_free(type);

            return FALSE;
        }

        g_free(type);
    }

    if ((node = xmlnode_find(root, "PHOTO"))) {

        if ((node = xmlnode_find(root, "BINVAL"))) {

            photo_bin = xmlnode_content(node);

            /* decode the base64-encoded photo string. */
            photo = hybrid_base64_decode(photo_bin, &photo_len);

            /* set icon for the buddy. */
            hybrid_blist_set_buddy_icon(buddy->buddy, photo,
                                    photo_len, buddy->photo);
            g_free(photo_bin);
            g_free(photo);
        }
    }

    return TRUE;
}

static void
xmpp_stream_process_presence(XmppStream *stream, xmlnode *root)
{
    xmlnode   *node;
    XmppBuddy *buddy;
    gchar     *value;
    gchar     *full_jid;
    gchar     *bare_jid;
    gchar     *show;
    gchar     *status;
    gchar     *photo;

    if (!xmlnode_has_prop(root, "from")) {
        hybrid_debug_error("xmpp", "invalid presence.");
        return;
    }

    full_jid = xmlnode_prop(root, "from");
    bare_jid = get_bare_jid(full_jid);

    if (xmlnode_has_prop(root, "type")) {

        value = xmlnode_prop(root, "type");
        if (g_strcmp0(value, "unavailable") == 0) {

            if (!(buddy = xmpp_buddy_find(stream->account, bare_jid))) {
                goto presence_over;
            }

            xmpp_buddy_set_show(buddy, full_jid, value);
            g_free(value);
            goto presence_over;

        } else if (g_strcmp0(value, "subscribed") == 0) {

            hybrid_message_box_show(HYBRID_MESSAGE_INFO,
                    "(<b>%s</b>) has accepted your request.", bare_jid);
            g_free(value);
            goto presence_over;

        } else if (g_strcmp0(value, "subscribe") == 0) {

            hybrid_buddy_request_window_create(stream->account->account,
                    full_jid, NULL);
            g_free(value);
            goto presence_over;
        }

        g_free(value);
    }

    if (!(buddy = xmpp_buddy_find(stream->account, bare_jid))) {
        goto presence_over;
    }

    /*
     * If the presence message doesn't have a <show> label,
     * then it means the current status of the buddy is 'avaiable'.
     */
    if ((node = xmlnode_find(root, "show"))) {

        show = xmlnode_content(node);
        xmpp_buddy_set_show(buddy, full_jid, show);
        g_free(show);

    } else {
        xmpp_buddy_set_show(buddy, full_jid, "avaiable");
    }

    if ((node = xmlnode_find(root, "status"))) {

        status = xmlnode_content(node);
        xmpp_buddy_set_status(buddy, full_jid, status);
        g_free(status);

    }
    /*
     * Check whether it has a photo label, then we can
     * determine whether to fetch the buddy's photo.
     */
    if ((node = xmlnode_find(root, "photo"))) {

        photo = xmlnode_content(node);

        if (g_strcmp0(photo, buddy->photo) != 0) {

            xmpp_buddy_set_photo(buddy, photo);
            xmpp_buddy_get_info(stream, buddy->jid,
                    (trans_callback)buddy_get_info_cb, buddy);
        }

        g_free(photo);
    }

presence_over:
    g_free(full_jid);
    g_free(bare_jid);
}

static void
xmpp_stream_process_message(XmppStream *stream, xmlnode *root)
{
    gchar         *value;
    gchar         *bare_jid;
    xmlnode       *node;
    HybridAccount *account;

    g_return_if_fail(stream != NULL);
    g_return_if_fail(root != NULL);

    account = stream->account->account;

    if (!xmlnode_has_prop(root, "type")) {
        hybrid_debug_error("xmpp",
                "invalid message received without a type property.");
        return;
    }

    value = xmlnode_prop(root, "type");

    if (g_strcmp0(value, "chat") != 0) {

        hybrid_debug_error("xmpp", "unsupported message type.");
        g_free(value);

        return;
    }

    g_free(value);

    if (!xmlnode_has_prop(root, "from")) {

        hybrid_debug_error("xmpp", "invalid message without a from property.");
        return;
    }

    value = xmlnode_prop(root, "from");
    bare_jid = get_bare_jid(value);
    g_free(value);

    if ((node = xmlnode_find(root, "composing"))) {
        hybrid_conv_got_inputing(account, bare_jid, FALSE);
    }

    if ((node = xmlnode_find(root, "active"))) {
        hybrid_conv_clear_inputing(account, bare_jid);
    }

    if ((node = xmlnode_find(root, "paused"))) {
        hybrid_conv_stop_inputing(account, bare_jid);
    }

    if ((node = xmlnode_find(root, "body"))) {

        value = xmlnode_content(node);
        hybrid_conv_got_message(account, bare_jid, value, time(NULL));
        g_free(value);

        return;
    }

}

static void
xmpp_stream_process_failure(XmppStream *stream, xmlnode *root)
{
    xmlnode       *node;
    HybridAccount *account;

    account = stream->account->account;

    if ((node = xmlnode_find(root, "not-authorized"))) {
        hybrid_account_error_reason(account,
                                    _("Account not authorized."
                                      " Check Username and Password."));
    }
}

void
xmpp_stream_process(XmppStream *stream, xmlnode *node)
{
    g_return_if_fail(stream != NULL);
    g_return_if_fail(node != NULL);

    if (g_strcmp0(node->name, "features") == 0) {
        xmpp_process_feature(stream, node);

    } else if (g_strcmp0(node->name, "proceed") == 0) {

        if (stream->state == XMPP_STATE_TLS_AUTHENTICATING) {
            xmpp_stream_performtls(stream);

        }

    } else if (g_strcmp0(node->name, "success") == 0) {

        if (stream->state == XMPP_STATE_SASL_AUTHENTICATING) {

            xmpp_stream_new_on_sasl(stream);
        }

    } else if (g_strcmp0(node->name, "iq") == 0) {

        xmpp_stream_process_iq(stream, node);

    } else if (g_strcmp0(node->name, "presence") == 0) {

        xmpp_stream_process_presence(stream, node);

    } else if (g_strcmp0(node->name, "message") == 0) {

        xmpp_stream_process_message(stream, node);

    } else if (g_strcmp0(node->name, "failure") == 0) {

        xmpp_stream_process_failure(stream, node);

        /* Here we only take login failure into consideration.
         so we just return without free the xml node, leaving it
        to be freed in the protocol's close() callback function. */
        return;
    }

    xmlnode_free(stream->node);
    stream->node = NULL;
}

static gchar*
generate_starttls_body(XmppStream *stream)
{
    xmlnode *node;
    gchar   *body;

    node = xmlnode_create("starttls");
    xmlnode_new_namespace(node, NULL, NS_XMPP_TLS);

    body = xmlnode_to_string(node);

    xmlnode_free(node);

    return body;
}

/**
 * Create the initiate stream string, we'll get:
 *
 * <stream:stream xmlns="jabber:client"
 * xmlns:stream="http://etherx.jabber.org/streams"
 * version="1.0" to="gmail.com">
 */
static gchar*
create_initiate_stream(XmppStream *stream)
{
    xmlnode *node;
    gchar   *version;
    gchar   *res;

    node = xmlnode_create("stream:stream");

    xmlnode_new_prop(node, "to", stream->account->to);

    xmlnode_new_namespace(node, NULL, "jabber:client");
    xmlnode_new_namespace(node, "stream",
                          "http://etherx.jabber.org/streams");

    version = g_strdup_printf("%d.%d",
                              stream->major_version,
                              stream->miner_version);

    xmlnode_new_prop(node, "version", version);

    g_free(version);

    res = xmlnode_to_string(node);
    xmpp_strip_end_label(res);
    xmlnode_free(node);

    return res;
}

void
xmpp_stream_get_account_info(XmppStream *stream)
{

}

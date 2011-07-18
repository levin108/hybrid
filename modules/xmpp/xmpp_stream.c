#include "util.h"
#include "connect.h"

#include "xmpp_util.h"
#include "xmpp_stream.h"
#include "xmpp_parser.h"
#include "xmpp_buddy.h"
#include "xmpp_iq.h"

static gchar *generate_starttls_body(XmppStream *stream);
static gchar *create_initiate_stream(XmppStream *xs);
static void xmpp_stream_get_roster(XmppStream *stream);

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

	stream->account = account;
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
	if (stream) {
		g_free(stream);
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
	guchar *auth;
	gchar *auth_encoded;
	gchar *xml_string;
	gint username_len;
	gint password_len;

	xmlnode *node;

	g_return_if_fail(stream != NULL);

	hybrid_debug_info("xmpp", "start sasl authentication.");

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


	auth_encoded = hybrid_base64(auth, username_len + password_len + 2);

	g_free(auth);

	/* construct the xml string, which is in form of:
	 *
	 * <auth xmlns="urn:ietf:params:xml:ns:xmpp-sasl" 
	 * mechanism="PLAIN">encoded sasl string</auth> 
	 */
	node = xmlnode_create("auth");

	xmlnode_new_namespace(node, NULL, SASL_NAMESPACE);
	xmlnode_new_prop(node, "mechanism", "PLAIN");
	xmlnode_set_content(node, auth_encoded);

	g_free(auth_encoded);
	
	xml_string = xmlnode_to_string(node);
	xmlnode_free(node);

	hybrid_debug_info("xmpp", "sasl send:\n%s", xml_string);

	if (hybrid_ssl_write(stream->ssl, xml_string,
				strlen(xml_string)) == -1) {

		hybrid_account_error_reason(stream->account->account,
				"SASL authentication error.");

		return;
	}

	g_free(xml_string);

}

static gboolean
stream_recv_cb(gint sk, XmppStream *stream)
{
	gchar buf[BUF_LENGTH];
	gint n;

	if (!stream->ssl) {

		if ((n = recv(sk, buf, sizeof(buf) - 1, 0)) == -1) {
			
			hybrid_debug_error("xmpp", "init stream error.");

			return FALSE;
		}

	} else {
		if ((n = hybrid_ssl_read(stream->ssl, buf, sizeof(buf) - 1)) == -1) {
			
			return TRUE;
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
				"send initial jabber request failed");

		return FALSE;
	}

	/* send initiate stream request. */
	msg = create_initiate_stream(stream);

	hybrid_debug_info("xmpp", "send initite jabber stream:\n%s", msg);

	if (send(sk, msg, strlen(msg), 0) == -1) {

		hybrid_account_error_reason(stream->account->account,
				"send initial jabber request failed");
		g_free(msg);

		return FALSE;
	}

	g_free(msg);

	hybrid_event_add(sk, HYBRID_EVENT_READ,
			(input_func)stream_recv_cb, stream);

	return FALSE;
}

/**
 * Ok, TLS handshake success, we will continue the authtication
 * through the established TLS channel.
 */
static gboolean
tls_conn_cb(HybridSslConnection *ssl, XmppStream *stream)
{
	gchar *msg;

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
				"send initial jabber request failed");
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

	if (!(stream->ssl = hybrid_ssl_connect_with_fd(stream->sk,
					(ssl_callback)tls_conn_cb, stream))) {

		hybrid_account_error_reason(stream->account->account, 
				_("TLS hand-shake failed."));

		return;
	}
}

/**
 * Callback function for the start session request to process the response.
 */
static gboolean
start_session_cb(XmppStream *stream, xmlnode *root, gpointer user_data)
{
	xmpp_stream_get_roster(stream);
	
	return TRUE;
}

/**
 * Callback function for the resource bind request to process the response.
 */
static gboolean
resource_bind_cb(XmppStream *stream, xmlnode *root, gpointer user_data)
{
	gchar *type;
	gchar *jid;
	xmlnode *node;

	type = xmlnode_prop(root, "type");

	if (g_strcmp0(type, "result")) {
		hybrid_account_error_reason(stream->account->account,
				_("resource bind error."));
		return FALSE;
	}

	if (!(node = xmlnode_find(root, "jid"))) {
		hybrid_account_error_reason(stream->account->account,
				_("resource bind error: no jabber id returned."));
		return FALSE;
	}

	jid = xmlnode_content(node);
	xmpp_stream_set_jid(stream, jid);
	g_free(jid);

	/* request to start a session. */
	IqRequest *iq;

	iq = iq_request_create(stream, IQ_TYPE_SET);

	node = xmlnode_new_child(iq->node, "session");
	xmlnode_new_namespace(node, NULL, SESSION_NAMESPACE);

	iq_request_set_callback(iq, start_session_cb, NULL);

	if (iq_request_send(iq) != HYBRID_OK) {

		hybrid_account_error_reason(stream->account->account,
				_("start session error."));

		return TRUE;
	}

	return TRUE;
}

/**
 * Client binds a resource.
 */
static void
xmpp_stream_bind(XmppStream *stream)
{
	xmlnode *root;
	xmlnode *node;
	IqTransaction *trans;
	gchar *iq_id;
	gchar *xml_string;

	g_return_if_fail(stream != NULL);

	xmpp_stream_iqid_increase(stream);

	root = xmlnode_create("iq");

	xmlnode_new_prop(root, "type", "set");

	iq_id = xmpp_stream_get_iqid(stream);
	xmlnode_new_prop(root, "id", iq_id);
	g_free(iq_id);

	trans = iq_transaction_create(stream->current_iq_id);
	iq_transaction_set_callback(trans, resource_bind_cb, NULL);
	iq_transaction_add(stream, trans);

	node = xmlnode_new_child(root, "bind");
	xmlnode_new_namespace(node, NULL, BIND_NAMESPACE);

	xml_string = xmlnode_to_string(root);
	xmlnode_free(root);

	hybrid_debug_info("xmpp", "binds a resource,send:\n%s", xml_string);

	if (hybrid_ssl_write(stream->ssl, xml_string, strlen(xml_string)) == -1) {

		hybrid_account_error_reason(stream->account->account,
				"binds a resource error.");

		g_free(xml_string);

		return;
	}
	
	g_free(xml_string);
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
	xmlnode *root;
	xmlnode *node;
	gchar *iqid;
	gchar *xml_string;
	IqTransaction *trans;

	g_return_if_fail(stream != NULL);

	xmpp_stream_iqid_increase(stream);

	iqid = xmpp_stream_get_iqid(stream);

	root = xmlnode_create("iq");

	xmlnode_new_prop(root, "from", stream->jid);
	xmlnode_new_prop(root, "id", iqid);
	xmlnode_new_prop(root, "type", "get");

	g_free(iqid);

	trans = iq_transaction_create(stream->current_iq_id);
	iq_transaction_set_callback(trans, request_roster_cb, NULL);
	iq_transaction_add(stream, trans);


	node = xmlnode_new_child(root, "query");
	xmlnode_new_namespace(node, NULL, ROSTER_NAMESPACE);
#if 0
	xmlnode_new_namespace(node, "gr", NS_GOOGLE_ROSTER);
	xmlnode_new_prop(node, "gr:ext", "2");
#endif

	xml_string = xmlnode_to_string(root);
	xmlnode_free(root);

	hybrid_debug_info("xmpp", "binds a resource,send:\n%s", xml_string);

	if (hybrid_ssl_write(stream->ssl, xml_string, strlen(xml_string)) == -1) {

		hybrid_account_error_reason(stream->account->account,
				"binds a resource error.");

		g_free(xml_string);

		return;
	}

	g_free(xml_string);
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
				"start new stream on sasl layer error.");
		return;
	}

	g_free(msg);
}

/**
 * Process the iq response.
 */
static void
xmpp_stream_process_iq(XmppStream *stream, xmlnode *node)
{
	gchar *id;
	gint id_int;
	GSList *pos;
	IqTransaction *trans;

	g_return_if_fail(stream != NULL);
	g_return_if_fail(node != NULL);

	if (!xmlnode_has_prop(node, "type") ||
		!xmlnode_has_prop(node, "id")) {
		hybrid_debug_error("xmpp", "invalid iq response.");

		return;
	}

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
}

static void
xmpp_stream_process_presence(XmppStream *stream, xmlnode *root)
{
	xmlnode *node;
	XmppBuddy *buddy;
	gchar *jid;
	gchar *bare_jid;
	gchar *show;
	gchar *status;

	if (!xmlnode_has_prop(root, "from")) {
		hybrid_debug_error("xmpp", "invalid presence.");
		return;
	}

	jid = xmlnode_prop(root, "from");
	bare_jid = get_bare_jid(jid);
	g_free(jid);

	if (!(buddy = xmpp_buddy_find(bare_jid))) {
		return;
	}

	g_print("%s, %s\n", root->name, xmlnode_to_string(root));

	if ((node = xmlnode_find(root, "show"))) {

		show = xmlnode_content(node);
		xmpp_buddy_set_show(buddy, show);
		g_free(show);

	} else {
		xmpp_buddy_set_show(buddy, "avaiable");
	}

	if ((node = xmlnode_find(root, "status"))) {

		status = xmlnode_content(node);
		xmpp_buddy_set_status(buddy, status);
		g_free(status);
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
	}
}

static gchar*
generate_starttls_body(XmppStream *stream)
{
	xmlnode *node;
	gchar *body;

	node = xmlnode_create("starttls");
	xmlnode_new_namespace(node, NULL, TLS_NAMESPACE);

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
	gchar *version;
	gchar *res;

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


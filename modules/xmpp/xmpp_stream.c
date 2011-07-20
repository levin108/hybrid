#include "util.h"
#include "connect.h"
#include "conv.h"

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

static gchar*
get_resource(const gchar *full_jid)
{
	gchar *pos;

	for (pos = (gchar *)full_jid; *pos && *pos != '/'; pos ++);

	pos ++;

	return g_strndup(pos, full_jid + strlen(full_jid) - pos);
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
	IqTransaction *trans;

	if (stream) {
		g_free(stream->jid);
		g_free(stream->stream_id);
		xmlnode_free(stream->node);
		xmpp_account_destroy(stream->account);
		
		while (stream->pending_trans) {
			trans = (IqTransaction*)stream->pending_trans->data;
			iq_transaction_remove(stream, trans);
		}

		hybrid_connection_destroy(stream->conn);
		hybrid_ssl_connection_destory(stream->ssl);

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


	auth_encoded = hybrid_base64_encode(auth, username_len + password_len + 2);

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

	stream->source = hybrid_event_add(sk, HYBRID_EVENT_READ,
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
	xmpp_stream_get_roster(stream);
}

static gboolean
account_get_info_cb(XmppStream *stream, xmlnode *root, gpointer user_data)
{
	xmlnode *node;
	gchar *value;
	guchar *photo;
	gint photo_len;

	HybridAccount *account;

	account = stream->account->account;

	if (xmlnode_has_prop(root, "type")) {
		value = xmlnode_prop(root, "type");

		if (g_strcmp0(value, "result") != 0) {

			hybrid_account_error_reason(account, _("Login failed."));
			g_free(value);

			return FALSE;
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
			hybrid_account_set_icon(account, photo,	photo_len, "");

			g_free(value);
			g_free(photo);
		}
	}

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
	
	if (!account->nickname && !account->icon_crc) {
		/*
		 * Both the nickname and the icon checksum of the account
		 * is NULL, it means that this is the first time this account
		 * was enabled, so we need to fetch the account's information
		 * first before we finished logining.
		 */
		xmpp_buddy_get_info(stream, stream->account->username,
				(trans_callback)account_get_info_cb, NULL);

	} else {

		auth_success(stream);
	}
	
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
	xmlnode *node;
	IqRequest *iq;

	g_return_if_fail(stream != NULL);

	iq = iq_request_create(stream, IQ_TYPE_SET);

	node = xmlnode_new_child(iq->node, "bind");
	xmlnode_new_namespace(node, NULL, BIND_NAMESPACE);

	iq_request_set_callback(iq, resource_bind_cb, NULL);

	if (iq_request_send(iq) != HYBRID_OK) {

		hybrid_account_error_reason(
				stream->account->account,
				"binds a resource error.");

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
	xmlnode *node;
	IqRequest *iq;

	g_return_if_fail(stream != NULL);

	xmpp_stream_iqid_increase(stream);

	iq = iq_request_create(stream, IQ_TYPE_GET);
	iq_request_set_callback(iq, request_roster_cb, NULL);

	node = xmlnode_new_child(iq->node, "query");
	xmlnode_new_namespace(node, NULL, ROSTER_NAMESPACE);

#if 0
	xmlnode_new_namespace(node, "gr", NS_GOOGLE_ROSTER);
	xmlnode_new_prop(node, "gr:ext", "2");
#endif

	if (iq_request_send(iq) != HYBRID_OK) {

		hybrid_account_error_reason(
				stream->account->account,
				"binds a resource error.");
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

/**
 * Callback function to process the buddy's get-info response.
 */
static gboolean
buddy_get_info_cb(XmppStream *stream, xmlnode *root, XmppBuddy *buddy)
{
	xmlnode *node;
	gchar *type;
	gchar *photo_bin;
	guchar *photo;
	gint photo_len;

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
	xmlnode *node;
	XmppBuddy *buddy;
	gchar *value;
	gchar *resource;
	gchar *bare_jid;
	gchar *show;
	gchar *status;
	gchar *photo;

	if (!xmlnode_has_prop(root, "from")) {
		hybrid_debug_error("xmpp", "invalid presence.");
		return;
	}

	value = xmlnode_prop(root, "from");
	bare_jid = get_bare_jid(value);
	resource = get_resource(value);
	g_free(value);

	if (!(buddy = xmpp_buddy_find(bare_jid))) {

		g_free(bare_jid);
		g_free(resource);

		return;
	}

	xmpp_buddy_set_resource(buddy, resource);

	g_free(resource);
	g_free(bare_jid);

	if (xmlnode_has_prop(root, "type")) {

		value = xmlnode_prop(root, "type");
		if (g_strcmp0(value, "unavailable") == 0) {

			xmpp_buddy_set_show(buddy, value);
			g_free(value);

			return;
		}
		g_free(value);
	}

	/*
	 * If the presence message doesn't have a <show> label,
	 * then it means the current status of the buddy is 'avaiable'.
	 */
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
}

static void
xmpp_stream_process_message(XmppStream *stream, xmlnode *root)
{
	gchar *value;
	gchar *bare_jid;
	xmlnode *node;

	g_return_if_fail(stream != NULL);
	g_return_if_fail(root != NULL);

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

	if (!(node = xmlnode_find(root, "body"))) {
		hybrid_debug_error("xmpp", "invalid message without a body.");
		return;
	}

	value = xmlnode_content(node);

	hybrid_conv_got_message(stream->account->account,
			bare_jid, value, time(NULL));

	g_free(value);

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

void
xmpp_stream_get_account_info(XmppStream *stream)
{

}

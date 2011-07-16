#include "util.h"
#include "connect.h"

#include "xmpp_util.h"
#include "xmpp_stream.h"

static gchar *generate_starttls_body(XmppStream *stream);
static gchar *create_initiate_stream(XmppStream *xs);

XmppStream*
xmpp_stream_create(void)
{
	XmppStream *stream;

	stream = g_new0(XmppStream, 1);

	stream->major_version = 1;
	stream->miner_version = 0;

	return stream;
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

	body = generate_starttls_body(stream);

	if (send(stream->sk, body, strlen(body), 0) == -1) {

		hybrid_debug_error("stream", "start tls error.");

		return;
	}

	g_free(body);
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
	g_free(stream->stream_id);
	stream->stream_id = NULL;

	hybrid_debug_info("stream", "send start stream request:\n%s", msg);

	if (hybrid_ssl_write(ssl, msg, strlen(msg)) == -1) {

		hybrid_account_error_reason(stream->account,
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

		hybrid_account_error_reason(stream->account, 
				_("TLS hand-shake failed."));

		return;
	}
}

void
xmpp_stream_process(XmppStream *stream, xmlnode *node)
{
	if (g_strcmp0(node->name, "features") == 0) {
		xmpp_stream_starttls(stream);

	} else if (g_strcmp0(node->name, "proceed") == 0) {
		xmpp_stream_performtls(stream);
	}
}

static gchar*
generate_starttls_body(XmppStream *stream)
{
	xmlnode *node;
	gchar *body;

	node = xmlnode_create("starttls");
	xmlnode_new_namespace(node, NULL, "urn:ietf:params:xml:ns:xmpp-tls");

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

	xmlnode_new_prop(node, "to", stream->to);

	xmlnode_new_namespace(node, NULL, "jabber:client");
	xmlnode_new_namespace(node, "stream", "http://etherx.jabber.org/streams");

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

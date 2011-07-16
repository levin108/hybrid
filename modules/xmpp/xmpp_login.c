#include "util.h"
#include "connect.h"
#include "eventloop.h"
#include "xmlnode.h"

#include "xmpp_util.h"
#include "xmpp_login.h"
#include "xmpp_parser.h"

static gchar *create_initiate_stream(XmppStream *xs);

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
			
			hybrid_debug_error("xmpp", "init stream error.");

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
stream_init(gint sk, XmppStream *stream)
{
	gchar *msg;

	stream->sk = sk;

	/* send version. */
	msg = "<?xml version='1.0' ?>";

	if (send(sk, msg, strlen(msg), 0) == -1) {

		hybrid_account_error_reason(stream->account,
				"send initial jabber request failed");

		return FALSE;
	}

	/* send initiate stream request. */
	msg = create_initiate_stream(stream);

	hybrid_debug_info("xmpp", "send initite jabber stream:\n%s", msg);

	if (send(sk, msg, strlen(msg), 0) == -1) {

		hybrid_account_error_reason(stream->account,
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

	xmlnode_new_namespace(node, NULL, "jabber:client");
	xmlnode_new_namespace(node, "stream", "http://etherx.jabber.org/streams");

	version = g_strdup_printf("%d.%d",
				stream->major_version,
				stream->miner_version);

	xmlnode_new_prop(node, "version", version);
	xmlnode_new_prop(node, "to", "gmail.com");

	g_free(version);

	res = xmlnode_to_string(node);
	xmpp_strip_end_label(res);
	xmlnode_free(node);

	return res;
}

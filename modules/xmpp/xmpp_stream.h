#ifndef HYBRID_XMPP_STREAM_H
#define HYBRID_XMPP_STREAM_H
#include <glib.h>
#include "xmlnode.h"
#include "connect.h"
#include "account.h"

#include "xmpp_account.h"

typedef struct _XmppStream XmppStream;

struct _XmppStream {
	gint sk;  /**< the socket descriptor. */

	gchar *stream_id;
	gint major_version; /**< default 1 */
	gint miner_version; /**< default 0 */

	xmlnode *node;

	XmppAccount *account;

	HybridConnection *conn;
	HybridSslConnection *ssl;

	xmlParserCtxt *xml_ctxt;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the jabber stream.Note that this is
 * a callback function.
 *
 * @param sk     The socket destriptor.
 * @param stream The xmpp stream.
 */
gboolean xmpp_stream_init(gint sk, XmppStream *stream);

/**
 * Create a xmpp stream.
 *
 * @param account The xmpp account context.
 *
 * @return The xmpp stream created.
 */
XmppStream *xmpp_stream_create(XmppAccount *account);

/**
 * Destroy a xmpp stream.
 *
 * @param steam The stream to destroy.
 */
void xmpp_stream_destroy(XmppStream *stream);

/**
 * Process a xml packet.
 *
 * @param stream The xmpp stream context.
 * @param node   The root node of the xml packet.
 */
void xmpp_stream_process(XmppStream *stream, xmlnode *node);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_XMPP_STREAM_H */

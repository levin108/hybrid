#ifndef HYBRID_XMPP_STREAM_H
#define HYBRID_XMPP_STREAM_H
#include <glib.h>
#include <libxml/parser.h>

typedef struct _XmppStream XmppStream;

struct _XmppStream {
	gint sk;  /**< the socket descriptor. */

	gint major_version; /* default 1 */
	gint miner_version; /* default 0 */

	xmlParserCtxt *xml_ctxt;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a xmpp stream.
 *
 * @return The xmpp stream created.
 */
XmppStream *xmpp_stream_create(void);

/**
 * Destroy a xmpp stream.
 *
 * @param steam The stream to destroy.
 */
void xmpp_stream_destroy(XmppStream *stream);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_XMPP_STREAM_H */

#ifndef HYBRID_XMPP_LOGIN_H
#define HYBRID_XMPP_LOGIN_H
#include "xmpp_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the jabber stream.
 *
 * @param sk     The socket destriptor.
 * @param stream The xmpp stream.
 */
gboolean stream_init(gint sk, XmppStream *stream);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_XMPP_LOGIN_H */

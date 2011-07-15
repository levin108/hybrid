#ifndef HYBRID_XMPP_LOGIN_H
#define HYBRID_XMPP_LOGIN_H
#include "xmpp_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Callback function of the connect event.
 *
 * @param sk The socket destriptor.
 * @param xs The xmpp stream.
 */
gboolean init_connect(gint sk, XmppStream *xs);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_XMPP_LOGIN_H */

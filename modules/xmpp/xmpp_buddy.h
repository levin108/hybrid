#ifndef HYBRID_XMPP_BUDDY_H
#define HYBRID_XMPP_BUDDY_H

#include "xmpp_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Process the roster returned from the server.
 *
 * @param stream The xmpp stream.
 * @param root   The root node of the roster xml context.
 */
void xmpp_buddy_process_roster(XmppStream *stream, xmlnode *root);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_XMPP_BUDDY_H */

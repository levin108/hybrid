#ifndef HYBRID_XMPP_PARSER_H
#define HYBRID_XMPP_PARSER_H

#include "xmpp_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Process the pushed messages from the server.
 *
 * @param stream The stream context.
 * @param buffer The message buffer.
 * @param len    Size of the buffer.
 */
void xmpp_process_pushed(XmppStream *stream, const gchar *buffer, gint len);

#ifdef __cplusplus
}
#endif

#endif

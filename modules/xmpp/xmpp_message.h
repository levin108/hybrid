#ifndef HYBRID_XMPP_MESSAGE_H
#define HYBRID_XMPP_MESSAGE_H
#include <glib.h>
#include "xmpp_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set a message with type 'chat' to a specified buddy.
 *
 * @param stream The xmpp stream.
 * @param text   The contenxt of the message.
 * @param to     Bare jid of the receiver.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_message_send(XmppStream *stream, const gchar *text, const gchar *to);

/**
 * Set typing message to a specified buddy.
 *
 * @param stream The xmpp stream.
 * @param to     Bare jid of the receiver.
 * @param state  State of the typeing action.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_message_send_typing(XmppStream *stream, const gchar *to,
		HybridInputState state);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_XMPP_MESSAGE_H */

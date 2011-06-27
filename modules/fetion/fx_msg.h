#ifndef HYBRID_FX_MSG_H
#define HYBRID_FX_MSG_H

#include <gtk/gtk.h>
#include "fx_account.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Parse the system message. The output is the content 
 * and a url to the website the message refered to.
 *
 * @param sipmsg  The sip message of the system message.
 * @param content The content of the message to be set.
 * @param url     The url of the url to be set.
 */
gint fetion_message_parse_sysmsg(const gchar *sipmsg,
		gchar **content, gchar **url);

/**
 * Send a text message to a buddy who is invisible or offline.
 *
 * @param account The fetion account.
 * @param userid  The receiver's userid.
 * @param text    The content of the message.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint fetion_message_send(fetion_account *account, const gchar *userid,
						const gchar *text);

/**
 * Process the received message, send back a response message, and show
 * the message in the chat panel.
 *
 * @param account The fetion account.
 * @param sipmsg  The sip message.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint fetion_process_message(fetion_account *account, const gchar *sipmsg);

/**
 * Start a new chat channel, and send the text through the new channel.
 * The message is:
 *
 * S fetion.com.cn SIP-C/4.0
 * F: 547264589
 * I: 4
 * Q: 2 S
 * N: StartChat
 *
 * @param account The fetion account.
 * @param userid  The receiver buddy's userid.
 * @param text    The content of the message.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint fetion_message_new_chat(fetion_account *account, const gchar *userid,
								const gchar *text);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_FX_MSG_H */

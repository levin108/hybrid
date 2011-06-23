#ifndef HYBRID_FX_MSG_H
#define HYBRID_FX_MSG_H

#include <gtk/gtk.h>

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

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_FX_MSG_H */

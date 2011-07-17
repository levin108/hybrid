#ifndef HYBRID_XMPP_UTIL_H
#define HYBRID_XMPP_UTIL_H
#include <glib.h>
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Strip the '/' from a label,make it an unclosed xml label.
 */
void xmpp_strip_end_label(gchar *xml_string);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_XMPP_UTIL_H */

#include <glib.h>

#ifndef HYBRID_DEBUG_H
#define HYBRID_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

void hybrid_debug_info(const gchar *relm, const gchar *format, ...);

void hybrid_debug_error(const gchar *relm, const gchar *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_DEBUG_H */

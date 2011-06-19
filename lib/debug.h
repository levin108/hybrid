#include <glib.h>

#ifndef HYBIRD_DEBUG_H
#define HYBIRD_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

void hybird_debug_info(const gchar *relm, const gchar *format, ...);

void hybird_debug_error(const gchar *relm, const gchar *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* HYBIRD_DEBUG_H */

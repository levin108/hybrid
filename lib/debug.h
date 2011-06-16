#include <glib.h>

#ifndef Hybird_DEBUG_H
#define Hybird_DEBUG_H

void hybird_debug_info(const gchar *relm, const gchar *format, ...);

void hybird_debug_error(const gchar *relm, const gchar *format, ...);

#endif

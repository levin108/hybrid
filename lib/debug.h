#include <glib.h>

#ifndef IM_DEBUG_H
#define IM_DEBUG_H

void im_debug_info(const gchar *relm, const gchar *format, ...);

void im_debug_error(const gchar *relm, const gchar *format, ...);

#endif

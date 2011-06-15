#include "debug.h"
#include "util.h"

void 
im_debug_info(const gchar *relm, const gchar *format, ...)
{
	gchar fmt[4096];
	va_list ap;

	snprintf(fmt, sizeof(fmt) - 1, "INFO <%s> %s\n", relm, format);
	va_start(ap, format);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
}

void 
im_debug_error(const gchar *relm, const gchar *format, ...)
{
	gchar fmt[4096];
	va_list ap;

	snprintf(fmt, sizeof(fmt) - 1, "ERROR *** <%s> *** %s\n", relm, format);
	va_start(ap, format);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
}

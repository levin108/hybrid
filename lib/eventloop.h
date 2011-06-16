#ifndef Hybird_EVENTLOOP_H
#define Hybird_EVENTLOOP_H

#include <glib.h>

#include "connect.h"

#define Hybird_EVENT_READ  (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define Hybird_EVENT_WRITE (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

typedef gboolean (*input_func)(gint sk, gpointer user_data);

#ifdef __cplusplus
extern "C" {
#endif
	
/**
 * Add an read event handler.
 *
 * @param sk The input socket descriptor.
 * @param func The callback function.
 * @param user_data User-specified data to pass to func.
 *
 * @return The event source id.
 */
guint hybird_event_add(gint sk, gint event_type, input_func func,
		gpointer user_data);

/**
 * Add an ssl read event handler.
 *
 * @param isc The ssl context.
 * @param func The callback function.
 * @param user_data User-specified data to pass to func.
 *
 * @return The event source id.
 */
guint hybird_ssl_event_add(HybirdSslConnection *isc, ssl_callback func,
		gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif /* Hybird_EVENTLOOP_H */

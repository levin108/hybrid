#ifndef HYBRID_STATUSICON_H
#define HYBRID_STATUSICON_H
#include <gtk/gtk.h>

#include "blist.h"

typedef struct _HybridStatusIcon HybridStatusIcon;

struct _HybridStatusIcon {
	GtkStatusIcon *icon;
	gint conn_id;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the status icon.
 */
void hybrid_status_icon_init(void);

/**
 * Set the status icon blinking with the icon of the given buddy.
 *
 * @param The buddy whose icon will be blinking in the status icon.
 */
void hybrid_status_icon_blinking(HybridBuddy *buddy);

#ifdef __cplusplus
}
#endif

#endif

#ifndef HYBRID_BUDDYADD_H
#define HYBRID_BUDDYADD_H

#include <gtk/gtk.h>

typedef struct _HybridBuddyAddWindow HybridBuddyAddWindow;

struct _HybridBuddyAddWindow {
	GtkWidget *window;
	GtkWidget *account_combo;
	GtkWidget *group_combo;
	GtkWidget *username_entry;
	GtkWidget *localname_entry;
	GtkWidget *tips_textview;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a buddy-add window.
 *
 * @return The buddy-add window created.
 */
HybridBuddyAddWindow *hybrid_buddyadd_window_create();

#ifdef __cplusplus
}
#endif

#endif

#ifndef HYBRID_PREFERENCE_H
#define HYBRID_PREFERENCE_H
#include <gtk/gtk.h>

typedef struct _HybridPreference HybridPreference;

struct _HybridPreference {
	GtkWidget *window;
	GtkWidget *notebook;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create the preference window, if exists, just present the window.
 */
void hybrid_pref_create(void);

#ifdef __cplusplus
}
#endif

#endif

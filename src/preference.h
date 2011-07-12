#ifndef HYBRID_PREFERENCE_H
#define HYBRID_PREFERENCE_H
#include <gtk/gtk.h>

typedef struct _HybridPreference HybridPreference;

struct _HybridPreference {
	GtkWidget *window;
	GtkWidget *notebook;
	/*
	 * check button for mute.
	 */
	GtkWidget *mute_check;
	/*
	 * check button for whether to hide the send buttons 
	 * or not int the bottom of the chat window.
	 */
	GtkWidget *hcb_check; 

	/*
	 * check button for whether to show the chat dialogs
	 * in a single window with tabs.
	 */
	GtkWidget *single_cw_check;

	/*
	 * Combo box for choosing the position of the tabs;
	 */
	GtkWidget *position_of_tabs;
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

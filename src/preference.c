#include "preference.h"
#include "util.h"
#include "gtkutils.h"

static HybridPreference *pref_window = NULL;

/**
 * Initialize the basic settings page.
 */
void
pref_basic_init(GtkFixed *fixed)
{
	
}

/**
 * Initialize the preference window.
 */
void
pref_window_init(void)
{
	GtkWidget *fixed;
	GtkWidget *label;

	g_return_if_fail(pref_window != NULL);

	pref_window->notebook = gtk_notebook_new();

	gtk_container_add(GTK_CONTAINER(pref_window->window),
			pref_window->notebook);
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(pref_window->notebook), GTK_POS_TOP);
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(pref_window->notebook), TRUE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(pref_window->notebook), TRUE);

	fixed = gtk_fixed_new();
	label = gtk_label_new(_("Basic Settings"));
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_window->notebook),
	                         fixed, label);

	pref_basic_init(GTK_FIXED(fixed));
}

/**
 * Callback function for destroying the preference window.
 */
void
destroy_cb(GtkWidget *widget, gpointer user_data)
{
	if (pref_window) {
		g_free(pref_window);
		pref_window = NULL;
	}
}

void
hybrid_pref_create(void)
{
	if (pref_window) {

		gtk_window_present(GTK_WINDOW(pref_window->window));

		return;
	}

	pref_window = g_new0(HybridPreference, 1);

	pref_window->window = hybrid_create_window(_("Preference"), NULL,
	                                    GTK_WIN_POS_CENTER, FALSE);

	g_signal_connect(pref_window->window, "destroy",
	                 G_CALLBACK(destroy_cb), NULL);

	gtk_widget_set_usize(pref_window->window, 500, 400);
	gtk_container_set_border_width(GTK_CONTAINER(pref_window->window), 8);

	pref_window_init();

	gtk_widget_show_all(pref_window->window);
}

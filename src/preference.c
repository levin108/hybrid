#include "pref.h"
#include "util.h"

#include "preference.h"
#include "gtkutils.h"

static HybridPreference *pref_window = NULL;

/**
 * Initialize the basic settings page.
 */
void
pref_basic_init(GtkFixed *fixed)
{
	GtkWidget *label;

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), _("<b>Appearance</b>"));

	gtk_fixed_put(fixed, label, 20, 10);

	pref_window->mute_check = gtk_check_button_new_with_label(_("Mute"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_window->mute_check),
			hybrid_pref_get_boolean("mute"));
	gtk_fixed_put(fixed, pref_window->mute_check, 20, 35);

	pref_window->hcb_check = 
		gtk_check_button_new_with_label(_("Hide Send Buttons"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_window->hcb_check),
			hybrid_pref_get_boolean("hide_chat_buttons"));
	gtk_fixed_put(fixed, pref_window->hcb_check, 220, 35);

	pref_window->single_cw_check =
		gtk_check_button_new_with_label(_("Show Messages In A Single Window With Tabs"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_window->single_cw_check),
			hybrid_pref_get_boolean("single_chat_window"));
	gtk_fixed_put(fixed, pref_window->single_cw_check, 20, 65);

}

/**
 * Callback function for the cancel button.
 */
static void
cancel_cb(GtkWidget *widget, gpointer user_data)
{
	g_return_if_fail(pref_window != NULL);

	gtk_widget_destroy(pref_window->window);
}

/**
 * Callback function for the save button.
 */
static void
save_cb(GtkWidget *widget, gpointer user_data)
{
	g_return_if_fail(pref_window != NULL);

	if (gtk_toggle_button_get_active(
				GTK_TOGGLE_BUTTON(pref_window->mute_check))) {

		hybrid_pref_set_boolean("mute", TRUE);

	} else {
		hybrid_pref_set_boolean("mute", FALSE);
	}

	if (gtk_toggle_button_get_active(
				GTK_TOGGLE_BUTTON(pref_window->hcb_check))) {

		hybrid_pref_set_boolean("hide_chat_buttons", TRUE);

	} else {
		hybrid_pref_set_boolean("hide_chat_buttons", FALSE);
	}

	if (gtk_toggle_button_get_active(
				GTK_TOGGLE_BUTTON(pref_window->single_cw_check))) {

		hybrid_pref_set_boolean("single_chat_window", TRUE);

	} else {
		hybrid_pref_set_boolean("single_chat_window", FALSE);
	}

	hybrid_pref_save();

	gtk_widget_destroy(pref_window->window);
}

/**
 * Initialize the preference window.
 */
void
pref_window_init(void)
{
	GtkWidget *fixed;
	GtkWidget *label;
	GtkWidget *vbox;
	GtkWidget *action_area;
	GtkWidget *button;

	g_return_if_fail(pref_window != NULL);

	pref_window->notebook = gtk_notebook_new();

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(pref_window->window), vbox);

	gtk_box_pack_start(GTK_BOX(vbox), pref_window->notebook, TRUE, TRUE, 0);

	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(pref_window->notebook), GTK_POS_TOP);
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(pref_window->notebook), TRUE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(pref_window->notebook), TRUE);

	fixed = gtk_fixed_new();
	label = gtk_label_new(_("Basic Settings"));
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_window->notebook),
	                         fixed, label);

	pref_basic_init(GTK_FIXED(fixed));

	action_area = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), action_area, FALSE, FALSE, 5);

	button = gtk_button_new_with_label(_("Save"));
	gtk_widget_set_size_request(button, 100, 30);
	gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 0);
	g_signal_connect(button, "clicked", G_CALLBACK(save_cb), NULL);

	button = gtk_button_new_with_label(_("Cancel"));
	gtk_widget_set_size_request(button, 100, 30);
	gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
	g_signal_connect(button, "clicked", G_CALLBACK(cancel_cb), NULL);

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

	gtk_widget_set_size_request(pref_window->window, 450, 300);
	gtk_container_set_border_width(GTK_CONTAINER(pref_window->window), 8);

	pref_window_init();

	gtk_widget_show_all(pref_window->window);
}

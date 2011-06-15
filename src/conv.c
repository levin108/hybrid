#include "gtkutils.h"
#include "conv.h"
#include "util.h"

/* The list of the currently opened conversation dialogs. */
GSList *conv_list = NULL; 

/**
 * Callback function to handle the close button click event.
 */
static void
conv_close_cb(GtkWidget *widget, gpointer user_data)
{
	IMConversation *conv;

	conv = (IMConversation*)user_data;

	gtk_widget_destroy(conv->window);
}

/**
 * Callback function to handle the conversation window destroy event.
 */
static void
conv_destroy_cb(GtkWidget *widget, gpointer user_data)
{
	IMConversation *conv = (IMConversation*)user_data;
	GSList *pos;
	IMChatPanel *temp_chat;

	/* First we should free the memory in the list of IMChatPanel. */
	while (conv->chat_buddies) {
		pos = conv->chat_buddies;
		temp_chat = (IMChatPanel*)pos->data;
		conv->chat_buddies = g_slist_remove(conv->chat_buddies, pos->data);
		g_free(temp_chat);
	}

	conv_list = g_slist_remove(conv_list, conv);
	g_free(conv);
}

static void
switch_page_cb(GtkNotebook *notebook, gpointer newpage, guint newpage_nth,
		gpointer user_data)
{
	GSList *pos;
	IMChatPanel *chat;
	IMBuddy *buddy;
	GdkPixbuf *pixbuf;
	IMConversation *conv = (IMConversation*)user_data;	

	for (pos = conv->chat_buddies; pos; pos = pos->next) {
		chat = (IMChatPanel*)pos->data;

		if (chat->page_index == newpage_nth) {
			goto page_found;
		}
	}

	im_debug_error("conv", "FATAL, can not find an exist buddy\n");

	return;

page_found:
	buddy = chat->buddy;

	/* Set the conversation window's icon. */
	pixbuf = create_pixbuf(buddy->icon_data, buddy->icon_data_length);
	gtk_window_set_icon(GTK_WINDOW(conv->window), pixbuf);
	g_object_unref(pixbuf);

	/* Set the conversation window's title */
	gtk_window_set_title(GTK_WINDOW(conv->window), 
		(!buddy->name || *(buddy->name) == '\0') ? buddy->id : buddy->name);
}

/**
 * Create a new IM Conversation Dialog.
 */
static IMConversation*
im_conv_create()
{
	GtkWidget *vbox;
	GtkWidget *action_area;
	GtkWidget *halign;
	GtkWidget *button;

	IMConversation *imconv;

	imconv = g_new0(IMConversation, 1);

	/* create window */
	imconv->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(imconv->window), 550, 500);
	gtk_container_set_border_width(GTK_CONTAINER(imconv->window), 5);
	g_signal_connect(imconv->window, "destroy", G_CALLBACK(conv_destroy_cb),
			imconv);

	/* create vbox */
	vbox = gtk_vbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(imconv->window), vbox);

	/* create notebook */
	imconv->notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(imconv->notebook), GTK_POS_TOP);
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(imconv->notebook), TRUE);
	gtk_notebook_popup_enable(GTK_NOTEBOOK(imconv->notebook));
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(imconv->notebook), TRUE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(imconv->notebook), TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), imconv->notebook, TRUE, TRUE, 0);
	g_signal_connect(imconv->notebook, "switch-page",
			G_CALLBACK(switch_page_cb), imconv);

	/* create action area, "Close" button and "Send" button */
	action_area = gtk_hbox_new(FALSE, 0);

	halign = gtk_alignment_new(1, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(halign), action_area);
	gtk_box_pack_start(GTK_BOX(vbox), halign, FALSE, FALSE, 0);

	button = gtk_button_new_with_label(_("Close"));
	gtk_widget_set_usize(button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 2);
	g_signal_connect(button, "clicked",	G_CALLBACK(conv_close_cb), imconv);

	button = gtk_button_new_with_label(_("Send"));
	gtk_widget_set_usize(button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 2);
	//g_signal_connect(send_button , "clicked" , G_CALLBACK(fx_chat_on_send_clicked) , fxchat);

	gtk_widget_show_all(imconv->window);

	return imconv;
}

static void
menu_switch_page_cb(GtkWidget *widget, gpointer user_data)
{
	IMChatPanel *chat = (IMChatPanel*)user_data;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(chat->parent->notebook),
			chat->page_index);
}

static void
menu_close_current_page_cb(GtkWidget *widget, gpointer user_data)
{
	IMChatPanel *chat = (IMChatPanel*)user_data;

}

static gboolean
tab_press_cb(GtkWidget *widget, GdkEventButton *e, gpointer user_data)
{
	if (e->button == 3) { /**< right button clicked */
		IMChatPanel *chat = (IMChatPanel*)user_data;
		IMChatPanel *temp_chat;
		IMBuddy *temp_buddy;
		IMBuddy *buddy = chat->buddy;
		GdkPixbuf *pixbuf;
		GtkWidget *img;
		GtkWidget *menu;
		GtkWidget *submenu;
		GSList *pos;

		menu = gtk_menu_new();

		/* create labels menu */
		for (pos = chat->parent->chat_buddies; pos; pos = pos->next) {

			temp_chat = (IMChatPanel*)pos->data;	
			temp_buddy = temp_chat->buddy;
			pixbuf = create_pixbuf_at_size(temp_buddy->icon_data,
					temp_buddy->icon_data_length, 16, 16);
			img = gtk_image_new_from_pixbuf(pixbuf);
			g_object_unref(pixbuf);

			submenu = gtk_image_menu_item_new_with_label(temp_buddy->name);
			g_signal_connect(submenu, "activate",
					G_CALLBACK(menu_switch_page_cb), temp_chat);
			gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(submenu), img);

			gtk_menu_shell_append(GTK_MENU_SHELL(menu), submenu);
		}
		/* create move menu */
		submenu = gtk_menu_item_new_with_label(_("Close Current Page"));
		g_signal_connect(submenu, "activate",
				G_CALLBACK(menu_close_current_page_cb), temp_chat);

		gtk_widget_show_all(menu);

		gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
				(e != NULL) ? e->button : 0,
				gdk_event_get_time((GdkEvent*)e));

		return TRUE;
	}

	return FALSE;
}


/**
 * Create the tab label widget for the GtkNotebook.
 * The layout is:
 *
 * -----------------------------------------------------
 * | Status  |                     |   Close Button    |  
 * |  Icon   | buddy name (markup) |                   |
 * | (16×16) |                     |      (16×16)      | 
 * -----------------------------------------------------
 * |- GtkEventBox -> GtkCellView  -|--- GtkEventBox ---|
 */
static GtkWidget*
create_note_label(IMChatPanel *chat)
{
	GtkWidget *hbox;
	GtkWidget *eventbox;
	GtkWidget *close_image;
	GtkWidget *label;
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreePath *path;
	IMBuddy *buddy;
	GdkPixbuf *icon_pixbuf;

	g_return_val_if_fail(chat != NULL, NULL);

	buddy = chat->buddy;

	hbox = gtk_hbox_new(FALSE, 0);

	label = gtk_cell_view_new();

	store = gtk_list_store_new(TAB_COLUMNS,
			GDK_TYPE_PIXBUF,
			G_TYPE_STRING);

	gtk_cell_view_set_model(GTK_CELL_VIEW(label), GTK_TREE_MODEL(store));

	g_object_unref(store);

	/* buddy icon renderer */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(label), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(label), renderer,
			"pixbuf", TAB_STATUS_ICON_COLUMN, NULL);
	g_object_set(renderer, "yalign", 0.5, "xpad", 3, "ypad", 0, NULL);

	/* buddy name renderer */
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(label), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(label), renderer,
			"markup", TAB_NAME_COLUMN, NULL);
	g_object_set(renderer, "xalign", 0.5, "xpad", 6, "ypad", 0, NULL);
	g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

	chat->tablabel = label;
	gtk_list_store_append(store, &chat->tabiter);
	path = gtk_tree_path_new_from_string("0");
	gtk_cell_view_set_displayed_row(GTK_CELL_VIEW(label), path);
	gtk_tree_path_free(path);

	icon_pixbuf = create_presence_pixbuf(buddy->state, 16);

	gtk_list_store_set(store, &chat->tabiter, 
			TAB_STATUS_ICON_COLUMN, icon_pixbuf,
			TAB_NAME_COLUMN, buddy->name, -1);

	g_object_unref(icon_pixbuf);

	eventbox = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(eventbox), label);
	gtk_box_pack_start(GTK_BOX(hbox), eventbox, TRUE, TRUE, 0);
	gtk_widget_add_events(eventbox,
			GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);
	g_signal_connect(G_OBJECT(eventbox), "button-press-event",
			G_CALLBACK(tab_press_cb), chat);

	/* close button */
	eventbox = gtk_event_box_new();
	close_image = gtk_image_new_from_file(DATA_DIR"/close.png");
	gtk_container_add(GTK_CONTAINER(eventbox), close_image);

	gtk_box_pack_start(GTK_BOX(hbox), eventbox, FALSE, FALSE, 0);

	gtk_widget_show_all(hbox);

	return hbox;
}

/**
 * Create the buddy tips panel. We implement it with GtkCellView.
 * The layout is:
 *
 * -----------------------------------------------------
 * |         | Name                  |  Proto | Status |  
 * |  Icon   |--------------(markup)-|  Icon  |  Icon  |
 * | (32×32) | Mood phrase           | (16×16)| (16×16)| 
 * -----------------------------------------------------
 */
static void
create_buddy_tips_panel(GtkWidget *vbox, IMChatPanel *chat)
{
	GtkWidget *cellview;
	GtkListStore *store;
	GtkCellRenderer *renderer; 
	GtkTreePath *path;
	IMBuddy *buddy;
	gchar *name_text;
	GdkPixbuf *icon_pixbuf;

	g_return_if_fail(vbox != NULL);

	buddy = chat->buddy;

	cellview = gtk_cell_view_new();
	
	store = gtk_list_store_new(LABEL_COLUMNS,
			GDK_TYPE_PIXBUF,
			G_TYPE_STRING,
			GDK_TYPE_PIXBUF,
			GDK_TYPE_PIXBUF);

	gtk_cell_view_set_model(GTK_CELL_VIEW(cellview), GTK_TREE_MODEL(store));

	/* buddy icon renderer */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cellview), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cellview), renderer,
			"pixbuf", BUDDY_ICON_COLUMN, NULL);
	g_object_set(renderer, "yalign", 0.5, "xpad", 3, "ypad", 0, NULL);

	/* buddy name renderer */
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cellview), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cellview), renderer,
			"markup", BUDDY_NAME_COLUMN, NULL);
	g_object_set(renderer, "xalign", 0.0, "xpad", 6, "ypad", 0, NULL);
	g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

	/* protocol icon renderer */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cellview), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cellview), renderer,
			"pixbuf", BUDDY_PROTO_ICON_COLUMN, NULL);
	g_object_set(renderer, "xalign", 0.0, "xpad", 6, "ypad", 0, NULL);

	/* status icon renderer */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cellview), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cellview), renderer,
			"pixbuf", BUDDY_STATUS_ICON_COLUMN, NULL);
	g_object_set(renderer, "xalign", 0.0, "xpad", 6, "ypad", 0, NULL);

	gtk_list_store_append(store, &chat->tipiter);
	path = gtk_tree_path_new_from_string("0");
	gtk_cell_view_set_displayed_row(GTK_CELL_VIEW(cellview), path);
	gtk_tree_path_free(path);

	icon_pixbuf = create_round_pixbuf(buddy->icon_data,
			buddy->icon_data_length, 32);

	name_text = g_strdup_printf(
			"<b>%s</b>\n<small><span font=\"#8f8f8f\">%s</span></small>",
			buddy->name, buddy->mood);

	gtk_list_store_set(store, &chat->tipiter, 
			BUDDY_ICON_COLUMN, icon_pixbuf,
			BUDDY_NAME_COLUMN, name_text, -1);

	g_object_unref(icon_pixbuf);
	g_free(name_text);

	gtk_box_pack_start(GTK_BOX(vbox), cellview, FALSE, FALSE, 5);
}

/**
 * Initialize the chat panel.
 */
static void
init_chat_panel(IMChatPanel *chat)
{
	GtkWidget *vbox;
	GtkWidget *scroll;
	GtkWidget *button;
	GtkWidget *image_icon;
	GtkWidget *tablabel;
	IMConversation *conv;
	IMBuddy *buddy;

	g_return_if_fail(chat != NULL);

	conv = chat->parent;
	buddy = chat->buddy;

	if (g_slist_length(conv->chat_buddies) == 1) {
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(conv->notebook), FALSE);
		gtk_notebook_set_show_border(GTK_NOTEBOOK(conv->notebook), TRUE);

	} else {
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(conv->notebook), TRUE);
		gtk_notebook_set_show_border(GTK_NOTEBOOK(conv->notebook), TRUE);
	}

	vbox = gtk_vbox_new(FALSE, 0);	

	chat->pagelabel = gtk_label_new(buddy->name);
	chat->page_index = gtk_notebook_append_page(GTK_NOTEBOOK(conv->notebook), vbox,
			chat->pagelabel);
	tablabel = create_note_label(chat);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(conv->notebook), vbox, tablabel);
	gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(conv->notebook), vbox, TRUE);
	gtk_notebook_set_tab_detachable(GTK_NOTEBOOK(conv->notebook), vbox, TRUE);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(conv->notebook),
			vbox, TRUE, TRUE, GTK_PACK_START);

	/* create buddy tips panel */
	create_buddy_tips_panel(vbox, chat);

	/* create textview */
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
			GTK_SHADOW_ETCHED_IN);

	chat->textview = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(chat->textview),
			GTK_WRAP_WORD_CHAR);
	gtk_container_add(GTK_CONTAINER(scroll), chat->textview);

	/* create toolbar */
	chat->toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(chat->toolbar), GTK_TOOLBAR_ICONS);
	gtk_box_pack_start(GTK_BOX(vbox), chat->toolbar, FALSE, FALSE, 0);
	
	image_icon = gtk_image_new_from_file(DATA_DIR"/history.png");
	button = gtk_toolbar_append_item(GTK_TOOLBAR(chat->toolbar),
			_("Chat logs"), _("View chat logs"), NULL, image_icon,
			NULL, NULL);
	gtk_toolbar_append_space(GTK_TOOLBAR(chat->toolbar));

	image_icon = gtk_image_new_from_file(DATA_DIR"/nudge.png");
	button = gtk_toolbar_append_item(GTK_TOOLBAR(chat->toolbar),
			_("Screen jitter"), _("Send a screen jitter"), NULL,
			image_icon, NULL, NULL);
	gtk_toolbar_append_space(GTK_TOOLBAR(chat->toolbar));
	gtk_widget_show_all(chat->toolbar);

	/* create textview */
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, FALSE, FALSE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
			GTK_SHADOW_ETCHED_IN);

	chat->sendtext = gtk_text_view_new();
	gtk_widget_set_usize(chat->sendtext, 0, 80);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(chat->sendtext),
			GTK_WRAP_WORD_CHAR);
	gtk_container_add(GTK_CONTAINER(scroll), chat->sendtext);
	gtk_widget_show_all(scroll);

	gtk_widget_show_all(vbox);

	/* The function should stay here. Because of the following reason:
	 *
	 * Note that due to historical reasons, GtkNotebook refuses to
	 * switch to a page unless the child widget is visible.
	 *                                ---- GtkNotebook
	 * */
	gtk_notebook_set_current_page(GTK_NOTEBOOK(conv->notebook), chat->page_index);
}

IMChatPanel*
im_chat_panel_create(IMBuddy *buddy)
{
	IMChatPanel *chat = NULL;
	IMConversation *conv = NULL;
	GSList *conv_pos;
	GSList *chat_pos;

	g_return_val_if_fail(buddy != NULL, NULL);

	for (conv_pos = conv_list; conv_pos; conv_pos = conv_pos->next) {
		conv = (IMConversation*)conv_pos->data;

		for (chat_pos = conv->chat_buddies; chat_pos;
				chat_pos = chat_pos->next) {

			chat = (IMChatPanel*)chat_pos->data;

			if (chat->buddy == buddy) {
				goto found;
			}
		}
	}

	if (!conv) {
		conv = im_conv_create();
		conv_list = g_slist_append(conv_list, conv);
	}

	chat = g_new0(IMChatPanel, 1);
	chat->parent = conv;
	chat->buddy = buddy;
	conv->chat_buddies = g_slist_append(conv->chat_buddies, chat);

found:
	init_chat_panel(chat);	

	return chat;
}

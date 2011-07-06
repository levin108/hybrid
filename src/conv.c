#include <gdk/gdkkeysyms.h>

#include "gtkutils.h"
#include "chat-textview.h"
#include "conv.h"
#include "util.h"

/* The list of the currently opened conversation dialogs. */
GSList *conv_list = NULL; 

static GtkWidget *create_note_label(HybridChatWindow *chat);
static void chat_window_destroy(HybridChatWindow *chat);
static gboolean key_press_func(GtkWidget *widget, GdkEventKey *event,
							HybridConversation *conv);

/**
 * Callback function to handle the close button click event.
 */
static void
conv_close_cb(GtkWidget *widget, gpointer user_data)
{
	HybridConversation *conv;

	conv = (HybridConversation*)user_data;

	gtk_widget_destroy(conv->window);
}

/**
 * Callback function to handle the conversation window destroy event.
 */
static void
conv_destroy_cb(GtkWidget *widget, gpointer user_data)
{
	HybridConversation *conv = (HybridConversation*)user_data;
	GSList *pos;
	HybridChatWindow *temp_chat;

	/* First we should free the memory in the list of HybridChatWindow. */
	while (conv->chat_buddies) {

		pos = conv->chat_buddies;

		temp_chat = (HybridChatWindow*)pos->data;
		conv->chat_buddies = g_slist_remove(conv->chat_buddies, temp_chat);

		chat_window_destroy(temp_chat);
	}

	conv_list = g_slist_remove(conv_list, conv);
	g_free(conv);
}

static void
chat_window_destroy(HybridChatWindow *chat)
{
	if (chat) {
		g_free(chat->id);
		g_free(chat->title);

		if (chat->icon) {
			g_object_unref(chat->icon);
		}

		g_free(chat);
	}
}

static void
switch_page_cb(GtkNotebook *notebook, gpointer newpage, guint newpage_nth,
		gpointer user_data)
{
	GSList *pos;
	HybridChatWindow *chat;
	HybridBuddy *buddy;
	GdkPixbuf *pixbuf;
	HybridConversation *conv = (HybridConversation*)user_data;	
	gint page_index;

	for (pos = conv->chat_buddies; pos; pos = pos->next) {
		chat = (HybridChatWindow*)pos->data;

		page_index = gtk_notebook_page_num(GTK_NOTEBOOK(conv->notebook),
				chat->vbox);

		if (page_index == newpage_nth) {
			goto page_found;
		}
	}

	hybrid_debug_error("conv", "FATAL, can not find an exist buddy\n");

	return;

page_found:
	
	if (IS_SYSTEM_CHAT(chat)) {

		buddy = chat->data;

		/* Set the conversation window's icon. */
		pixbuf = hybrid_create_pixbuf(buddy->icon_data, buddy->icon_data_length);
		gtk_window_set_icon(GTK_WINDOW(conv->window), pixbuf);
		g_object_unref(pixbuf);

		/* Set the conversation window's title */
		gtk_window_set_title(GTK_WINDOW(conv->window), 
			(!buddy->name || *(buddy->name) == '\0') ? buddy->id : buddy->name);
	}
}

static void
message_send(HybridConversation *conv)
{
	GtkTextBuffer *send_tb;
	GtkTextIter start_iter;
	GtkTextIter stop_iter;
	GtkTextView *textview;
	GSList *pos;
	HybridChatWindow *chat;
	gint current_page;
	gchar *text;

	HybridBuddy   *buddy;
	HybridAccount *account;
	HybridModule  *module;


	/* find the current chat panel. */
	current_page = gtk_notebook_current_page(GTK_NOTEBOOK(conv->notebook));
	for (pos = conv->chat_buddies; pos; pos = pos->next) {
		chat = (HybridChatWindow*)pos->data;

		if (current_page == gtk_notebook_page_num(
					GTK_NOTEBOOK(conv->notebook), chat->vbox)) {
			goto chat_found;
		}
	}

	hybrid_debug_error("conv", "FATAL, can't find chat panel");

	return;

chat_found:

	textview = GTK_TEXT_VIEW(chat->sendtext);
	send_tb  = gtk_text_view_get_buffer(textview);

	gtk_text_buffer_get_start_iter(send_tb, &start_iter);
	gtk_text_buffer_get_end_iter(send_tb, &stop_iter);

	text = gtk_text_buffer_get_text(send_tb, &start_iter, &stop_iter, TRUE);

	if (*text == '\0') { 
		/* Yes, nothing was input, just return. */
		return;
	}

	gtk_text_buffer_delete(send_tb, &start_iter, &stop_iter);

	account = chat->account;

	/* Add message to the textview. */
	hybrid_chat_textview_append(chat->textview,
								account->nickname,
								text, time(NULL), TRUE);

	/* Call the protocol hook function. */
	if (IS_SYSTEM_CHAT(chat)) {
		buddy   = chat->data;
		module  = account->proto;

		if (module->info->chat_send) {
			module->info->chat_send(account, buddy, text);
		}
	}

	if (IS_USER_DEFINED_CHAT(chat)) {

		if (chat->callback) {
			chat->callback(account, text);
		}
	}
}

static void
conv_send_cb(GtkWidget *widget, HybridConversation *conv)
{
	message_send(conv);
}

static gboolean
key_pressed_cb(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
{
	HybridConversation *conv;

	conv = (HybridConversation*)user_data;

	if (event->keyval == GDK_Return ||
		event->keyval == GDK_ISO_Enter ||
		event->keyval == GDK_KP_Enter) {

		if (event->state & GDK_CONTROL_MASK ||
		   event->state & GDK_SHIFT_MASK) {

			return FALSE;
		} else {

			/* find the current chat panel. */
			if (gtk_im_context_filter_keypress(
						GTK_TEXT_VIEW(widget)->im_context, event)) {

				GTK_TEXT_VIEW(widget)->need_im_reset = TRUE;

				return TRUE;
			}

			message_send(conv);

			return TRUE;
		}
#if 0
		if(event->state & GDK_CONTROL_MASK)	{
			return TRUE;
		}else{
			return FALSE;
		}
#endif
	}

	return FALSE;
}

/**
 * Create a new Hybrid Conversation Dialog.
 */
static HybridConversation*
hybrid_conv_create()
{
	GtkWidget *vbox;
	GtkWidget *action_area;
	GtkWidget *halign;
	GtkWidget *button;

	HybridConversation *imconv;

	imconv = g_new0(HybridConversation, 1);

	/* create window */
	imconv->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(imconv->window), 550, 500);
	gtk_widget_set_size_request(imconv->window, 550, 550);
	gtk_container_set_border_width(GTK_CONTAINER(imconv->window), 3);
	g_signal_connect(imconv->window, "destroy", G_CALLBACK(conv_destroy_cb),
			imconv);
	g_signal_connect(imconv->window, "key-press-event",
			G_CALLBACK(key_press_func), imconv);

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
	gtk_box_pack_start(GTK_BOX(vbox), halign, FALSE, FALSE, 5);

	button = gtk_button_new_with_label(_("Close"));
	gtk_widget_set_usize(button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 2);
	g_signal_connect(button, "clicked",	G_CALLBACK(conv_close_cb), imconv);

	button = gtk_button_new_with_label(_("Send"));
	gtk_widget_set_usize(button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 2);
	g_signal_connect(button, "clicked", G_CALLBACK(conv_send_cb), imconv);

	gtk_widget_show_all(imconv->window);

	return imconv;
}

static void
menu_switch_page_cb(GtkWidget *widget, HybridChatWindow *chat)
{
	HybridConversation *conv = chat->parent;
	GtkNotebook *notebook = GTK_NOTEBOOK(conv->notebook);
	gint page_index = gtk_notebook_page_num(notebook, chat->vbox);

	gtk_notebook_set_current_page(notebook, page_index);

	/* focus the send textview */
	gtk_widget_grab_focus(chat->sendtext);
}

/**
 * Close a single tab.
 */
static void 
close_tab(HybridChatWindow *chat)
{
	HybridConversation *conv;
	gint page_index;

	g_return_if_fail(chat != NULL);

	conv = chat->parent;

	page_index = gtk_notebook_page_num(GTK_NOTEBOOK(conv->notebook),
			chat->vbox);
	gtk_notebook_remove_page(GTK_NOTEBOOK(conv->notebook), page_index);

	conv->chat_buddies = g_slist_remove(conv->chat_buddies, chat);

	if (g_slist_length(conv->chat_buddies) == 1) {
		/*
		 * We don't want to show the tabs any more 
		 * when we have only one tab left.  	
		 */
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(conv->notebook), FALSE);
	}

	g_free(chat);

	if (conv->chat_buddies == NULL) { 

	   /*
		* Now we need to destroy the conversation window.
		* NOTE: We don't have to free the resource here,
		*       it will be done in the callback function
		*       of the window-destroy event.
		*/
		gtk_widget_destroy(conv->window);
	}

}

static void
menu_close_current_page_cb(GtkWidget *widget, HybridChatWindow *chat)
{
	close_tab(chat);
}

static void
menu_popup_current_page_cb(GtkWidget *widget, HybridChatWindow *chat)
{
	HybridChatWindow *newchat;
	GtkWidget *vbox;
	gint page_index;

	HybridConversation *newconv;
	HybridConversation *parent;
	HybridBuddy *buddy;

	vbox = chat->vbox;
	/* 
	 * First we increase the reference value of vbox,
	 * prevent it from being destroyed by gtk_notebook_remove_page().
	 */
	g_object_ref(vbox);

	/*
	 * When closing the chat panel, the chat object will be destroyed,
	 * so we must store the buddy first before closing the tab. 
	 */
	buddy = chat->data;

	close_tab(chat);

	parent = chat->parent;

	newconv = hybrid_conv_create();
	conv_list = g_slist_append(conv_list, newconv);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(newconv->notebook), FALSE);

	newchat = g_new0(HybridChatWindow, 1);
	newchat->parent = newconv;
	newchat->data = buddy;
	newchat->vbox = vbox;
	newconv->chat_buddies = g_slist_append(newconv->chat_buddies, newchat);

	newchat->pagelabel = create_note_label(newchat);
	page_index = gtk_notebook_append_page(GTK_NOTEBOOK(newconv->notebook),
					vbox, newchat->pagelabel);
	gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(newconv->notebook), vbox, TRUE);
	gtk_notebook_set_tab_detachable(GTK_NOTEBOOK(newconv->notebook), vbox, TRUE);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(newconv->notebook),
			vbox, TRUE, TRUE, GTK_PACK_START);
}

static void
menu_close_other_pages_cb(GtkWidget *widget, HybridChatWindow *chat)
{
	HybridConversation *conv;
	GSList *pos;

	conv = chat->parent;

	while (g_slist_length(conv->chat_buddies) > 1) {
		pos = conv->chat_buddies;

		if (pos->data != chat) {
			close_tab(pos->data);

		} else {
			pos = pos->next;

			if (pos) {
				close_tab(pos->data);
			}
		}
	}
}

static void
menu_close_all_pages_cb(GtkWidget *widget, HybridChatWindow *chat)
{
	gtk_widget_destroy(chat->parent->window);
}

static gboolean
tab_press_cb(GtkWidget *widget, GdkEventButton *e, HybridChatWindow *chat)
{
	if (e->button == 1) {

		gtk_widget_grab_focus(chat->sendtext);

		return FALSE;
	}

	if (e->button == 3) { /**< right button clicked */

		HybridChatWindow *temp_chat;
		HybridBuddy *temp_buddy;
		GdkPixbuf *pixbuf;
		GtkWidget *img;
		GtkWidget *menu;
		GtkWidget *submenu;
		GtkWidget *seperator;
		GSList *pos;

		menu = gtk_menu_new();

		/* create labels menu */
		for (pos = chat->parent->chat_buddies; pos; pos = pos->next) {

			temp_chat = (HybridChatWindow*)pos->data;	

			if (IS_SYSTEM_CHAT(temp_chat)) {

				temp_buddy = temp_chat->data;

				pixbuf = hybrid_create_pixbuf_at_size(temp_buddy->icon_data,
							temp_buddy->icon_data_length, 16, 16);

				img = gtk_image_new_from_pixbuf(pixbuf);

				g_object_unref(pixbuf);

				submenu = gtk_image_menu_item_new_with_label(
						temp_buddy->name && *(temp_buddy->name) != '\0' ?
						temp_buddy->name : temp_buddy->id);
				gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(submenu), img);

				g_signal_connect(submenu, "activate",
						G_CALLBACK(menu_switch_page_cb), temp_chat);


				gtk_menu_shell_append(GTK_MENU_SHELL(menu), submenu);
			}
		}

		/* create seperator */
		seperator = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(menu) , seperator);

		/* create move menu */
		submenu = gtk_menu_item_new_with_label(_("Close Current Page"));

		gtk_menu_shell_append(GTK_MENU_SHELL(menu), submenu);

		g_signal_connect(submenu, "activate",
				G_CALLBACK(menu_close_current_page_cb), chat);

		submenu = gtk_menu_item_new_with_label(_("Popup Current Page"));

		gtk_menu_shell_append(GTK_MENU_SHELL(menu), submenu);

		g_signal_connect(submenu, "activate",
				G_CALLBACK(menu_popup_current_page_cb), chat);

		submenu = gtk_menu_item_new_with_label(_("Close Other Pages"));

		gtk_menu_shell_append(GTK_MENU_SHELL(menu), submenu);

		g_signal_connect(submenu, "activate",
				G_CALLBACK(menu_close_other_pages_cb), chat);

		submenu = gtk_menu_item_new_with_label(_("Close All Pages"));

		gtk_menu_shell_append(GTK_MENU_SHELL(menu), submenu);

		g_signal_connect(submenu, "activate",
				G_CALLBACK(menu_close_all_pages_cb), chat);

		gtk_widget_show_all(menu);

		gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
				(e != NULL) ? e->button : 0,
				gdk_event_get_time((GdkEvent*)e));

		return TRUE;
	}

	return FALSE;
}

static gboolean
tab_close_press_cb(GtkWidget *widget, GdkEventButton *e, gpointer user_data)
{
	HybridChatWindow *chat = (HybridChatWindow*)user_data;

	if (e->button == 1) {
		close_tab(chat);
	}

	return TRUE;
}


/**
 * Callback function of the conversation window's key-press event.
 */
static gboolean 
key_press_func(GtkWidget *widget, GdkEventKey *event, HybridConversation *conv)
{
	gint current_page;
	gint pages;
	GSList *pos;
	HybridChatWindow *chat;

	if (event->state & GDK_CONTROL_MASK) {

		/* CTRL+W close the chat tab. */
		if (event->keyval == GDK_w) {

			/* find the current chat panel. */
			current_page = gtk_notebook_current_page(
					GTK_NOTEBOOK(conv->notebook));

			for (pos = conv->chat_buddies; pos; pos = pos->next) {
				chat = (HybridChatWindow*)pos->data;

				if (current_page == gtk_notebook_page_num(
							GTK_NOTEBOOK(conv->notebook), chat->vbox)) {

					close_tab(chat);

					return TRUE;
				}
			}

			hybrid_debug_error("conv", "FATAL, can't find chat panel");

			return FALSE;
		}

		/* CTRL+Q close the window. */
		if (event->keyval == GDK_q) {

			gtk_widget_destroy(conv->window);

			return TRUE;
		}

		/* CTRL+TAB move to next page. */
		if (event->keyval == GDK_Tab) {

			pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(conv->notebook));

			if (pages == 1) {

				return TRUE;

			} else {
				current_page = gtk_notebook_current_page(
						GTK_NOTEBOOK(conv->notebook));

				if (pages - 1 == current_page) {
					gtk_notebook_set_current_page(GTK_NOTEBOOK(conv->notebook), 0);

				} else {
					gtk_notebook_next_page(GTK_NOTEBOOK(conv->notebook));
				}
			}

			return TRUE;
		}

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
create_note_label(HybridChatWindow *chat)
{
	GtkWidget *hbox;
	GtkWidget *eventbox;
	GtkWidget *close_image;
	GtkWidget *label;
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreePath *path;
	HybridBuddy *buddy;
	GdkPixbuf *icon_pixbuf;

	g_return_val_if_fail(chat != NULL, NULL);


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

	if (IS_SYSTEM_CHAT(chat)) {

		buddy = chat->data;

		icon_pixbuf = hybrid_create_presence_pixbuf(buddy->state, 16);

		gtk_list_store_set(store, &chat->tabiter, 
				TAB_STATUS_ICON_COLUMN, icon_pixbuf, TAB_NAME_COLUMN,
				buddy->name && *(buddy->name) != '\0' ? buddy->name : buddy->id,
				-1);

		g_object_unref(icon_pixbuf);
	}

	eventbox = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(eventbox), label);
	gtk_box_pack_start(GTK_BOX(hbox), eventbox, TRUE, TRUE, 0);
	gtk_widget_add_events(eventbox,
			GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);
	g_signal_connect(G_OBJECT(eventbox), "button-press-event",
			G_CALLBACK(tab_press_cb), chat);

	/* close button */
	eventbox = gtk_event_box_new();
	close_image = gtk_image_new_from_file(PIXMAPS_DIR"menus/close.png");
	g_signal_connect(G_OBJECT(eventbox), "button-press-event",
			G_CALLBACK(tab_close_press_cb), chat);
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
create_buddy_tips_panel(GtkWidget *vbox, HybridChatWindow *chat)
{
	GtkWidget *cellview;
	GtkListStore *store;
	GtkCellRenderer *renderer; 
	GtkTreePath *path;
	HybridAccount *account;
	HybridModule *proto;
	HybridBuddy *buddy;
	gchar *name_text;
	gchar *mood_text;
	GdkPixbuf *icon_pixbuf;
	GdkPixbuf *proto_pixbuf;
	GdkPixbuf *presence_pixbuf;

	g_return_if_fail(vbox != NULL);


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

	chat->tiplabel = cellview;

	if (IS_SYSTEM_CHAT(chat)) {

		buddy = chat->data;

		icon_pixbuf = hybrid_create_round_pixbuf(buddy->icon_data,
						buddy->icon_data_length, 32);

		presence_pixbuf = hybrid_create_presence_pixbuf(buddy->state, 16);

		mood_text = g_markup_escape_text(buddy->mood ? buddy->mood : "", -1);

		name_text = g_strdup_printf(
				"<b>%s</b>\n<small><span font=\"#8f8f8f\">%s</span></small>",
				buddy->name && *(buddy->name) != '\0' ? buddy->name : buddy->id,
				mood_text);

		gtk_list_store_set(store, &chat->tipiter, 
				BUDDY_ICON_COLUMN, icon_pixbuf,
				BUDDY_NAME_COLUMN, name_text, 
				BUDDY_STATUS_ICON_COLUMN, presence_pixbuf, -1);

		g_object_unref(icon_pixbuf);
		g_object_unref(presence_pixbuf);

		g_free(name_text);
		g_free(mood_text);
	}

	account = chat->account;
	proto   = account->proto;

	proto_pixbuf = hybrid_create_proto_icon(proto->info->name, 16);

	gtk_list_store_set(store, &chat->tipiter,
					BUDDY_PROTO_ICON_COLUMN, proto_pixbuf, -1);

	g_object_unref(proto_pixbuf);

	gtk_box_pack_start(GTK_BOX(vbox), cellview, FALSE, FALSE, 5);
}

static void
init_chat_window_body(GtkWidget *vbox, HybridChatWindow *chat)
{
	GtkWidget *scroll;
	GtkWidget *button;
	GtkWidget *image_icon;

	g_return_if_fail(vbox != NULL);
	g_return_if_fail(chat != NULL);

	/* create buddy tips panel */
	create_buddy_tips_panel(vbox, chat);

	/* create textview */
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
			GTK_SHADOW_ETCHED_IN);

	chat->textview = hybrid_chat_textview_create();
	gtk_container_add(GTK_CONTAINER(scroll), chat->textview);

	/* create toolbar */
	chat->toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(chat->toolbar), GTK_TOOLBAR_ICONS);
	gtk_box_pack_start(GTK_BOX(vbox), chat->toolbar, FALSE, FALSE, 0);
	
	image_icon = gtk_image_new_from_file(PIXMAPS_DIR"menus/logs.png");
	button = gtk_toolbar_append_item(GTK_TOOLBAR(chat->toolbar),
			_("Chat logs"), _("View chat logs"), NULL, image_icon,
			NULL, NULL);

	if (IS_SYSTEM_CHAT(chat)) {
		gtk_toolbar_append_space(GTK_TOOLBAR(chat->toolbar));
		image_icon = gtk_image_new_from_file(PIXMAPS_DIR"menus/nudge.png");
		button = gtk_toolbar_append_item(GTK_TOOLBAR(chat->toolbar),
				_("Screen jitter"), _("Send a screen jitter"), NULL,
				image_icon, NULL, NULL);
		gtk_toolbar_append_space(GTK_TOOLBAR(chat->toolbar));
	}

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
	g_signal_connect(chat->sendtext, "key_press_event",
			G_CALLBACK(key_pressed_cb), chat->parent);

	gtk_window_present(GTK_WINDOW(chat->parent->window));

	/* focus the send textview */
	GTK_WIDGET_SET_FLAGS(chat->sendtext, GTK_CAN_FOCUS);
	gtk_widget_grab_focus(chat->sendtext);

	gtk_widget_show_all(scroll);
	gtk_widget_show_all(vbox);
}

/**
 * Initialize the chat panel.
 */
static void
init_chat_window(HybridChatWindow *chat)
{
	GtkWidget *vbox;
	HybridConversation *conv;
	gint page_index;

	g_return_if_fail(chat != NULL);

	conv = chat->parent;

	if (g_slist_length(conv->chat_buddies) == 1) {
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(conv->notebook), FALSE);

	} else {
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(conv->notebook), TRUE);
	}

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	chat->vbox = vbox;

	chat->pagelabel = create_note_label(chat);
	page_index = gtk_notebook_append_page(GTK_NOTEBOOK(conv->notebook), vbox,
			chat->pagelabel);
	gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(conv->notebook), vbox, TRUE);
	gtk_notebook_set_tab_detachable(GTK_NOTEBOOK(conv->notebook), vbox, TRUE);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(conv->notebook),
			vbox, TRUE, TRUE, GTK_PACK_START);

	init_chat_window_body(vbox, chat);

	/*
	 * The function should stay here. Because of the following reason:
	 *
	 * Note that due to historical reasons, GtkNotebook refuses to
	 * switch to a page unless the child widget is visible.
	 *                                ---- GtkNotebook
	 */
	gtk_notebook_set_current_page(GTK_NOTEBOOK(conv->notebook), page_index);

	/* focus the send textview */
	gtk_widget_grab_focus(chat->sendtext);
}

HybridChatWindow*
hybrid_chat_window_create(HybridAccount *account, const gchar *id,
		HybridChatWindowType type)
{
	HybridChatWindow *chat = NULL;
	HybridConversation *conv = NULL;
	HybridBuddy *buddy;
	HybridModule *proto;
	GSList *conv_pos;
	GSList *chat_pos;

	g_return_val_if_fail(account != NULL, NULL);
	g_return_val_if_fail(id != NULL, NULL);

	if (type == HYBRID_CHAT_PANEL_SYSTEM) {
		if (!(buddy = (hybrid_blist_find_buddy(account, id)))) {
			
			hybrid_debug_error("conv", "FATAL, can't find buddy");

			return NULL;
		}

		proto = account->proto;

		/* we will check whether the protocol allows this buddy to be activated. */
		if (proto->info->chat_start) {
			
			if (!proto->info->chat_start(account, buddy)) {
				return NULL;
			}
		}
	}

	for (conv_pos = conv_list; conv_pos; conv_pos = conv_pos->next) {
		conv = (HybridConversation*)conv_pos->data;

		for (chat_pos = conv->chat_buddies; chat_pos;
				chat_pos = chat_pos->next) {

			chat = (HybridChatWindow*)chat_pos->data;

			if (g_strcmp0(id, chat->id) == 0) {
				goto found;
			}
		}
	}

	if (!conv) {
		conv = hybrid_conv_create();
		conv_list = g_slist_append(conv_list, conv);
	}

	chat = g_new0(HybridChatWindow, 1);
	chat->id      = g_strdup(id);
	chat->parent  = conv;
	chat->account = account;
	chat->type    = type;

	if (type == HYBRID_CHAT_PANEL_SYSTEM) {
		chat->data = buddy;
	}

	conv->chat_buddies = g_slist_append(conv->chat_buddies, chat);

	init_chat_window(chat);	
	return chat;

found:
	gtk_notebook_set_current_page(GTK_NOTEBOOK(conv->notebook),
		gtk_notebook_page_num(GTK_NOTEBOOK(conv->notebook), chat->vbox));

	/* focus the send textview */
	gtk_widget_grab_focus(chat->sendtext);

	return chat;
}

void
hybrid_conv_got_message(HybridAccount *account,
				const gchar *buddy_id, const gchar *message,
				time_t time)
{
	GSList *conv_pos;
	GSList *chat_pos;
	HybridConversation *conv;
	HybridChatWindow *chat;
	HybridBuddy *temp_buddy;
	HybridBuddy *buddy;
	gchar *msg;

	msg = hybrid_strip_html(message);

	g_return_if_fail(account != NULL);
	g_return_if_fail(buddy_id != NULL);
	g_return_if_fail(message != NULL);

	if (!(buddy = hybrid_blist_find_buddy(account, buddy_id))) {

		hybrid_debug_error("conv", "buddy doesn't exist.");
		g_free(msg);

		return;
	}

	for (conv_pos = conv_list; conv_pos; conv_pos = conv_pos->next) {
		conv = (HybridConversation*)conv_pos->data;

		for (chat_pos = conv->chat_buddies; chat_pos; 
				chat_pos = chat_pos->next) {
			chat = (HybridChatWindow*)chat_pos->data;

			temp_buddy = chat->data;

			if (g_strcmp0(temp_buddy->id, buddy_id) == 0) {
				goto got_chat_found;
			}
		}
	}
	
	/* Well, we haven't find an existing chat panel so far, so create one. */
	chat = hybrid_chat_window_create(account, buddy->id, HYBRID_CHAT_PANEL_SYSTEM);

got_chat_found:
	hybrid_chat_textview_append(chat->textview, buddy->name, msg, time, FALSE);

	g_free(msg);
}

void
hybrid_chat_window_set_title(HybridChatWindow *window, const gchar *title)
{
	GtkTreeModel *model;
	GtkListStore *store;

	g_return_if_fail(window != NULL);

	if (!IS_USER_DEFINED_CHAT(window)) {
		return;
	}

	window->title = g_strdup(title);

	model = gtk_cell_view_get_model(GTK_CELL_VIEW(window->tablabel));
	store = GTK_LIST_STORE(model);

	gtk_list_store_set(store, &window->tabiter, TAB_NAME_COLUMN, title, -1);

	model = gtk_cell_view_get_model(GTK_CELL_VIEW(window->tiplabel));
	store = GTK_LIST_STORE(model);

	gtk_list_store_set(store, &window->tipiter, BUDDY_NAME_COLUMN, title, -1);
}

void
hybrid_chat_window_set_icon(HybridChatWindow *window, GdkPixbuf *pixbuf)
{
	GtkTreeModel *model;
	GtkListStore *store;

	g_return_if_fail(window != NULL);

	if (!IS_USER_DEFINED_CHAT(window)) {
		return;
	}

	g_object_ref(pixbuf);
	window->icon = pixbuf;

	model = gtk_cell_view_get_model(GTK_CELL_VIEW(window->tablabel));
	store = GTK_LIST_STORE(model);

	gtk_list_store_set(store, &window->tabiter,
			TAB_STATUS_ICON_COLUMN, pixbuf,
			-1);

	model = gtk_cell_view_get_model(GTK_CELL_VIEW(window->tiplabel));
	store = GTK_LIST_STORE(model);

	gtk_list_store_set(store, &window->tipiter,
			BUDDY_ICON_COLUMN, pixbuf, -1);
}

void
hybrid_chat_window_set_callback(HybridChatWindow *window,
					ChatCallback callback)
{
	g_return_if_fail(window != NULL);

	window->callback = callback;
}

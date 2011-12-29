/***************************************************************************
 *   Copyright (C) 2011 by levin                                           *
 *   levin108@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 ***************************************************************************/
#include "about.h"
#include "gtkutils.h"

#define BUFLEN 4096

static void
follow_if_link (GtkWidget   *text_view,
                GtkTextIter *iter)
{
    GSList *tags = NULL, *tagp = NULL;

    tags = gtk_text_iter_get_tags (iter);
    for (tagp = tags;  tagp != NULL;  tagp = tagp->next)
        {
            GtkTextTag *tag = tagp->data;
            gchar *url = (gchar*)(g_object_get_data (G_OBJECT (tag), "url"));

            if (url)
                {
                    if(fork() == 0)
                        execlp("xdg-open" , "xdg-open" , url , (char**)NULL);
                    break;
                }
        }

    if (tags)
        g_slist_free (tags);
}

static gboolean
event_after (GtkWidget *text_view,
             GdkEvent  *ev)
{
    GtkTextIter     start, end, iter;
    GtkTextBuffer  *buffer;
    GdkEventButton *event;
    gint            x, y;

    if (ev->type != GDK_BUTTON_RELEASE)
        return FALSE;

    event = (GdkEventButton *)ev;

    if (event->button != 1)
        return FALSE;

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

    gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
    if (gtk_text_iter_get_offset (&start) != gtk_text_iter_get_offset (&end))
        return FALSE;

    gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view),
                                           GTK_TEXT_WINDOW_WIDGET,
                                           event->x, event->y, &x, &y);

    gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (text_view), &iter, x, y);

    follow_if_link (text_view, &iter);

    return FALSE;
}
static void
insert_link (GtkTextBuffer *buffer,
             GtkTextIter   *iter,
             gchar         *text,
             gchar         *url)
{
    GtkTextTag *tag;

    tag = gtk_text_buffer_create_tag (buffer, NULL,
                                      "foreground", "blue",
                                      "underline", PANGO_UNDERLINE_SINGLE,
                                      NULL);
    g_object_set_data (G_OBJECT (tag), "url", url ? url : text);
    gtk_text_buffer_insert_with_tags (buffer, iter, text, -1, tag, NULL);
}

static gboolean   hovering_over_link = FALSE;
static GdkCursor *hand_cursor        = NULL;
static GdkCursor *regular_cursor     = NULL;

static void
set_cursor_if_appropriate (GtkTextView *text_view,
                           gint         x,
                           gint         y)
{
    GSList      *tags     = NULL;
    GSList      *tagp     = NULL;
    GtkTextIter  iter;
    gboolean     hovering = FALSE;

    gtk_text_view_get_iter_at_location (text_view, &iter, x, y);
    tags = gtk_text_iter_get_tags (&iter);

    for (tagp = tags;  tagp != NULL;  tagp = tagp->next) {
        GtkTextTag *tag = tagp->data;
        gchar      *url = (gchar*)(g_object_get_data (G_OBJECT (tag), "url"));

        if (url) {
            hovering = TRUE;
            break;
        }
    }

    if (hovering != hovering_over_link) {
        hovering_over_link = hovering;

        if (hovering_over_link)
            gdk_window_set_cursor (gtk_text_view_get_window (text_view, GTK_TEXT_WINDOW_TEXT), hand_cursor);
        else
            gdk_window_set_cursor (gtk_text_view_get_window (text_view, GTK_TEXT_WINDOW_TEXT), regular_cursor);
    }

    if (tags)
        g_slist_free (tags);
}

static gboolean
motion_notify_event (GtkWidget      *text_view,
                     GdkEventMotion *event)
{
    gint x, y;

    gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view),
                                           GTK_TEXT_WINDOW_WIDGET,
                                           event->x, event->y, &x, &y);

    set_cursor_if_appropriate (GTK_TEXT_VIEW (text_view), x, y);

    gdk_window_get_pointer (text_view->window, NULL, NULL, NULL);
    return FALSE;
}

static void create_intro(GtkTextView *view)
{
    GtkTextTag  *tag1;
    GtkTextIter  iter;
    const char intro[] = N_("Hybrid is a lightweight extensible IM framework,"
                            "currently supports China Mobile Fetion protocol,"
                            "XMPP protocol, and IMAP mail notifier.\n\n\n");

    gtk_text_view_set_editable(view,FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD_CHAR);
    g_signal_connect (view, "motion-notify-event",
                      G_CALLBACK (motion_notify_event), NULL);
    g_signal_connect(view, "event-after",
                     G_CALLBACK (event_after), NULL);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);

    tag1 = gtk_text_buffer_create_tag (buffer, NULL,
                                       "left-margin", 25,
                                       "right-margin", 25,
                                       NULL);


    gtk_text_buffer_get_start_iter(buffer, &iter);

    gtk_text_buffer_insert(buffer, &iter, "\n\n", 2);

    gtk_text_buffer_insert_with_tags(buffer, &iter,
                                     _(intro), -1, tag1, NULL);

    gtk_text_buffer_insert_with_tags(buffer, &iter,
                                     _("\nWebsite: "), -1, tag1, NULL);
    insert_link(buffer, &iter,
                "https://github.com/levin108/hybrid", NULL);

    gtk_text_buffer_insert_with_tags(buffer, &iter,
                                     _("\nBug report: "), -1, tag1, NULL);
    insert_link(buffer, &iter, "https://github.com/levin108/hybrid/issues", NULL);
}

static void create_gpl(GtkTextView *view)
{
    GtkTextTag    *tag;
    GtkTextIter    iter;
    char           *buf1 = N_("This program is free software; you can redistribute it and/or modify "
                              "it under the terms of the GNU General Public License as published by "
                              "the Free Software Foundation; either version 2 of the License, or "
                              "(at your option) any later version.\n\n");
    char           *buf2 = N_("This program is distributed in the hope that it will be useful, "
                              "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                              "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
                              "GNU General Public License for more details.\n\n");
    char           *buf3 = N_("You should have received a copy of the GNU General Public License "
                              "along with this program; if not, see:");

    gtk_text_view_set_editable(view, FALSE);
    gtk_text_view_set_wrap_mode(view, GTK_WRAP_WORD);
    g_signal_connect (view, "motion-notify-event",
                      G_CALLBACK (motion_notify_event), NULL);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);

    tag = gtk_text_buffer_create_tag (buffer, NULL,
                                      "left-margin", 5,
                                      "right-margin", 5,
                                      NULL);

    gtk_text_buffer_get_start_iter(buffer, &iter);

    gtk_text_buffer_insert(buffer, &iter, "\n\n", 2);
    gtk_text_buffer_insert_with_tags(buffer, &iter, _(buf1), -1, tag, NULL);
    gtk_text_buffer_insert_with_tags(buffer, &iter, _(buf2), -1, tag, NULL);
    gtk_text_buffer_insert_with_tags(buffer, &iter, _(buf3), -1, tag, NULL);
    insert_link(buffer, &iter, "http://www.gnu.org/licenses/gpl-2.0.txt", NULL);
}

static void create_contri(GtkTextView *view)
{
    GtkTextTag    *tag1;
    GtkTextTag    *tag2;
    GtkTextIter    iter;

    gtk_text_view_set_editable(view,FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD_CHAR);
    g_signal_connect (view, "motion-notify-event",
                      G_CALLBACK (motion_notify_event), NULL);
    g_signal_connect(view, "event-after",
                     G_CALLBACK (event_after), NULL);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);

    tag1 = gtk_text_buffer_create_tag (buffer, NULL,
                                       "left-margin", 25,
                                       "right-margin", 25,
                                       NULL);

    tag2 = gtk_text_buffer_create_tag (buffer, NULL,
                                       "left-margin", 25,
                                       "weight", PANGO_WEIGHT_BOLD,
                                       NULL);

    gtk_text_buffer_get_start_iter(buffer, &iter);

    gtk_text_buffer_insert(buffer, &iter, "\n\n", 2);

    gtk_text_buffer_insert_with_tags(buffer, &iter,
                                     "Levin Li <", -1, tag1, NULL);
    insert_link(buffer, &iter, "levin108@gmail.com", "mailto:levin108@gmail.com");
    gtk_text_buffer_insert(buffer, &iter, ("> ("), 3);
    insert_link(buffer, &iter, "@levin108", "http://twitter.com/levin108");
    gtk_text_buffer_insert(buffer, &iter, (")\n"), 2);

    gtk_text_buffer_insert_with_tags(buffer, &iter,
                                     "Yichao Yu <", -1, tag1, NULL);
    insert_link(buffer, &iter, "yyc1992@gmail.com", "mailto:yyc1992@gmail.com");
    gtk_text_buffer_insert(buffer, &iter, ("> ("), 3);
    insert_link(buffer, &iter, "@yuyichao", "http://twitter.com/yuyichao");
    gtk_text_buffer_insert(buffer, &iter, (")\n"), 2);

}

void
close_about(GtkWidget *widget, GtkWidget *window)
{
    gtk_widget_destroy(window);
}

void
hybrid_about_create()
{
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *notebook;
    GtkWidget *scroll;
    GtkWidget *textview;
    GtkWidget *tablabel;
    GtkWidget *action_area;
    GtkWidget *button;

    hand_cursor = gdk_cursor_new (GDK_HAND2);
    regular_cursor = gdk_cursor_new (GDK_XTERM);

    window = hybrid_create_window(_("About Hybrid"), NULL,
                                  GTK_WIN_POS_CENTER, FALSE);
    gtk_widget_set_size_request(window, 460, 300);

    vbox = gtk_vbox_new(FALSE, 5);

    notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_LEFT);
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 5);

    tablabel = gtk_label_new(_("Introduction"));
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER,
                                   GTK_POLICY_AUTOMATIC);
    textview = gtk_text_view_new();
    create_intro(GTK_TEXT_VIEW(textview));
    gtk_container_add(GTK_CONTAINER(scroll), textview);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             scroll, tablabel);

    tablabel = gtk_label_new(_("Developers"));
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER,
                                   GTK_POLICY_AUTOMATIC);
    textview = gtk_text_view_new();
    create_contri(GTK_TEXT_VIEW(textview));
    gtk_container_add(GTK_CONTAINER(scroll), textview);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             scroll, tablabel);

    tablabel = gtk_label_new(_("License"));
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    textview = gtk_text_view_new();
    create_gpl(GTK_TEXT_VIEW(textview));
    gtk_container_add(GTK_CONTAINER(scroll), textview);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             scroll, tablabel);

    action_area = gtk_hbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), action_area, FALSE, FALSE, 5);
    button = gtk_button_new_with_label(_("Close"));
    gtk_widget_set_usize(button, 90, 0);
    gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
    g_signal_connect(button, "clicked", G_CALLBACK(close_about), window);

    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
}

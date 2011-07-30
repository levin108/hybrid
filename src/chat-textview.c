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

#include "conv.h"
#include "chat-textview.h"

GtkWidget*
hybrid_chat_textview_create()
{
	GtkWidget *textview;
	GtkTextBuffer *buffer;
	GtkTextIter end_iter;

	textview = gtk_text_view_new();

	gtk_widget_set_size_request(textview, 0, 100);

	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textview), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_CHAR);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_create_tag(buffer, "blue", "foreground", "#639900", NULL);
	gtk_text_buffer_create_tag(buffer, "grey", "foreground", "#808080", NULL);
	gtk_text_buffer_create_tag(buffer, "green", "foreground", "#0088bf", NULL);
	gtk_text_buffer_create_tag(buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(buffer, "lm10", "left_margin", 3, NULL);
	gtk_text_buffer_create_tag(buffer, "wrap", "wrap-mode", GTK_WRAP_WORD_CHAR, NULL);
	gtk_text_buffer_create_tag(buffer, "small", "size-points", 9.0, NULL);
	gtk_text_buffer_get_end_iter(buffer, &end_iter);
	gtk_text_buffer_create_mark(buffer, "scroll", &end_iter, FALSE);

	return textview;
}

void
hybrid_chat_textview_append(GtkWidget *textview, const gchar *name,
							const gchar *message, time_t msg_time,
							gboolean sendout)
{
	GtkTextBuffer *recv_tb;
	GtkTextIter end_iter;
	GtkTextIter stop_iter;
	GtkTextIter start_iter;
	GtkTextMark *mark;
	gchar *names;
	const gchar *color;
	struct tm *tm_time;
	gchar time[128];

	g_return_if_fail(textview != NULL);
	g_return_if_fail(message != NULL);

	tm_time = localtime(&msg_time);
	strftime(time, sizeof(time) - 1, _("%H:%M:%S"), tm_time);

	recv_tb  = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_get_end_iter(recv_tb, &end_iter);

	gtk_text_view_backward_display_line(GTK_TEXT_VIEW(textview), &end_iter);

	gtk_text_buffer_get_end_iter(recv_tb, &stop_iter);
	gtk_text_buffer_get_start_iter(recv_tb, &start_iter);

	/* first line */
	if (gtk_text_iter_equal(&end_iter, &stop_iter)) {
		gtk_text_buffer_delete(recv_tb, &start_iter, &stop_iter);

	} else {
		gtk_text_buffer_delete(recv_tb, &end_iter, &stop_iter);

		if (!gtk_text_iter_equal(&start_iter, &end_iter)) {
			gtk_text_buffer_insert(recv_tb, &end_iter, "\n", -1);
		}
	}

	gtk_text_buffer_get_end_iter(recv_tb, &end_iter);

	if (sendout) {
		color = "blue";

	} else {
		color = "green";
	}

	names = g_strdup_printf(" (%s) ", time);

	gtk_text_buffer_insert_with_tags_by_name(recv_tb, &end_iter, 
					names, strlen(names), color, "wrap", NULL);
	g_free(names);

	gtk_text_buffer_insert_with_tags_by_name(recv_tb, &end_iter, 
					name, strlen(name), color, "bold", "wrap", NULL);

	names = _(" said:");

	gtk_text_buffer_insert_with_tags_by_name(recv_tb, &end_iter, 
					names, strlen(names), color, "wrap", NULL);

	gtk_text_buffer_insert(recv_tb, &end_iter, "\n", -1);
	
	gtk_text_buffer_insert_with_tags_by_name(recv_tb, &end_iter, 
					message, strlen(message), "lm10", "wrap", NULL);

	gtk_text_buffer_insert(recv_tb, &end_iter, "\n", -1);
	gtk_text_iter_set_line_offset(&end_iter, 0);

	mark = gtk_text_buffer_get_mark(recv_tb, "scroll");
	gtk_text_buffer_move_mark(recv_tb, mark, &end_iter);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(textview), mark);
}

void
hybrid_chat_textview_notify(GtkWidget *textview, const gchar *text, gint type)
{
	GtkTextBuffer *recv_tb;
	GtkTextIter end_iter;
	GtkTextIter stop_iter;
	GtkTextIter start_iter;
	GtkTextMark *mark;

	recv_tb  = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_get_end_iter(recv_tb, &end_iter);

	gtk_text_view_backward_display_line(GTK_TEXT_VIEW(textview), &end_iter);

	gtk_text_buffer_get_end_iter(recv_tb, &stop_iter);
	gtk_text_buffer_get_start_iter(recv_tb, &start_iter);

	/* first line */
	if (gtk_text_iter_equal(&end_iter, &stop_iter)) {
		gtk_text_buffer_delete(recv_tb, &start_iter, &stop_iter);

	} else {
		gtk_text_buffer_delete(recv_tb, &end_iter, &stop_iter);

		if (!gtk_text_iter_equal(&start_iter, &end_iter)) {
			gtk_text_buffer_insert(recv_tb, &end_iter, "\n", -1);
		}
	}

	gtk_text_buffer_get_end_iter(recv_tb, &end_iter);

	if (type == MSG_NOTIFICATION_INPUT) {
		gtk_text_buffer_insert_with_tags_by_name(recv_tb, &end_iter, 
						text, strlen(text), "grey", "small", "wrap", NULL);
	}

	mark = gtk_text_buffer_get_mark(recv_tb, "scroll");
	gtk_text_buffer_move_mark(recv_tb, mark, &end_iter);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(textview), mark);
}

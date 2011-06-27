#include "chat-textview.h"

GtkWidget*
hybrid_chat_textview_create()
{
	GtkWidget *textview;
	GtkTextBuffer *buffer;
	GtkTextIter end_iter;

	textview = gtk_text_view_new();

	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textview), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_CHAR);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_create_tag(buffer, "blue", "foreground", "#639900", NULL);
	gtk_text_buffer_create_tag(buffer, "grey", "foreground", "#808080", NULL);
	gtk_text_buffer_create_tag(buffer, "green", "foreground", "#0088bf", NULL);
	gtk_text_buffer_create_tag(buffer, "lm10", "left_margin", 10, NULL);
	gtk_text_buffer_create_tag(buffer, "small", "left_margin", 5, NULL);
	gtk_text_buffer_get_end_iter(buffer, &end_iter);
	gtk_text_buffer_create_mark(buffer, "scroll", &end_iter, FALSE);

	return textview;
}

void
hybrid_chat_textview_append(GtkWidget *textview, HybridBuddy *buddy,
							const gchar *message, gboolean sendout)
{
	GtkTextBuffer *recv_tb;
	GtkTextIter end_iter;
	GtkTextMark *mark;
	HybridAccount *account;
	gchar *names;

	g_return_if_fail(textview != NULL);
	g_return_if_fail(buddy != NULL);
	g_return_if_fail(message != NULL);

	account = buddy->account;

	recv_tb  = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_get_end_iter(recv_tb, &end_iter);

	if (sendout) {
		names = g_strdup_printf("%s said:", account->nickname);
		gtk_text_buffer_insert_with_tags_by_name(recv_tb, &end_iter, 
							names, strlen(names), "blue", NULL);

	} else {
		names = g_strdup_printf("%s said:", buddy->name);
		gtk_text_buffer_insert_with_tags_by_name(recv_tb, &end_iter, 
							names, strlen(names), "green", NULL);
	}

	g_free(names);

	gtk_text_buffer_insert(recv_tb, &end_iter, "\n", -1);
	
	gtk_text_buffer_insert_with_tags_by_name(recv_tb, &end_iter, 
						message, strlen(message), "lm10", NULL);

	gtk_text_buffer_insert(recv_tb, &end_iter, "\n", -1);
	gtk_text_iter_set_line_offset(&end_iter, 0);
	mark = gtk_text_buffer_get_mark(recv_tb, "scroll");
	gtk_text_buffer_move_mark(recv_tb, mark, &end_iter);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(textview), mark);
}

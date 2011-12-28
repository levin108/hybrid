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

#include "pref.h"
#include "util.h"

#include "conv.h"
#include "preference.h"
#include "gtkutils.h"

static HybridPrefWin *main_pref_window = NULL;

guint bool_pref_add_entry(GtkWidget *section, guint pos,
                          HybridPrefEntry *entry);
void bool_pref_save(HybridPrefEntry *entry);
void bool_pref_destroy(HybridPrefEntry *entry);

static PrefAddFuncs bool_add_funcs = {
    .add_entry = bool_pref_add_entry,
    .save = bool_pref_save,
    .destroy = bool_pref_destroy
};

guint string_pref_add_entry(GtkWidget *section, guint pos,
                            HybridPrefEntry *entry);
void string_pref_save(HybridPrefEntry *entry);
void string_pref_destroy(HybridPrefEntry *entry);

static PrefAddFuncs string_add_funcs = {
    .add_entry = string_pref_add_entry,
    .save = string_pref_save,
    .destroy = string_pref_destroy
};

guint int_pref_add_entry(GtkWidget *section, guint pos, HybridPrefEntry *entry);
void int_pref_save(HybridPrefEntry *entry);
void int_pref_destroy(HybridPrefEntry *entry);

static PrefAddFuncs int_add_funcs = {
    .add_entry = int_pref_add_entry,
    .save = int_pref_save,
    .destroy = int_pref_destroy
};

guint select_pref_add_entry(GtkWidget *section, guint pos,
                            HybridPrefEntry *entry);
void select_pref_save(HybridPrefEntry *entry);
void select_pref_destroy(HybridPrefEntry *entry);

static PrefAddFuncs select_add_funcs = {
    .add_entry = select_pref_add_entry,
    .save = select_pref_save,
    .destroy = select_pref_destroy
};

static PrefAddFuncs *pref_types[] = {
    [PREF_KEY_NONE] = NULL,
    [PREF_KEY_BOOL] = &bool_add_funcs,
    [PREF_KEY_STRING] = &string_add_funcs,
    [PREF_KEY_INT] = &int_add_funcs,
    [PREF_KEY_SELECT ... PREF_KEY_ENUM] = &select_add_funcs
};

guint
bool_pref_add_entry(GtkWidget *section, guint pos, HybridPrefEntry *entry)
{
    GtkWidget *checkbutton;

    checkbutton = gtk_check_button_new_with_label(entry->name);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),
                                 hybrid_pref_get_boolean(entry->win->pref,
                                                         entry->key));
    if (entry->tooltip)
        gtk_widget_set_tooltip_markup(checkbutton, entry->tooltip);
    entry->data = checkbutton;

    gtk_table_attach_defaults(GTK_TABLE(section), checkbutton, 0, 2,
                             pos, pos + 1);

    return 1;
}

void
bool_pref_save(HybridPrefEntry *entry)
{
    hybrid_pref_set_boolean(entry->win->pref, entry->key,
                            gtk_toggle_button_get_active(
                                GTK_TOGGLE_BUTTON(entry->data)));
}

void
bool_pref_destroy(HybridPrefEntry *entry)
{
    return;
}

guint
string_pref_add_entry(GtkWidget *section, guint pos, HybridPrefEntry *entry)
{
    GtkWidget *text;
    GtkWidget *label;
    gchar *value;

    label = gtk_label_new(entry->name);
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    if (entry->tooltip)
        gtk_widget_set_tooltip_markup(label, entry->tooltip);
    text = gtk_entry_new();

    value = hybrid_pref_get_string(entry->win->pref, entry->key);
    if (value) {
        gtk_entry_set_text(GTK_ENTRY(text), value);
        g_free(value);
    }

    entry->data = text;

    gtk_table_attach_defaults(GTK_TABLE(section), label, 0, 1, pos, pos + 1);
    gtk_table_attach_defaults(GTK_TABLE(section), text, 1, 2, pos, pos + 1);
    return 1;
}

void
string_pref_save(HybridPrefEntry *entry)
{
    hybrid_pref_set_string(entry->win->pref, entry->key,
                           gtk_entry_get_text(GTK_ENTRY(entry->data)));
}

void
string_pref_destroy(HybridPrefEntry *entry)
{
    return;
}

guint
int_pref_add_entry(GtkWidget *section, guint pos, HybridPrefEntry *entry)
{
    GtkWidget *number;
    GtkWidget *label;
    gint value;
    IntRange *range;
    gint upper;
    gint lower;
    gint step;

    label = gtk_label_new(entry->name);
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    if (entry->tooltip)
        gtk_widget_set_tooltip_markup(label, entry->tooltip);

    if ((range = entry->data)) {
        upper = range->upper;
        lower = range->lower;
        step = range->step;
    } else {
        /* The largest range I can think of now is the number of port. */
        upper = 65535;
        lower = 0;
        step = 1;
    }
    number = gtk_spin_button_new_with_range(lower, upper, step);

    value = hybrid_pref_get_int(entry->win->pref, entry->key);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(number), value);

    entry->data = number;

    gtk_table_attach_defaults(GTK_TABLE(section), label, 0, 1, pos, pos + 1);
    gtk_table_attach_defaults(GTK_TABLE(section), number, 1, 2, pos, pos + 1);
    return 1;
}

void
int_pref_save(HybridPrefEntry *entry)
{
    hybrid_pref_set_int(entry->win->pref, entry->key,
                        gtk_spin_button_get_value_as_int(
                            GTK_SPIN_BUTTON(entry->data)));
}

void
int_pref_destroy(HybridPrefEntry *entry)
{
    return;
}

guint
select_pref_add_entry(GtkWidget *section, guint pos, HybridPrefEntry *entry)
{
    GtkWidget *combo;
    GtkWidget *label;
    str_o_int value;
    gint i;
    gint active = -1;
    SelectOption *options = entry->data;
    str_o_int *data;

    label = gtk_label_new(entry->name);
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    if (entry->tooltip)
        gtk_widget_set_tooltip_markup(label, entry->tooltip);

    combo = gtk_combo_box_text_new();
    if (entry->type_num == PREF_KEY_SELECT) {
        value.str = hybrid_pref_get_string(entry->win->pref, entry->key);
    } else {
        value.num = hybrid_pref_get_int(entry->win->pref, entry->key);
    }
    for (i = 0;options[i].name;i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo),
                                       options[i].name);
        if (entry->type_num == PREF_KEY_SELECT) {
            if (!g_strcmp0(options[i].value.str, value.str))
                active = i;
        } else {
            if (options[i].value.num == value.num)
                active = i;
        }
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), active);

    entry->data = data = g_new0(str_o_int, i + 2);
    data[0].str = (gpointer)(data + i + 1);
    data[1].str = (gpointer)combo;
    if (entry->type_num == PREF_KEY_SELECT) {
        for (i++;i > 1;i--)
            data[i].str = g_strdup(options[i - 2].value.str);
    } else {
        for (i++;i > 1;i--)
            data[i].num = options[i - 2].value.num;
    }

    gtk_table_attach_defaults(GTK_TABLE(section), label, 0, 1, pos, pos + 1);
    gtk_table_attach_defaults(GTK_TABLE(section), combo, 1, 2, pos, pos + 1);
    if (entry->type_num == PREF_KEY_SELECT && value.p)
        g_free(value.p);
    return 1;
}

void
select_pref_save(HybridPrefEntry *entry)
{
    str_o_int *data = entry->data;
    gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(data[1].str));
    if (active < 0 || (active + data + 2) > (str_o_int *)(data->str)) {
        hybrid_debug_error("select_save", "active id out of range.");
        return;
    }
    if (entry->type_num == PREF_KEY_SELECT) {
        hybrid_pref_set_string(entry->win->pref, entry->key,
                               data[active + 2].str);
    } else {
        hybrid_pref_set_int(entry->win->pref, entry->key,
                            data[active + 2].num);
    }
}

void
select_pref_destroy(HybridPrefEntry *entry)
{
    gpointer *p;
    gpointer *data = entry->data;
    if (entry->type_num == PREF_KEY_SELECT) {
        for (p = (gpointer*)data[0];p > (data + 1);p--) {
            g_free(*p);
        }
    }
    g_free(data);
    return;
}

GtkWidget*
hybrid_pref_win_add_tab(HybridPrefWin *pref_win, const gchar *name)
{
    GtkWidget *page;
    GtkWidget *label;
    GtkWidget *align;

    page = gtk_vbox_new(FALSE, 10);
    align = gtk_alignment_new(0, 0, 1, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(align), 4, 10, 4, 10);
    gtk_container_add(GTK_CONTAINER(align), page);
    label = gtk_label_new(name);
    gtk_notebook_append_page(GTK_NOTEBOOK(pref_win->notebook),
                             align, label);

    return page;
}

GtkWidget*
hybrid_pref_tab_add_section(GtkWidget *tab, const gchar *name)
{
    GtkWidget *child;
    GtkWidget *frame;

    frame = gtk_frame_new(name ? name : "");
    gtk_container_set_border_width(GTK_CONTAINER(frame), 10);
    child = gtk_table_new(1, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(child), 6);
    gtk_table_set_col_spacings(GTK_TABLE(child), 10);
    gtk_container_add(GTK_CONTAINER(frame), child);
    gtk_container_add(GTK_CONTAINER(tab), frame);

    return child;
}

static void
entry_destroy_cb(HybridPrefEntry *entry)
{
    if (entry->type && entry->type->destroy)
        entry->type->destroy(entry);
    if (entry->name)
        g_free(entry->name);
    if (entry->key)
        g_free(entry->key);
    if (entry->tooltip)
        g_free(entry->tooltip);
    g_free(entry);
}

static void
entry_response_cb(GtkDialog *dialog, gint response_id, HybridPrefEntry *entry)
{
    if (response_id == GTK_RESPONSE_ACCEPT)
        entry->type->save(entry);
}

void
hybrid_pref_section_add_entry(HybridPrefWin *pref_win, GtkWidget *section,
                              PrefKeyType type, gchar *name, gchar *key,
                              gchar *tooltip, gpointer data)
{
    PrefAddFuncs *funcs;
    HybridPrefEntry *entry;
    guint width;
    guint height;
    guint delta;
    guint tmp;

    if (type >= PREF_KEY_MAX || type < PREF_KEY_NONE) {
        hybrid_debug_error("pref_add_entry", "Unknown type %d", type);
        return;
    }

    if (type == PREF_KEY_NONE) {
        hybrid_debug_error("pref_add_entry",
                           "Doesn't support custom type yet.");
        return;
    }

    /* Dosen't make sence for these two field to be NULL. */
    if (!(name || key)) {
        hybrid_debug_error("pref_add_entry", "name or key is NULL.");
        return;
    }

    funcs = pref_types[type];

    entry = g_new0(HybridPrefEntry, 1);
    entry->name = g_strdup(name);
    entry->key = g_strdup(key);
    entry->tooltip = tooltip ? g_strdup(tooltip) : NULL;
    entry->data = data;
    entry->type = funcs;
    entry->win = pref_win;
    entry->type_num = type;

    gtk_table_get_size(GTK_TABLE(section), &height, &width);
    delta = funcs->add_entry(section, height - 1, entry);
    if (!delta) {
        entry->type = NULL;
        entry_destroy_cb(entry);
        hybrid_debug_error("pref_add_entry", "Cannot add entry.");
        return;
    }
    gtk_table_get_size(GTK_TABLE(section), &tmp, &width);
    gtk_table_resize(GTK_TABLE(section), height + delta, width);

    g_signal_connect(pref_win->window, "response",
                     G_CALLBACK(entry_response_cb), entry);
    g_signal_connect_swapped(section, "destroy",
                             G_CALLBACK(entry_destroy_cb), entry);
}

static void
response_cb(GtkDialog *dialog, gint response_id, HybridPrefWin *pref_win)
{
    hybrid_pref_save(pref_win->pref);
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

void hybrid_pref_win_finish(HybridPrefWin *pref_win)
{
    g_signal_connect(pref_win->window, "response",
                     G_CALLBACK(response_cb), pref_win);
    gtk_widget_show_all(pref_win->window);
}

/**
 * Initialize the basic settings page.
 */
void
pref_basic_init(GtkWidget *tab)
{
    GtkWidget *section = hybrid_pref_tab_add_section(tab, _("Chat Window"));
    int i, j;

    hybrid_pref_section_add_entry(main_pref_window, section, PREF_KEY_BOOL,
                                  _("Hide Action Buttons"), "hide_chat_buttons",
                                  _("Hide Action Buttons"), NULL);

    HybridChatTheme *themes = hybrid_chat_window_get_themes();

    for (i = 0;themes[i].name;i++);

    SelectOption *options = g_new0(SelectOption, i+1);

    for (j = 0;j < i;j++) {
        options[j].name = options[j].value.str = themes[j].name;
    }

    hybrid_pref_section_add_entry(main_pref_window, section, PREF_KEY_SELECT,
                                  _("Chat Theme:"), "chat_theme",
                                  _("Chat Theme:"), options);
    g_free(options);

    section = hybrid_pref_tab_add_section(tab, _("Tabs"));
    hybrid_pref_section_add_entry(
        main_pref_window, section, PREF_KEY_BOOL,
        _("Show Messages In A Single Window With Tabs"), "single_chat_window",
        _("Show Messages In A Single Window With Tabs"), NULL);

    SelectOption pos_options[] = {
        {
            .name = _("Top"),
            .value.num = GTK_POS_TOP
        }, {
            .name = _("Right"),
            .value.num = GTK_POS_RIGHT
        }, {
            .name = _("Bottom"),
            .value.num = GTK_POS_BOTTOM
        }, {
            .name = _("Left"),
            .value.num = GTK_POS_LEFT
        }, {
            .name = NULL
        }
    };
    hybrid_pref_section_add_entry(main_pref_window, section, PREF_KEY_ENUM,
                                  _("Tab Position:"), "tab_pos",
                                  _("Tab Position:"), &pos_options);
}

/**
 * Initialize the sound settings panel.
 */
static void
pref_sound_init(GtkWidget *tab)
{
    GtkWidget *section = hybrid_pref_tab_add_section(tab, NULL);

    hybrid_pref_section_add_entry(main_pref_window, section, PREF_KEY_BOOL,
                                  _("Mute"), "mute", _("Mute"), NULL);
}

/**
 * Initialize the preference window.
 */
void
pref_window_init(void)
{
}

/**
 * Callback function for destroying the preference window.
 */
static void
destroy_cb(GtkWidget *widget, gpointer pref_win)
{
    g_free(pref_win);
}

static void
main_destroy_cb(GtkWidget *widget, gpointer p)
{
    main_pref_window = NULL;
}

HybridPrefWin*
hybrid_pref_win_new(HybridPref *pref, const gchar *title)
{
    HybridPrefWin *pref_win;
    GtkWidget *content_area;
    GdkPixbuf *icon;
    title = title ? title : _("Preference");
    pref_win = g_new0(HybridPrefWin, 1);

    pref_win->pref = pref;
    /* Use dialog window in order to be tiling wm/filter friendly. */
    /* TODO put this into hybrid_create_window */
    pref_win->window = gtk_dialog_new_with_buttons(title, NULL, 0,
                                                   GTK_STOCK_OK,
                                                   GTK_RESPONSE_ACCEPT,
                                                   GTK_STOCK_CANCEL,
                                                   GTK_RESPONSE_REJECT,
                                                   NULL);
    icon = hybrid_create_default_icon(0);
    gtk_window_set_icon(GTK_WINDOW(pref_win->window), icon);
    gtk_window_set_resizable(GTK_WINDOW(pref_win->window), FALSE);
    gtk_window_set_position(GTK_WINDOW(pref_win->window), GTK_WIN_POS_CENTER);

    /* doesn't set the pointer to NULL, need to be handled elsewhere */
    /* by connect_after to the same signal? */
    g_signal_connect(pref_win->window, "destroy",
                     G_CALLBACK(destroy_cb), pref_win);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(pref_win->window));

    gtk_container_set_border_width(GTK_CONTAINER(content_area), 8);

    pref_win->notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(content_area), pref_win->notebook);

    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(pref_win->notebook), GTK_POS_TOP);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(pref_win->notebook), TRUE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(pref_win->notebook), TRUE);

    return pref_win;
}

void
hybrid_pref_create(void)
{
    GtkWidget *tab;
    if (main_pref_window) {
        gtk_window_present(GTK_WINDOW(main_pref_window->window));
        return;
    }

    main_pref_window = hybrid_pref_win_new(NULL, NULL);

    g_signal_connect_after(main_pref_window->window, "destroy",
                           G_CALLBACK(main_destroy_cb), NULL);

    tab = hybrid_pref_win_add_tab(main_pref_window, _("Basic Settings"));
    pref_basic_init(tab);

    tab = hybrid_pref_win_add_tab(main_pref_window, _("Sound"));
    pref_sound_init(tab);

    gtk_widget_set_size_request(main_pref_window->window, 450, 300);

    hybrid_pref_win_finish(main_pref_window);
}

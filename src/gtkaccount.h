#ifndef IM_GTKACCOUNT_H
#define IM_GTKACCOUNT_H

#include <gtk/gtk.h>

typedef struct _HybirdAccountPanel HybirdAccountPanel;
typedef struct _HybirdAccountEditPanel HybirdAccountEditPanel;

struct _HybirdAccountPanel {
	GtkWidget *window;
	GtkListStore *account_store;
	GtkWidget *account_tree;
};

struct _HybirdAccountEditPanel {
	GtkWidget *window;
	GtkWidget *username_entry;
	GtkWidget *password_entry;
	GtkWidget *proto_combo;
};

enum {
	HYBIRD_ENABLE_COLUMN,
	HYBIRD_NAME_COLUMN,
	HYBIRD_PROTO_ICON_COLUMN,
	HYBIRD_PROTO_NAME_COLUMN,
	HYBIRD_ACCOUNT_COLUMN,
	HYBIRD_ACCOUNT_COLUMNS
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create an account management panel.
 */
HybirdAccountPanel *hybird_account_panel_create();

#ifdef __cplusplus
}
#endif

#endif /* IM_GTKACCOUNT_H */

#ifndef IM_GTKACCOUNT_H
#define IM_GTKACCOUNT_H

#include <gtk/gtk.h>

typedef struct _HybridAccountPanel HybridAccountPanel;
typedef struct _HybridAccountEditPanel HybridAccountEditPanel;

struct _HybridAccountPanel {
	GtkWidget *window;
	GtkListStore *account_store;
	GtkWidget *account_tree;
};

struct _HybridAccountEditPanel {
	GtkWidget *window;
	GtkWidget *username_entry;
	GtkWidget *password_entry;
	GtkWidget *proto_combo;
};

enum {
	HYBRID_ENABLE_COLUMN,
	HYBRID_NAME_COLUMN,
	HYBRID_PROTO_ICON_COLUMN,
	HYBRID_PROTO_NAME_COLUMN,
	HYBRID_ACCOUNT_COLUMN,
	HYBRID_ACCOUNT_COLUMNS
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create an account management panel.
 */
HybridAccountPanel *hybrid_account_panel_create();

#ifdef __cplusplus
}
#endif

#endif /* IM_GTKACCOUNT_H */

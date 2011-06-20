#ifndef IM_INFO_H
#define IM_INFO_H

#include <gtk/gtk.h>

typedef struct _HybridInfo HybridInfo;
typedef struct _HybridInfoItem HybridInfoItem;

#include "blist.h"

struct _HybridInfo {
	GtkWidget *window;
	GtkWidget *treeview;
	GtkListStore *store;
	HybridBuddy *buddy;
	GSList *item_list;
};

struct _HybridInfoItem {
	gchar *name;
	gchar *value;
};

enum {
	HYBRID_INFO_NAME_COLUMN,
	HYBRID_INFO_VALUE_COLUMN,
	HYBRID_INFO_COLUMNS
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a profile info panel, and push it to the list.
 * If the panel associated with the given buddy ID exists in
 * the panel list, just return it instead of creating a new one.
 *
 * @param buddy The buddy whose profile panel to create.
 *
 * @return The profile info panel created.
 */
HybridInfo *hybrid_info_create(HybridBuddy *buddy);

/**
 * Add a name-value pair to the info panel.
 *
 * @param info The info panel context.
 * @param name The name of the pair.
 * @param value The value of the pair.
 */
void hybrid_info_add_pair(HybridInfo *info, const gchar *name,
		const gchar *value);

#ifdef __cplusplus
}
#endif

#endif /* IM_INFO_H */

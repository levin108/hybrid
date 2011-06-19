#ifndef IM_INFO_H
#define IM_INFO_H

#include <gtk/gtk.h>

typedef struct _HybirdInfo HybirdInfo;
typedef struct _HybirdInfoItem HybirdInfoItem;

#include "blist.h"

struct _HybirdInfo {
	GtkWidget *window;
	GtkWidget *treeview;
	GtkListStore *store;
	HybirdBuddy *buddy;
	GSList *item_list;
};

struct _HybirdInfoItem {
	gchar *name;
	gchar *value;
};

enum {
	HYBIRD_INFO_NAME_COLUMN,
	HYBIRD_INFO_VALUE_COLUMN,
	HYBIRD_INFO_COLUMNS
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
HybirdInfo *hybird_info_create(HybirdBuddy *buddy);

/**
 * Add a name-value pair to the info panel.
 *
 * @param info The info panel context.
 * @param name The name of the pair.
 * @param value The value of the pair.
 */
void hybird_info_add_pair(HybirdInfo *info, const gchar *name,
		const gchar *value);

#ifdef __cplusplus
}
#endif

#endif /* IM_INFO_H */

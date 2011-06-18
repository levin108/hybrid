#ifndef IM_INFO_H
#define IM_INFO_H

#include <gtk/gtk.h>

typedef struct _HybirdInfo HybirdInfo;

#include "blist.h"

struct _HybirdInfo {
	GtkWidget *window;
	GtkWidget *treeview;
	GtkTreeStore *store;
	HybirdBuddy *buddy;
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

#ifdef __cplusplus
}
#endif

#endif /* IM_INFO_H */

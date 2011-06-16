#ifndef Hybird_BLIST_H
#define Hybird_BLIST_H
#include <gtk/gtk.h>

#define GROUPNAME_LENGTH   320
#define CONTACTNAME_LENGTH 320
#include "account.h"

typedef struct _HybirdBlist    HybirdBlist;
typedef struct _HybirdGroup   HybirdGroup;
typedef struct _HybirdBuddy HybirdBuddy;

struct _HybirdBlist {
	GtkWidget *treeview;
	GtkTreeStore *treemodel;
	GtkTreeViewColumn *column;
};

struct _HybirdGroup {
	GtkTreeIter iter;
	HybirdAccount *account;
	gchar *id;
	gchar *name;
};

struct _HybirdBuddy {
	GtkTreeIter iter;
	HybirdAccount *account;
	HybirdGroup *parent;
	gchar *id;	/**< User Identity. */
	gchar *name; /**< The name string. */
	gchar *mood; /**< The mood phrase. */
	gint   state; /**< The online status. */
	guchar *icon_data; /**< The portrait raw data. */
	gsize icon_data_length;
};

enum {
	Hybird_BLIST_BUDDY_ID,
	Hybird_BLIST_STATUS_ICON,
	Hybird_BLIST_BUDDY_NAME,
	Hybird_BLIST_PROTO_ICON,
	Hybird_BLIST_BUDDY_ICON,
	Hybird_BLIST_BUDDY_STATE,
	Hybird_BLIST_GROUP_EXPANDER_COLUMN_VISIBLE,
	Hybird_BLIST_CONTACT_EXPANDER_COLUMN_VISIBLE,
	Hybird_BLIST_STATUS_ICON_COLUMN_VISIBLE,
	Hybird_BLIST_PROTO_ICON_COLUMN_VISIBLE,
	Hybird_BLIST_BUDDY_ICON_COLUMN_VISIBLE,
	Hybird_BLIST_COLUMNS
};

/**
 * Test the buddy's online state.
 */
#define BUDDY_IS_ONLINE(b)    ((b)->state == Hybird_STATE_ONLINE)
#define BUDDY_IS_OFFLINE(b)   ((b)->state == Hybird_STATE_OFFLINE)
#define BUDDY_IS_AWAY(b)      ((b)->state == Hybird_STATE_AWAY)
#define BUDDY_IS_BUSY(b)      ((b)->state == Hybird_STATE_BUSY)
#define BUDDY_IS_INVISIBLE(b) ((b)->state == Hybird_STATE_INVISIBLE)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create an HybirdBlist struct, the memory allocated should be freed.
 *
 * @return The HybirdBList struct created
 */
HybirdBlist *hybird_blist_create();

/**
 * Initialize the specified HybirdBlist struct.
 *
 * @param blist HybirdBlist struct to be initialized.
 */
void hybird_blist_init();

/**
 * Add a new group to the buddy list
 *
 * @param ac The account context.
 * @param id Id of the new group to add
 * @param name Name of the new group to add
 *
 * @return The new added group
 */
HybirdGroup *hybird_blist_add_group(HybirdAccount *ac, const gchar *id, const gchar *name);

/**
 * Add a new buddy to the buddy list
 *
 * @param ac The account context.
 * @param parent Group to which the new buddy belongs
 * @param id String ID that unique identify the new buddy
 * @param name Name of the new buddy to add
 *
 * @return The new added contact
 */
HybirdBuddy *hybird_blist_add_buddy(HybirdAccount *ac, HybirdGroup *parent, 
		const gchar *id, const gchar *name);

/**
 * Set the buddy's display name.
 *
 * @param buddy The buddy to be set
 * @param name The buddy name to set
 */
void hybird_blist_set_buddy_name(HybirdBuddy *buddy, const gchar *name);

/**
 * Set the buddy's mood phrase.
 *
 * @param buddy The buddy to be set.
 * @param name The buddy name to set.
 */
void hybird_blist_set_buddy_mood(HybirdBuddy *buddy, const gchar *name);

/**
 * Set the buddy's state.
 *
 * @param buddy The buddy to be set.
 * @param The state number to set.
 */
void hybird_blist_set_buddy_state(HybirdBuddy *buddy, gint state);

/**
 * Set the buddy's portrait icon.
 *
 * @param buddy The buddy to set.
 * @param icon_data The icon raw data.
 * @param len The length of the icon raw data.
 */
void hybird_blist_set_buddy_icon(HybirdBuddy *buddy,
		const guchar *icon_data, gsize len);

/**
 * Find a group with the specified id.
 *
 * @param id ID of the group to find.
 *
 * @return HybirdGroup if fount, NULL if no group was found.
 */
HybirdGroup *hybird_blist_find_group_by_id(const gchar *id);

/**
 * Find a group with the specified name.
 *
 * @param id Name of the group to find.
 *
 * @return HybirdGroup if found, NULL if no group was found.
 */
HybirdGroup *hybird_blist_find_group_by_name(const gchar *name);

/**
 * Find a buddy with the specified ID.
 *
 * @param id The buddy ID.
 *
 * @return HybirdBuddy if found, NULL if no buddy was found.
 */
HybirdBuddy *hybird_blist_find_buddy(const gchar *id);

#ifdef _cplusplus
}
#endif

#endif /* Hybird_BLIST_H */

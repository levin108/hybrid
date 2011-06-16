#ifndef HYBIRD_BLIST_H
#define HYBIRD_BLIST_H
#include <gtk/gtk.h>

#define GROUPNAME_LENGTH   320
#define CONTACTNAME_LENGTH 320
#include "xmlnode.h"
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

	xmlnode *cache_node; /**< The corresponding xml node in cache context.*/

	gchar *id;	/**< User Identity. */
	gchar *name; /**< The name string. */
	gchar *mood; /**< The mood phrase. */
	gint   state; /**< The presence status. */
	guchar *icon_data; /**< The portrait raw data. */
	gsize icon_data_length; /**< The size of the portrait raw data */
	gchar *icon_crc; /**< The portrait crc. */
};

enum {
	HYBIRD_BLIST_BUDDY_ID,
	HYBIRD_BLIST_STATUS_ICON,
	HYBIRD_BLIST_BUDDY_NAME,
	HYBIRD_BLIST_PROTO_ICON,
	HYBIRD_BLIST_BUDDY_ICON,
	HYBIRD_BLIST_BUDDY_STATE,
	HYBIRD_BLIST_GROUP_EXPANDER_COLUMN_VISIBLE,
	HYBIRD_BLIST_CONTACT_EXPANDER_COLUMN_VISIBLE,
	HYBIRD_BLIST_STATUS_ICON_COLUMN_VISIBLE,
	HYBIRD_BLIST_PROTO_ICON_COLUMN_VISIBLE,
	HYBIRD_BLIST_BUDDY_ICON_COLUMN_VISIBLE,
	HYBIRD_BLIST_COLUMNS
};

/**
 * Test the buddy's online state.
 */
#define BUDDY_IS_ONLINE(b)    ((b)->state == HYBIRD_STATE_ONLINE)
#define BUDDY_IS_OFFLINE(b)   ((b)->state == HYBIRD_STATE_OFFLINE)
#define BUDDY_IS_AWAY(b)      ((b)->state == HYBIRD_STATE_AWAY)
#define BUDDY_IS_BUSY(b)      ((b)->state == HYBIRD_STATE_BUSY)
#define BUDDY_IS_INVISIBLE(b) ((b)->state == HYBIRD_STATE_INVISIBLE)

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
 * Destroy a group context and free the memory allocated.
 *
 * @param group The group context to destroy.
 */
void hybird_blist_group_destroy(HybirdGroup *group);

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
 * Destroy a buddy context and free the memory allocated.
 *
 * @param buddy The buddy context to destroy.
 */
void hybird_blist_buddy_destroy(HybirdBuddy *buddy);
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
 * @param crc The checksum of the icon.
 */
void hybird_blist_set_buddy_icon(HybirdBuddy *buddy,
		const guchar *icon_data, gsize len, const gchar *crc);

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

/**
 * Write the buddy information to the cache which in fact is 
 * a XML tree in the memory, if you want to synchronize the cache
 * with the cache file, use hybird_blist_cache_flush().
 *
 * @param buddy The buddy to write to cache.
 */
void hybird_blist_buddy_to_cache(HybirdBuddy *buddy);
#ifdef _cplusplus
}
#endif

#endif /* HYBIRD_BLIST_H */

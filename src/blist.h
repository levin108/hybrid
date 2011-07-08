#ifndef HYBRID_BLIST_H
#define HYBRID_BLIST_H
#include <gtk/gtk.h>

#define GROUPNAME_LENGTH   320
#define CONTACTNAME_LENGTH 320

typedef struct _HybridBlist HybridBlist;
typedef struct _HybridGroup HybridGroup;
typedef struct _HybridBuddy HybridBuddy;
typedef enum _HybridBlistCacheType HybridBlistCacheType;

#include "xmlnode.h"
#include "account.h"

struct _HybridBlist {
	GtkWidget *treeview;
	GtkTreeStore *treemodel;
	GtkTreeViewColumn *text_column;
	GtkCellRenderer *text_renderer;
};

struct _HybridGroup {
	GtkTreeIter iter;
	HybridAccount *account; /**< The corresponding xml node in cache context.*/

	xmlnode *cache_node;

	gint buddy_count; /**< count of the buddies belonging to this group. */
	gint online_count; /**< count of the online buddies belonging to this group.*/
	gchar *id;
	gchar *name;
	gint renamable; /**< whether this group can be renamed. */
};

struct _HybridBuddy {
	GtkTreeIter iter;
	HybridAccount *account;
	HybridGroup *parent;

	xmlnode *cache_node; /**< The corresponding xml node in cache context.*/

	gchar *id;	/**< User Identity. */
	gchar *name; /**< The name string. */
	gchar *mood; /**< The mood phrase. */
	gint   state; /**< The presence status. */
	gint status; /**< 0 if this buddy is normal, 1 if it's unathorized. */
	gchar *icon_name; /**< The portrait file name.  */
	guchar *icon_data; /**< The portrait raw data. */
	gsize icon_data_length; /**< The size of the portrait raw data */
	gchar *icon_crc; /**< The portrait crc. */
};

enum {
	HYBRID_BLIST_BUDDY_ID,
	HYBRID_BLIST_STATUS_ICON,
	HYBRID_BLIST_BUDDY_NAME,
	HYBRID_BLIST_PROTO_ICON,
	HYBRID_BLIST_BUDDY_ICON,
	HYBRID_BLIST_BUDDY_STATE,
	HYBRID_BLIST_OBJECT_COLUMN,
	HYBRID_BLIST_GROUP_EXPANDER_COLUMN_VISIBLE,
	HYBRID_BLIST_CONTACT_EXPANDER_COLUMN_VISIBLE,
	HYBRID_BLIST_STATUS_ICON_COLUMN_VISIBLE,
	HYBRID_BLIST_PROTO_ICON_COLUMN_VISIBLE,
	HYBRID_BLIST_BUDDY_ICON_COLUMN_VISIBLE,
	HYBRID_BLIST_COLUMNS
};

enum _HybridBlistCacheType {
	HYBRID_BLIST_CACHE_ADD,           /**< Add a new item to the blist cache. */
	HYBRID_BLIST_CACHE_UPDATE_NAME,   /**< Update the name of an existing item. */
	HYBRID_BLIST_CACHE_UPDATE_MOOD,   /**< Update the mood of an existing item. */
	HYBRID_BLIST_CACHE_UPDATE_ICON,   /**< Update the icon of an existing item. */
	HYBRID_BLIST_CACHE_UPDATE_STATUS  /**< Update the status of an existing item. */
};

/**
 * Test the buddy's online state.
 */
#define BUDDY_IS_ONLINE(b)    ((b)->state == HYBRID_STATE_ONLINE)
#define BUDDY_IS_OFFLINE(b)   ((b)->state == HYBRID_STATE_OFFLINE)
#define BUDDY_IS_AWAY(b)      ((b)->state == HYBRID_STATE_AWAY)
#define BUDDY_IS_BUSY(b)      ((b)->state == HYBRID_STATE_BUSY)
#define BUDDY_IS_INVISIBLE(b) ((b)->state == HYBRID_STATE_INVISIBLE)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create an HybridBlist struct, the memory allocated should be freed.
 *
 * @return The HybridBList struct created
 */
HybridBlist *hybrid_blist_create();

/**
 * Initialize the specified HybridBlist struct.
 *
 * @param blist HybridBlist struct to be initialized.
 */
void hybrid_blist_init();

/**
 * Add a new group to the buddy list
 *
 * @param ac The account context.
 * @param id Id of the new group to add
 * @param name Name of the new group to add
 *
 * @return The new added group
 */
HybridGroup *hybrid_blist_add_group(HybridAccount *ac, const gchar *id, const gchar *name);

/**
 * Remove a group.
 *
 * @param group The group to remove.
 */
void hybrid_blist_remove_group(HybridGroup *group);

/**
 * Destroy a group context and free the memory allocated.
 *
 * @param group The group context to destroy.
 */
void hybrid_blist_group_destroy(HybridGroup *group);

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
HybridBuddy *hybrid_blist_add_buddy(HybridAccount *ac, HybridGroup *parent, 
		const gchar *id, const gchar *name);

/**
 * Remove a buddy.
 *
 * @param buddy The buddy to remove.
 */
void hybrid_blist_remove_buddy(HybridBuddy *buddy);

/**
 * Destroy a buddy context and free the memory allocated.
 *
 * @param buddy The buddy context to destroy.
 */
void hybrid_blist_buddy_destroy(HybridBuddy *buddy);

/**
 * Set the buddy's display name.
 *
 * @param buddy The buddy to be set
 * @param name The buddy name to set
 */
void hybrid_blist_set_buddy_name(HybridBuddy *buddy, const gchar *name);

/**
 * Private version of the hybrid_blist_set_buddy_name()
 */
void hybrid_blist_set_buddy_name_priv(HybridBuddy *buddy, const gchar *name);

/**
 * Set the buddy's mood phrase.
 *
 * @param buddy The buddy to be set.
 * @param name The buddy name to set.
 */
void hybrid_blist_set_buddy_mood(HybridBuddy *buddy, const gchar *name);

/**
 * Private version of the hybrid_blist_set_buddy_mood().
 */
void hybrid_blist_set_buddy_mood_priv(HybridBuddy *buddy, const gchar *name);

/**
 * Set the buddy's state.
 *
 * @param buddy The buddy to be set.
 * @param The state number to set.
 */
void hybrid_blist_set_buddy_state(HybridBuddy *buddy, gint state);

/**
 * Set the buddy's status, note that 'status' is not 'state' which presents
 * the buddy's presence state, whereas 'status' presents that whether this
 * buddy has been authorized.
 *
 * @param buddy      The buddy to be set.
 * @param authorized TRUE if the buddy has been authorized, in other word, it's
 *                   a normal buddy, FALSE if the buddy has not been authorized,
 *                   which in most protocols means that you can not manipulate
 *                   this buddy.
 */
void hybrid_blist_set_buddy_status(HybridBuddy *buddy, gboolean authorized);

/**
 * Private version of the hybrid_blist_set_buddy_status()
 */
void hybrid_blist_set_buddy_status_priv(HybridBuddy *buddy, gboolean authorized);

/**
 * Get whether a buddy has been authorized.
 *
 * @param buddy The buddy.
 * 
 * @return TRUE if the buddy if authorized, FALSE if not.
 */
gboolean hybrid_blist_get_buddy_authorized(HybridBuddy *buddy);

/**
 * Set the buddy's portrait icon. If you want to set portrait icon
 * to the default icon, just set the icon_data to NULL, at the same time,
 * the len must be 0.
 *
 * @param buddy     The buddy to set.
 * @param icon_data The icon raw data.
 * @param len       The length of the icon raw data.
 * @param crc       The checksum of the icon.
 */
void hybrid_blist_set_buddy_icon(HybridBuddy *buddy,
		const guchar *icon_data, gsize len, const gchar *crc);

/**
 * Get the portrait checksum of the specified buddy.
 *
 * @param buddy The buddy.
 *
 * @return The checksum of the buddy's portrait.
 */
const gchar *hybrid_blist_get_buddy_checksum(HybridBuddy *buddy);

/**
 * Set whether the group can be renamed.
 *
 * @param group The group.
 * @param renamable TRUE if this group can be renamed, otherwise FALSE.
 */
void hybrid_blist_set_group_renamable(HybridGroup *group, gboolean renamable);

/**
 * Get whether the group can be renamed.
 *
 * @param group The group.
 * 
 * @return TRUE if this group can be renamed, otherwise FALSE.
 */
gboolean hybrid_blist_get_group_renamable(HybridGroup *group);

/**
 * Find a group with the specified id.
 *
 * @param account The account to which the group belongs.
 * @param id      ID of the group to find.
 *
 * @return HybridGroup if fount, NULL if no group was found.
 */
HybridGroup *hybrid_blist_find_group(HybridAccount *account, const gchar *id);

/**
 * Find a buddy with the specified ID.
 *
 * @param account The account to which the buddy belongs.
 * @param id      The buddy ID.
 *
 * @return HybridBuddy if found, NULL if no buddy was found.
 */
HybridBuddy *hybrid_blist_find_buddy(HybridAccount *account, const gchar *id);

/**
 * Select the first item of the account, it's used when an new
 * account was enabled, in this time, if there was no account whose
 * account was selected, then we do nothing, but if there was, we
 * show make it select the items of the current account.
 *
 * @param account The account.
 */
void hybrid_blist_select_first_item(HybridAccount *account);

#ifdef _cplusplus
}
#endif

#endif /* HYBRID_BLIST_H */

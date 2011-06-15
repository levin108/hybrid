#ifndef IM_BLIST_H
#define IM_BLIST_H
#include <gtk/gtk.h>

#define GROUPNAME_LENGTH   320
#define CONTACTNAME_LENGTH 320
#include "account.h"

typedef struct _IMBlist    IMBlist;
typedef struct _IMGroup   IMGroup;
typedef struct _IMBuddy IMBuddy;

struct _IMBlist {
	GtkWidget *treeview;
	GtkTreeStore *treemodel;
	GtkTreeViewColumn *column;
};

struct _IMGroup {
	GtkTreeIter iter;
	IMAccount *account;
	gchar *id;
	gchar *name;
};

struct _IMBuddy {
	GtkTreeIter iter;
	IMAccount *account;
	IMGroup *parent;
	gchar *id;	/**< User Identity. */
	gchar *name; /**< The name string. */
	gchar *mood; /**< The mood phrase. */
	gint   state; /**< The online status. */
	guchar *icon_data; /**< The portrait raw data. */
	gsize icon_data_length;
};

enum {
	IM_BLIST_BUDDY_ID,
	IM_BLIST_STATUS_ICON,
	IM_BLIST_BUDDY_NAME,
	IM_BLIST_PROTO_ICON,
	IM_BLIST_BUDDY_ICON,
	IM_BLIST_BUDDY_STATE,
	IM_BLIST_GROUP_EXPANDER_COLUMN_VISIBLE,
	IM_BLIST_CONTACT_EXPANDER_COLUMN_VISIBLE,
	IM_BLIST_STATUS_ICON_COLUMN_VISIBLE,
	IM_BLIST_PROTO_ICON_COLUMN_VISIBLE,
	IM_BLIST_BUDDY_ICON_COLUMN_VISIBLE,
	IM_BLIST_COLUMNS
};

/**
 * Test the buddy's online state.
 */
#define BUDDY_IS_ONLINE(b)    ((b)->state == IM_STATE_ONLINE)
#define BUDDY_IS_OFFLINE(b)   ((b)->state == IM_STATE_OFFLINE)
#define BUDDY_IS_AWAY(b)      ((b)->state == IM_STATE_AWAY)
#define BUDDY_IS_BUSY(b)      ((b)->state == IM_STATE_BUSY)
#define BUDDY_IS_INVISIBLE(b) ((b)->state == IM_STATE_INVISIBLE)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create an IMBlist struct, the memory allocated should be freed.
 *
 * @return The IMBList struct created
 */
IMBlist *im_blist_create();

/**
 * Initialize the specified IMBlist struct.
 *
 * @param blist IMBlist struct to be initialized.
 */
void im_blist_init();

/**
 * Add a new group to the buddy list
 *
 * @param ac The account context.
 * @param id Id of the new group to add
 * @param name Name of the new group to add
 *
 * @return The new added group
 */
IMGroup *im_blist_add_group(IMAccount *ac, const gchar *id, const gchar *name);

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
IMBuddy *im_blist_add_buddy(IMAccount *ac, IMGroup *parent, 
		const gchar *id, const gchar *name);

/**
 * Set the buddy's display name.
 *
 * @param buddy The buddy to be set
 * @param name The buddy name to set
 */
void im_blist_set_buddy_name(IMBuddy *buddy, const gchar *name);

/**
 * Set the buddy's mood phrase.
 *
 * @param buddy The buddy to be set.
 * @param name The buddy name to set.
 */
void im_blist_set_buddy_mood(IMBuddy *buddy, const gchar *name);

/**
 * Set the buddy's state.
 *
 * @param buddy The buddy to be set.
 * @param The state number to set.
 */
void im_blist_set_buddy_state(IMBuddy *buddy, gint state);

/**
 * Set the buddy's portrait icon.
 *
 * @param buddy The buddy to set.
 * @param icon_data The icon raw data.
 * @param len The length of the icon raw data.
 */
void im_blist_set_buddy_icon(IMBuddy *buddy,
		const guchar *icon_data, gsize len);

/**
 * Find a group with the specified id.
 *
 * @param id ID of the group to find.
 *
 * @return IMGroup if fount, NULL if no group was found.
 */
IMGroup *im_blist_find_group_by_id(const gchar *id);

/**
 * Find a group with the specified name.
 *
 * @param id Name of the group to find.
 *
 * @return IMGroup if found, NULL if no group was found.
 */
IMGroup *im_blist_find_group_by_name(const gchar *name);

/**
 * Find a buddy with the specified ID.
 *
 * @param id The buddy ID.
 *
 * @return IMBuddy if found, NULL if no buddy was found.
 */
IMBuddy *im_blist_find_buddy(const gchar *id);

#ifdef _cplusplus
}
#endif

#endif /* IM_BLIST_H */

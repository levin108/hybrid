#ifndef HYBRID_FX_GROUP_H
#define HYBRID_FX_GROUP_H
#include <glib.h>
#include "fx_account.h"

typedef struct _fetion_group fetion_group;

struct _fetion_group {
	gint group_id;
	gchar *group_name;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a fetion group, needs to be destroyed after used.
 *
 * @param id The group id.
 * @param name The group name.
 *
 * @return The fetion group created.
 */
fetion_group *fetion_group_create(gint id, const gchar *name);

/**
 * Destroy a fetion group.
 *
 * @param group The fetion group to destroy.
 */
void fetion_group_destroy(fetion_group *group);

/**
 * Rename the group, the message is:
 *
 * S fetion.com.cn SIP-C/4.0
 * F: 999999999
 * I: 9
 * Q: 2 S
 * N: SetBuddyListInfo
 * L: 132
 * 
 * <args><contacts><buddy-lists>
 * <buddy-list id="2" name="text"/>
 * </buddy-lists></contacts></args>
 *
 * @param account The fetion account.
 * @param id      The id of the group to rename.
 * @param name    The new name.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint fetion_group_edit(fetion_account *account, const gchar *id,
						const gchar *name);

/**
 * Add a new group, the message is:
 *
 * S fetion.com.cn SIP-C/4.0
 * F: 547264589
 * I: 7
 * Q: 2 S
 * N: CreateBuddyList
 * L: 108
 *
 * <args><contacts><buddy-lists><buddy-list name="test"/>
 * </buddy-lists></contacts></args>
 *
 * @param account The fetion account.
 * @param name    The name of the new group.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint fetion_group_add(fetion_account *account, const gchar *name);

/* UI ops  */

/**
 * Init the group list in the UI TreeView.
 *
 * @param ac The fetion account context.
 */
void fetion_groups_init(fetion_account *ac);

#ifdef __cplusplus
}
#endif

#endif

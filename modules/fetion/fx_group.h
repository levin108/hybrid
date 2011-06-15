#ifndef IM_FX_GROUP_H
#define IM_FX_GROUP_H
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

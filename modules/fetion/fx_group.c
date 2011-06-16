#include "util.h"
#include "blist.h"

#include "fx_group.h"

fetion_group*
fetion_group_create(gint id, const gchar *name)
{
	fetion_group *group;

	g_return_val_if_fail(name != NULL, NULL);

	group = g_new0(fetion_group, 1);
	group->group_id = id;
	group->group_name = g_strdup(name);

	return group;
}

void
fetion_group_destroy(fetion_group *group)
{
	if (group) {
		g_free(group->group_name);
		g_free(group);
	}
}

void
fetion_groups_init(fetion_account *ac)
{
	GSList *pos;
	fetion_group *group;
	gchar buf[BUF_LENGTH];

	for (pos = ac->groups; pos; pos = pos->next) {
		group = (fetion_group*)pos->data;
		g_snprintf(buf, sizeof(buf) - 1, "%d", group->group_id);
		hybird_blist_add_group(ac->account, buf, group->group_name);
	}
}

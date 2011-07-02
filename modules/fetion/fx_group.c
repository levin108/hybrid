#include "util.h"
#include "blist.h"

#include "fx_trans.h"
#include "fx_group.h"

static gchar* generate_group_edit_body(const gchar *group_id, const gchar *group_name);

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

static gint
group_edit_cb(fetion_account *ac, const gchar *sipmsg,
			fetion_transaction *trans)
{
	hybrid_debug_info("fetion", "edit group, recv:\n%s", sipmsg);

	return HYBRID_OK;
}

gint
fetion_group_edit(fetion_account *account, const gchar *id,
						const gchar *name)
{
	fetion_sip *sip;
	sip_header *eheader;
	fetion_transaction *trans;
	gchar *body;
	gchar *sip_text;

	g_return_val_if_fail(account != NULL, HYBRID_ERROR);
	g_return_val_if_fail(id != NULL, HYBRID_ERROR);
	g_return_val_if_fail(name != NULL, HYBRID_ERROR);

	sip = account->sip;

	fetion_sip_set_type(sip, SIP_SERVICE);

	eheader = sip_event_header_create(SIP_EVENT_SETBUDDYLISTINFO);
	fetion_sip_add_header(sip, eheader);

	trans = transaction_create();
	transaction_set_callid(trans, sip->callid);
	transaction_set_callback(trans, group_edit_cb);
	transaction_add(account, trans);

	body = generate_group_edit_body(id, name);
	sip_text = fetion_sip_to_string(sip, body);
	g_free(body);

	hybrid_debug_info("fetion", "rename group,send\n%s", sip_text);

	if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {

		hybrid_debug_info("fetion", "rename group failed");

		return HYBRID_ERROR;
	}

	g_free(sip_text);

	return HYBRID_OK;
}

void
fetion_groups_init(fetion_account *ac)
{
	GSList *pos;
	HybridGroup *hgroup;
	fetion_group *group;
	gchar buf[BUF_LENGTH];

	for (pos = ac->groups; pos; pos = pos->next) {

		group = (fetion_group*)pos->data;

		g_snprintf(buf, sizeof(buf) - 1, "%d", group->group_id);
		hgroup = hybrid_blist_add_group(ac->account, buf, group->group_name);

		/* The group named 'Ungrouped' can't be renamed. */
		if (group->group_id == 0) {
			hybrid_blist_set_group_renamable(hgroup, FALSE);
		}
	}
}

static gchar*
generate_group_edit_body(const gchar *group_id, const gchar *group_name)
{
	const gchar *body;
	xmlnode *root;
	xmlnode *node;

	body = "<args></args>";

	root = xmlnode_root(body, strlen(body));

	node = xmlnode_new_child(root, "contacts");
	node = xmlnode_new_child(node, "buddy-lists");
	node = xmlnode_new_child(node, "buddy-list");

	xmlnode_new_prop(node, "id", group_id);
	xmlnode_new_prop(node, "name", group_name);

	return xmlnode_to_string(root);
}

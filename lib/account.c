#include <glib.h>
#include "util.h"
#include "account.h"
#include "blist.h"

static void load_blist_from_disk(HybirdAccount *account);

HybirdAccount*
hybird_account_create(HybirdModule *proto)
{
	extern HybirdConfig *global_config;
	g_return_val_if_fail(proto != NULL, NULL);

	HybirdAccount *ac = g_new0(HybirdAccount, 1);
	hybird_account_set_username(ac, "547264589");
	hybird_account_set_password(ac, "lwp1279");
	
	ac->config = global_config;
	ac->proto = proto;

	return ac;
}

void
hybird_account_destroy(HybirdAccount *account)
{
	if (account) {
		g_free(account->username);
		g_free(account->password);
		g_free(account);
	}
}

void 
hybird_account_set_username(HybirdAccount *account, const gchar *username)
{
	g_return_if_fail(account != NULL);

	g_free(account->username);

	account->username = g_strdup(username);
}

void 
hybird_account_set_password(HybirdAccount *account, const gchar *password)
{
	g_return_if_fail(account != NULL);

	g_free(account->password);

	account->password = g_strdup(password);
}

void
hybird_account_error_reason(HybirdAccount *account, const gchar *reason)
{
	/* TODO */
	g_return_if_fail(account != NULL);

	hybird_account_destroy(account);
}

void
hybird_account_set_connection_status(HybirdAccount *account,
		HybirdConnectionStatusType status)
{
	if (account->connect_state != HYBIRD_CONNECTION_CONNECTED &&
		status == HYBIRD_CONNECTION_CONNECTED) {

		/* 
		 * It's necessary to set the status value here, because if
		 * connection status is not HYBIRD_CONNECTION_CONNECTED, 
		 * hybird_blist_add_buddy() just return without doing anything,
		 * which will cause load_blist_from_disk() fail to add buddies.
		 */
		account->connect_state = status;

		/* here we load the blist from the local cache file. */
		load_blist_from_disk(account);
	}

	account->connect_state = status;
}

static void
load_blist_from_disk(HybirdAccount *account)
{
	HybirdConfig *config;
	HybirdBlistCache *cache;
	HybirdGroup *group;
	HybirdBuddy *buddy;
	xmlnode *root;
	xmlnode *node;
	xmlnode *group_node;
	xmlnode *buddy_node;
	gchar *id;
	gchar *name;
	gchar *value;
	guchar *icon_data;
	gsize icon_data_len;


	g_return_if_fail(account != NULL);

	config = account->config;
	cache = config->blist_cache;
	root = cache->root;

	if (!(node = xmlnode_find(root, "buddies"))) {
		hybird_debug_error("account", 
			"can't find node named 'buddies' in blist.xml");
		return;
	}

	group_node = xmlnode_child(node);

	while (group_node) {

		if (!xmlnode_has_prop(group_node, "id") ||
			!xmlnode_has_prop(group_node, "name")) {

			hybird_debug_error("account", "invalid group node");
			group_node = xmlnode_next(group_node);
			continue;
		}

		id = xmlnode_prop(group_node, "id");
		name = xmlnode_prop(group_node, "name");

		group = hybird_blist_add_group(account, id, name);
		
		/*
		 * Iterate the buddy nodes, use the attribute values
		 * to create new HybirdBuddys, and add them to the 
		 * buddy list and the GtkTreeView. 
		 */
		buddy_node = xmlnode_child(group_node);

		while (buddy_node) {

			if (!xmlnode_has_prop(buddy_node, "id") ||
				!xmlnode_has_prop(buddy_node, "name")) {

				hybird_debug_error("account", "invalid buddy node");
				buddy_node = xmlnode_next(buddy_node);
				continue;
			}

			id = xmlnode_prop(buddy_node, "id");
			name = xmlnode_prop(buddy_node, "name");

			buddy = hybird_blist_add_buddy(account, group, id, name);
			buddy->state = HYBIRD_STATE_OFFLINE;

			//buddy_node = xmlnode_next(buddy_node);
			//continue;

			//buddy->cache_node = buddy_node;

			g_free(id);
			g_free(name);

			if (xmlnode_has_prop(buddy_node, "mood")) {
				value = xmlnode_prop(buddy_node, "mood");
				hybird_blist_set_buddy_mood(buddy, value);
				g_free(value);
			}

			/*
			 * If buddy node has attribute named 'icon', and the value ot the 
			 * attribute is not empty, then load the file pointed by 
			 * the icon value as the buddy's portrait, orelse load the 
			 * default icon, we DONT need to load the default icon file
			 * any more, let the hybird_blist_set_buddy_icon() do it, just
			 * set the relevant argument to NULL.
			 */
			if (xmlnode_has_prop(buddy_node, "icon")) {
				value = xmlnode_prop(buddy_node, "icon");

				if (*value != '\0') {
					/* Set the buddy's icon name */
					g_free(buddy->icon_name);
					buddy->icon_name = g_strdup(value);

					name = g_strdup_printf("%s/%s", config->icon_path, value);

					g_file_get_contents(name, (gchar**)&icon_data,
							&icon_data_len, NULL);
					g_free(name);

				} else {
					icon_data = NULL;
					icon_data_len = 0;
				}
				
				g_free(value);

			} else {
				icon_data = NULL;
				icon_data_len = 0;
			}

			value = NULL;

			if (xmlnode_has_prop(buddy_node, "crc")) {
				value = xmlnode_prop(buddy_node, "crc");
				if (*value == '\0') {
					g_free(value);
					value = NULL;
				}
			}

			hybird_blist_set_buddy_icon(buddy, icon_data,
					icon_data_len, value);

			g_free(value);
			g_free(icon_data);

			buddy_node = xmlnode_next(buddy_node);
		}


		group_node = xmlnode_next(group_node);
	}

}

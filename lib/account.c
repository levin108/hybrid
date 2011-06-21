#include <glib.h>
#include "util.h"
#include "config.h"
#include "account.h"
#include "blist.h"

GSList *account_list = NULL;

static void load_blist_from_disk(HybridAccount *account);

void
hybrid_account_init(void)
{
	gchar *account_file;
	gchar *config_path;
	xmlnode *root;
	xmlnode *node;
	HybridAccount *account;
	gboolean flush = FALSE;

	gchar *username;
	gchar *protoname;
	gchar *password;
	HybridModule *module;

	config_path = hybrid_config_get_path();
	account_file = g_strdup_printf("%s/accounts.xml", config_path);
	g_free(config_path);

	if (!(root = xmlnode_root_from_file(account_file))) {
		const gchar *root_name = "<accounts></accounts>";
		hybrid_debug_info("account", "accounts.xml doesn't exist or"
				"in bad format, create a new one.");
		root = xmlnode_root(root_name, strlen(root_name));
		flush ^= 1;
	}

	if ((node = xmlnode_child(root))) {

		while (node) {
			if (g_strcmp0(node->name, "account") ||
				!xmlnode_has_prop(node, "user") ||
				!xmlnode_has_prop(node, "proto")) {
				hybrid_debug_error("account", "accounts.xml is in bad format,"
						"please try to remove ~/.config/hybrid/accounts.xml,"
						"and then restart hybrid :)");
				xmlnode_free(root);
				return;
			}

			username = xmlnode_prop(node, "user");
			protoname = xmlnode_prop(node, "proto");

			module = hybrid_module_find(protoname);

			account = hybrid_account_create(module);
			hybrid_account_set_username(account, username);

			if (xmlnode_has_prop(node, "pass")) {
				password = xmlnode_prop(node, "pass");
				hybrid_account_set_password(account, password);
				g_free(password);
			}

			g_free(username);
			g_free(protoname);

			account_list = g_slist_append(account_list, account);
				
			node = node->next;
		}
	}

	if (flush) {
		xmlnode_save_file(root, account_file);
	}

	xmlnode_free(root);
}

HybridAccount*
hybrid_account_get(const gchar *proto_name,	const gchar *username)
{
	HybridAccount *account;
	HybridModule *module;
	GSList *pos;

	for (pos = account_list; pos; pos = pos->next) {
		account = (HybridAccount*)pos->data;

		if (g_strcmp0(account->username, username) == 0 &&
			g_strcmp0(account->proto->info->name, proto_name) == 0) {

			return account;
		}
	}

	if (!(module = hybrid_module_find(proto_name))) {
		hybrid_debug_error("account", "create account error,"
				"invalid protocal name.");
		return NULL;
	}

	account = hybrid_account_create(module);
	hybrid_account_set_username(account, username);

	account_list = g_slist_append(account_list, account);

	return account;
}

void
hybrid_account_update(HybridAccount *account)
{
	gchar *account_file;
	gchar *config_path;
	xmlnode *root;
	xmlnode *node;
	gchar *username;
	gchar *protoname;

	config_path = hybrid_config_get_path();
	account_file = g_strdup_printf("%s/accounts.xml", config_path);
	g_free(config_path);

	if (!(root = xmlnode_root_from_file(account_file))) {
		hybrid_debug_error("account", "save account information failed,"
				"invalid accounts.xml");
		g_free(account_file);
		return;
	}

	if ((node = xmlnode_child(root))) {

		while (node) {
			if (g_strcmp0(node->name, "account") ||
				!xmlnode_has_prop(node, "user") ||
				!xmlnode_has_prop(node, "proto")) {
				hybrid_debug_error("account", 
						"invalid node found in accounts.xml");
				node = xmlnode_next(node);

				continue;
			}

			username = xmlnode_prop(node, "user");
			protoname = xmlnode_prop(node, "proto");

			if (g_strcmp0(username, account->username) == 0 &&
				g_strcmp0(protoname, account->proto->info->name) == 0) {
				goto update_node;
			}
			
			node = xmlnode_next(node);
		}
	}
	
	/* Node not found, create a new one. */
	node = xmlnode_new_child(root, "account");
update_node:

	if (xmlnode_has_prop(node, "proto")) {
		xmlnode_set_prop(node, "proto", account->proto->info->name);

	} else {
		xmlnode_new_prop(node, "proto", account->proto->info->name);
	}

	if (xmlnode_has_prop(node, "user")) {
		xmlnode_set_prop(node, "user", account->username);

	} else {
		xmlnode_new_prop(node, "user", account->username);
	}

	if (xmlnode_has_prop(node, "pass")) {
		xmlnode_set_prop(node, "pass", account->password);

	} else {
		xmlnode_new_prop(node, "pass", account->password);
	}

	xmlnode_save_file(root, account_file);

	g_free(account_file);
	xmlnode_free(root);
}

void
hybrid_account_remove(const gchar *protoname, const gchar *username)
{
	gchar *account_file;
	gchar *config_path;
	HybridAccount *account;
	xmlnode *root;
	xmlnode *node;
	GSList *pos;
	gchar *user_name;
	gchar *proto_name;

	/* Remove the revelent node from accounts.xml */
	config_path = hybrid_config_get_path();
	account_file = g_strdup_printf("%s/accounts.xml", config_path);
	g_free(config_path);

	if (!(root = xmlnode_root_from_file(account_file))) {
		hybrid_debug_error("account", "remove account information failed,"
				"invalid accounts.xml");
		g_free(account_file);
		return;
	}

	if ((node = xmlnode_child(root))) {

		while (node) {
			if (g_strcmp0(node->name, "account") ||
				!xmlnode_has_prop(node, "user") ||
				!xmlnode_has_prop(node, "proto")) {
				hybrid_debug_error("account", 
						"invalid node found in accounts.xml");
				node = xmlnode_next(node);

				continue;
			}

			user_name = xmlnode_prop(node, "user");
			proto_name = xmlnode_prop(node, "proto");

			if (g_strcmp0(user_name, username) == 0 &&
				g_strcmp0(proto_name, proto_name) == 0) {
				xmlnode_remove_node(node);
				xmlnode_save_file(root, account_file);
				break;
			}
			
			node = xmlnode_next(node);
		}
	}

	g_free(account_file);
	xmlnode_free(root);

	/* Remove the account from the global account list. */
	for (pos = account_list; pos; pos = pos->next) {
		account = (HybridAccount*)pos->data;

		if (g_strcmp0(account->username, username) == 0 &&
			g_strcmp0(account->proto->info->name, protoname) == 0) {

			account_list = g_slist_remove(account_list, account);
			hybrid_account_destroy(account);

			break;
		}
	}
}

HybridAccount*
hybrid_account_create(HybridModule *proto)
{
	extern HybridConfig *global_config;
	g_return_val_if_fail(proto != NULL, NULL);

	HybridAccount *ac = g_new0(HybridAccount, 1);
	
	ac->buddy_list = g_hash_table_new(g_str_hash, g_str_equal);
	ac->group_list = g_hash_table_new(g_str_hash, g_str_equal);
	ac->config = global_config;
	ac->proto = proto;

	return ac;
}

void
hybrid_account_destroy(HybridAccount *account)
{
	if (account) {
		g_free(account->username);
		g_free(account->password);
		g_free(account);
	}
}

void 
hybrid_account_set_username(HybridAccount *account, const gchar *username)
{
	g_return_if_fail(account != NULL);

	g_free(account->username);

	account->username = g_strdup(username);
}

void 
hybrid_account_set_password(HybridAccount *account, const gchar *password)
{
	g_return_if_fail(account != NULL);

	g_free(account->password);

	account->password = g_strdup(password);
}

void
hybrid_account_error_reason(HybridAccount *account, const gchar *reason)
{
	/* TODO */
	g_return_if_fail(account != NULL);

	//hybrid_account_destroy(account);
}

void
hybrid_account_set_connection_status(HybridAccount *account,
		HybridConnectionStatusType status)
{
	if (account->connect_state != HYBRID_CONNECTION_CONNECTED &&
		status == HYBRID_CONNECTION_CONNECTED) {

		/* 
		 * It's necessary to set the status value here, because if
		 * connection status is not HYBRID_CONNECTION_CONNECTED, 
		 * hybrid_blist_add_buddy() just return without doing anything,
		 * which will cause load_blist_from_disk() fail to add buddies.
		 */
		account->connect_state = status;

		/* here we load the blist from the local cache file. */
		load_blist_from_disk(account);
	}

	account->connect_state = status;
}

static void
load_blist_from_disk(HybridAccount *account)
{
	HybridConfig *config;
	HybridBlistCache *cache;
	HybridGroup *group;
	HybridBuddy *buddy;
	xmlnode *root;
	xmlnode *node;
	xmlnode *group_node;
	xmlnode *buddy_node;
	xmlnode *account_node;
	gchar *id;
	gchar *name;
	gchar *value;
	guchar *icon_data;
	gsize icon_data_len;


	g_return_if_fail(account != NULL);

	config = account->config;
	cache = config->blist_cache;
	root = cache->root;

	if (!(node = xmlnode_find(root, "accounts"))) {
		hybrid_debug_error("account", 
			"can't find node named 'buddies' in blist.xml");
		return;
	}

	for (account_node = xmlnode_child(node); account_node;
			account_node = account_node->next) {
		if (g_strcmp0(account_node->name, "account")) {
			hybrid_debug_error("account", 
				"invalid blist.xml");
			return;
		}

		if (!xmlnode_has_prop(account_node, "proto") ||
			!xmlnode_has_prop(account_node, "username")) {
			hybrid_debug_error("account", 
				"invalid account node in blist.xml");
			continue;
		}
		name = xmlnode_prop(account_node, "username");
		value = xmlnode_prop(account_node, "proto");

		if (g_strcmp0(name, account->username) == 0 &&
			g_strcmp0(name, account->proto->info->name) == 0) {
			node = xmlnode_find(account_node, "buddies");
			goto account_found;
		}
	}

	return;
account_found:

	group_node = xmlnode_child(node);

	while (group_node) {

		if (!xmlnode_has_prop(group_node, "id") ||
			!xmlnode_has_prop(group_node, "name")) {

			hybrid_debug_error("account", "invalid group node");
			group_node = xmlnode_next(group_node);
			continue;
		}

		id = xmlnode_prop(group_node, "id");
		name = xmlnode_prop(group_node, "name");

		group = hybrid_blist_add_group(account, id, name);

		g_free(id);
		g_free(name);
		
		/*
		 * Iterate the buddy nodes, use the attribute values
		 * to create new HybridBuddys, and add them to the 
		 * buddy list and the GtkTreeView. 
		 */
		buddy_node = xmlnode_child(group_node);

		while (buddy_node) {

			if (!xmlnode_has_prop(buddy_node, "id") ||
				!xmlnode_has_prop(buddy_node, "name")) {

				hybrid_debug_error("account", "invalid buddy node");
				buddy_node = xmlnode_next(buddy_node);
				continue;
			}

			id = xmlnode_prop(buddy_node, "id");
			name = xmlnode_prop(buddy_node, "name");

			buddy = hybrid_blist_add_buddy(account, group, id, name);
			buddy->state = HYBRID_STATE_OFFLINE;

			//buddy_node = xmlnode_next(buddy_node);
			//continue;

			buddy->cache_node = buddy_node;

			g_free(id);
			g_free(name);

			if (xmlnode_has_prop(buddy_node, "mood")) {
				value = xmlnode_prop(buddy_node, "mood");
				hybrid_blist_set_buddy_mood(buddy, value);
				g_free(value);
			}

			/*
			 * If buddy node has attribute named 'icon', and the value ot the 
			 * attribute is not empty, then load the file pointed by 
			 * the icon value as the buddy's portrait, orelse load the 
			 * default icon, we DONT need to load the default icon file
			 * any more, let the hybrid_blist_set_buddy_icon() do it, just
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

			hybrid_blist_set_buddy_icon(buddy, icon_data,
					icon_data_len, value);

			g_free(value);
			g_free(icon_data);

			buddy_node = xmlnode_next(buddy_node);
		}

		group_node = xmlnode_next(group_node);
	}

}

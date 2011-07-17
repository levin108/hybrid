#include "xmpp_buddy.h"

static void
xmpp_buddy_subscribe(XmppStream *stream, const gchar *jid)
{
	xmlnode *node;
	gchar *xml_string;

	g_return_if_fail(stream != NULL);

	node = xmlnode_create("presence");

	xml_string = xmlnode_to_string(node);
	
	hybrid_debug_info("xmpp", "subscribe %s,send:\n%s",
			jid, xml_string);

	if (hybrid_ssl_write(stream->ssl, xml_string, strlen(xml_string)) == -1) {

	//	hybrid_account_error_reason(stream->account->account,
	//			"subscribe buddy failed");
		g_print("subscribe error\n");
		g_usleep(1000);

		g_free(xml_string);

		return;
	}

	g_free(xml_string);
}

void
xmpp_buddy_process_roster(XmppStream *stream, xmlnode *root)
{
	gchar *value;
	gchar *jid;
	gchar *name;
	gchar *group_name;
	xmlnode *node;
	xmlnode *group_node;
	xmlnode *item_nodes;
	HybridGroup *group;
	HybridAccount *account;

	g_return_if_fail(stream != NULL);
	g_return_if_fail(root != NULL);

	account = stream->account->account;

	hybrid_account_set_connection_status(account, 
					HYBRID_CONNECTION_CONNECTED);

	if (!xmlnode_has_prop(root, "type")) {
		goto roster_err;
	}

	value = xmlnode_prop(root, "type");
	if (g_strcmp0(value, "result") != 0) {
		goto roster_err;
	}
	g_free(value);

	if (!(node = xmlnode_find(root, "query"))) {
		goto roster_err;
	}

	item_nodes = xmlnode_child(node);

	for (node = item_nodes; node; node = node->next) {
		jid = xmlnode_prop(node, "jid");
		name = xmlnode_prop(node, "name");

		if (!(group_node = xmlnode_find(node, "group"))) {
			group_name = g_strdup(_("Buddies"));

		} else {
			group_name = xmlnode_content(group_node);
		}

		group = hybrid_blist_add_group(stream->account->account,
					group_name, group_name);

		hybrid_blist_add_buddy(stream->account->account,
					group, jid, name);

		g_free(group_name);

		g_free(jid);
		g_free(name);
	}

	xmpp_buddy_subscribe(stream, NULL);

	return;

roster_err:
	hybrid_account_error_reason(stream->account->account,
			_("request roster failed."));
}

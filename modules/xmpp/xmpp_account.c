#include "xmpp_account.h"
#include "xmpp_stream.h"
#include "xmpp_iq.h"

XmppAccount*
xmpp_account_create(HybridAccount *account, const gchar *username,
					const gchar *password, const gchar *to)
{
	XmppAccount *ac;

	g_return_val_if_fail(account != NULL, NULL);
	g_return_val_if_fail(username != NULL, NULL);
	g_return_val_if_fail(password != NULL, NULL);
	g_return_val_if_fail(to != NULL, NULL);

	ac = g_new0(XmppAccount, 1);

	ac->account = account;
	ac->username = g_strdup(username);
	ac->password = g_strdup(password);
	ac->to = g_strdup(to);

	return ac;
}

void
xmpp_account_destroy(XmppAccount *account)
{
	if (account) {
		g_free(account->username);
		g_free(account->password);
		g_free(account->to);

		g_free(account);
	}
}

gint
xmpp_account_modify_status(XmppStream *stream, const gchar *status)
{
	xmlnode *root;
	xmlnode *node;
	gchar *xml_string;

	root = xmlnode_create("presence");
	xmlnode_new_prop(root, "to", stream->jid);
	//xmlnode_new_prop(root, "to", stream->account->username);
#if 0
	//xmlnode_new_prop(root, "id", "presence_1");
	xmlnode_new_prop(root, "from", stream->jid);
	
	node = xmlnode_new_child(root, "show");
	node = xmlnode_new_child(root, "status");
	xmlnode_set_content(node, status);
#endif

	xml_string = xmlnode_to_string(root);

	xmlnode_free(root);

	hybrid_debug_info("xmpp", "modify status,send:\n%s", xml_string);

	if (hybrid_ssl_write(stream->ssl, xml_string, strlen(xml_string)) == -1) {

		hybrid_debug_error("xmpp", "modify status failed.");
		g_free(xml_string);

		return HYBRID_ERROR;
	}

	g_free(xml_string);

	return HYBRID_OK;
}

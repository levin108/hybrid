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
xmpp_account_modify_status(XmppStream *stream, gint state, const gchar *status)
{
	xmlnode *root;
	xmlnode *node;
	gchar *xml_string;
	HybridAccount *account;

	g_return_val_if_fail(stream != NULL, HYBRID_ERROR);

	account = stream->account->account;

	if (state == HYBRID_STATE_OFFLINE) {
		state = HYBRID_STATE_ONLINE;
	}

	root = xmlnode_create("presence");
	xmlnode_new_prop(root, "from", stream->jid);

	switch (state) {
		case HYBRID_STATE_AWAY:
			node = xmlnode_new_child(root, "show");
			xmlnode_set_content(node, "away");
			break;
		case HYBRID_STATE_BUSY:
			node = xmlnode_new_child(root, "show");
			xmlnode_set_content(node, "dnd");
			break;
		default:
			break;
	};

	if (status) {
		node = xmlnode_new_child(root, "status");
		xmlnode_set_content(node, status);
	}

	xml_string = xmlnode_to_string(root);
	xmlnode_free(root);

	hybrid_debug_info("xmpp", "modify status,send:\n%s", xml_string);

	if (hybrid_ssl_write(stream->ssl, xml_string, 
				strlen(xml_string)) == -1) {

		hybrid_debug_error("xmpp", "modify status failed.");
		g_free(xml_string);

		return HYBRID_ERROR;
	}

	g_free(xml_string);

	/* we store the status. */
	if (status && g_strcmp0(account->status_text, status)) {
		hybrid_account_set_status_text(account, status);
		hybrid_account_update(account);
	}

	/* set account's presence state. */
	hybrid_account_set_state(account, state);

	return HYBRID_OK;
}

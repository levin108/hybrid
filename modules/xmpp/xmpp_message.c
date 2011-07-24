#include "xmpp_message.h"

gint
xmpp_message_send(XmppStream *stream, const gchar *text, const gchar *to)
{
	xmlnode *root;
	xmlnode *node;
	gchar *xml_string;

	g_return_val_if_fail(stream != NULL, HYBRID_ERROR);
	g_return_val_if_fail(text != NULL, HYBRID_ERROR);
	g_return_val_if_fail(to != NULL, HYBRID_ERROR);

	root = xmlnode_create("message");
	xmlnode_new_prop(root, "from", stream->jid);
	xmlnode_new_prop(root, "to", to);
	xmlnode_new_prop(root, "type", "chat");
	node = xmlnode_new_child(root, "body");
	xmlnode_set_content(node, text);

	xml_string = xmlnode_to_string(root);
	xmlnode_free(root);

	hybrid_debug_info("xmpp", "send message to %s:\n%s", to, xml_string);

	if (hybrid_ssl_write(stream->ssl, xml_string, strlen(xml_string)) == -1) {

		hybrid_debug_error("xmpp", "send message to %s failed\n", to);
		g_free(xml_string);

		return HYBRID_ERROR;
	}

	g_free(xml_string);

	return HYBRID_OK;
}

gint
xmpp_message_send_typing(XmppStream *stream, const gchar *to, HybridInputState state)
{
	xmlnode *root;
	xmlnode *node;
	gchar *xml_string;

	g_return_val_if_fail(stream != NULL, HYBRID_ERROR);
	g_return_val_if_fail(to != NULL, HYBRID_ERROR);

	root = xmlnode_create("message");
	xmlnode_new_prop(root, "from", stream->jid);
	xmlnode_new_prop(root, "to", to);
	xmlnode_new_prop(root, "type", "chat");
	switch (state) {
		case INPUT_STATE_TYPING:
			node = xmlnode_new_child(root, "composing");
			break;
		case INPUT_STATE_ACTIVE:
			node = xmlnode_new_child(root, "active");
			break;
		case INPUT_STATE_PAUSED:
			node = xmlnode_new_child(root, "paused");
			break;
		default:
			node = NULL;
			break;
	}

	if (node) {
		xmlnode_new_namespace(node, NULL, NS_CHANGESTATES);
	}

	xml_string = xmlnode_to_string(root);
	xmlnode_free(root);

	hybrid_debug_info("xmpp", "send message to %s:\n%s", to, xml_string);

	if (hybrid_ssl_write(stream->ssl, xml_string, strlen(xml_string)) == -1) {

		hybrid_debug_error("xmpp", "send message to %s failed\n", to);
		g_free(xml_string);

		return HYBRID_ERROR;
	}

	g_free(xml_string);

	return HYBRID_OK;
}

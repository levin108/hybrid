#include "xmpp_iq.h"

static gint current_iq_id = 1;

IqRequest*
iq_request_create(XmppStream *stream, gint type)
{
	IqRequest *iq;
	gchar *id_str;

	iq = g_new0(IqRequest, 1);

	iq->type = type;
	iq->node = xmlnode_create("iq");
	iq->id = current_iq_id ++;
	iq->stream = stream;

	switch (type) {
		case IQ_TYPE_SET:
			xmlnode_new_prop(iq->node, "type", "set");
			break;
		case IQ_TYPE_GET:
			xmlnode_new_prop(iq->node, "type", "get");
			break;
		default:
			g_free(iq);
			hybrid_debug_error("iq", "unknown iq type.");
			return NULL;
	}

	id_str = g_strdup_printf("%d", iq->id);
	xmlnode_new_prop(iq->node, "id", id_str);
	g_free(id_str);

	return iq;
}

void
iq_request_set_callback(IqRequest *iq, trans_callback callback,
		gpointer user_data)
{
	g_return_if_fail(iq != NULL);

	if (!iq->trans) {
		iq->trans = iq_transaction_create(iq->id);
	}

	iq_transaction_set_callback(iq->trans, callback, user_data);
	iq_transaction_add(iq->stream, iq->trans);
}

gint
iq_request_send(IqRequest *iq)
{
	gchar *xml_string;

	g_return_val_if_fail(iq != NULL, HYBRID_ERROR);

	xml_string = xmlnode_to_string(iq->node);

	hybrid_debug_info("xmpp", "send iq request:\n%s", xml_string);

	if (hybrid_ssl_write(iq->stream->ssl,
				xml_string, strlen(xml_string)) == -1) {

		hybrid_debug_error("iq", "send iq request failed");

		g_free(xml_string);

		return HYBRID_ERROR;
	}

	g_free(xml_string);

	return HYBRID_OK;
}

void
iq_request_destroy(IqRequest *iq)
{
	if (iq) {
		xmlnode_free(iq->node);
		g_free(iq);
	}
}

IqTransaction*
iq_transaction_create(gint iq_id)
{
	IqTransaction *trans;

	trans = g_new0(IqTransaction, 1);

	trans->iq_id = iq_id;

	return trans;
}

void
iq_transaction_set_callback(IqTransaction *trans, trans_callback callback,
						gpointer user_data)
{
	g_return_if_fail(trans != NULL);

	trans->callback = callback;
	trans->user_data = user_data;
}

void
iq_transaction_add(XmppStream *stream, IqTransaction *trans)
{
	g_return_if_fail(stream != NULL);

	stream->pending_trans = g_slist_append(stream->pending_trans, trans);
}

void
iq_transaction_remove(XmppStream *stream, IqTransaction *trans)
{
	g_return_if_fail(stream != NULL);

	stream->pending_trans = g_slist_remove(stream->pending_trans, trans);
}

void
iq_transaction_destroy(IqTransaction *trans)
{
	g_free(trans);
}

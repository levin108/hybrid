#include "xmpp_buddy.h"
#include "xmpp_iq.h"

static GHashTable *xmpp_buddies = NULL;

/**
 * Scribe the presence information of the roster by
 * sending a <presence/> label to the server.
 */
static void
xmpp_buddy_presence(XmppStream *stream)
{
	xmlnode *node;
	gchar *xml_string;
	HybridAccount *account;

	g_return_if_fail(stream != NULL);

	account = stream->account->account;

	node = xmlnode_create("presence");

	xml_string = xmlnode_to_string(node);
	
	hybrid_debug_info("xmpp", "subscribe presence,send:\n%s", xml_string);

	if (hybrid_ssl_write(stream->ssl, xml_string, strlen(xml_string)) == -1) {

		hybrid_account_error_reason(stream->account->account,
				"subscribe presence failed");
		g_free(xml_string);

		return;
	}

	g_free(xml_string);

	xmpp_account_modify_status(stream, account->state, 
			account->status_text);
}

void
xmpp_buddy_process_roster(XmppStream *stream, xmlnode *root)
{
	gchar *value;
	gchar *jid, *name, *scribe;
	gchar *group_name;
	xmlnode *node;
	xmlnode *group_node;
	xmlnode *item_nodes;
	HybridGroup *group;
	HybridAccount *account;
	HybridBuddy *hd;
	XmppBuddy *buddy;
	gchar *pos;

	g_return_if_fail(stream != NULL);
	g_return_if_fail(root != NULL);

	account = stream->account->account;

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

		/*
		 * If the item has no property named 'name', then we make the
		 * prefix name of the jid as the buddy's name as default.
		 */
		if (xmlnode_has_prop(node, "name")) {
			name   = xmlnode_prop(node, "name");

		} else {
			for (pos = jid; *pos && *pos != '@'; pos ++);
			name = g_strndup(jid, pos - jid);
		}
		scribe = xmlnode_prop(node, "subscription");

		/*
		 * If the presence node doesn't has a group node,
		 * so we make it belongs to default group 'Buddies'.
		 */
		if (!(group_node = xmlnode_find(node, "group"))) {
			group_name = g_strdup(_("Buddies"));

		} else {
			group_name = xmlnode_content(group_node);
		}

		/* add this buddy's group to the buddy list. */
		group = hybrid_blist_add_group(account, group_name, group_name);
		g_free(group_name);

		/* add this buddy to the buddy list. */
		hd = hybrid_blist_add_buddy(account, group, jid, name);
		g_free(jid);

		/*
		 * Maybe the buddy with the specified jabber id already exists,
		 * then hybrid_blist_add_buddy() will not set the buddy name even
		 * if the name has changed, so we must set the name manually by
		 * hybrid_blist_set_buddy_name() in case that the name changed.
		 */
		hybrid_blist_set_buddy_name(hd, name);
		g_free(name);

		buddy = xmpp_buddy_create(stream, hd);
		xmpp_buddy_set_subscription(buddy, scribe);
	}

	/* subsribe the presence of the roster. */
	xmpp_buddy_presence(stream);

	return;

roster_err:
	hybrid_account_error_reason(stream->account->account,
			_("request roster failed."));
}

XmppBuddy*
xmpp_buddy_create(XmppStream *stream, HybridBuddy *hybrid_buddy)
{
	XmppBuddy *buddy;
	gchar *pos;

	g_return_val_if_fail(hybrid_buddy != NULL, NULL);

	if (!xmpp_buddies) {
		xmpp_buddies = g_hash_table_new(g_str_hash, g_str_equal);
	}

	if ((buddy = g_hash_table_lookup(xmpp_buddies, hybrid_buddy->id))) {
		return buddy;
	}

	for (pos = hybrid_buddy->id; *pos; pos ++) {
		*pos = g_ascii_tolower(*pos);
	}

	buddy = g_new0(XmppBuddy, 1);
	buddy->jid = g_strdup(hybrid_buddy->id);
	buddy->buddy = hybrid_buddy;
	buddy->stream = stream;

	xmpp_buddy_set_name(buddy, hybrid_buddy->name);
	xmpp_buddy_set_group_name(buddy, hybrid_buddy->parent->name);
	xmpp_buddy_set_photo(buddy, hybrid_blist_get_buddy_checksum(hybrid_buddy));


	g_hash_table_insert(xmpp_buddies, buddy->jid, buddy);

	return buddy;
}

void
xmpp_buddy_set_name(XmppBuddy *buddy, const gchar *name)
{
	gchar *tmp;

	g_return_if_fail(buddy != NULL);

	tmp = buddy->name;
	buddy->name = g_strdup(name);
	g_free(tmp);
}

void
xmpp_buddy_set_subscription(XmppBuddy *buddy, const gchar *sub)
{
	gchar *tmp;

	g_return_if_fail(buddy != NULL);

	tmp = buddy->subscription;
	buddy->subscription = g_strdup(sub);
	g_free(tmp);
}

void
xmpp_buddy_set_resource(XmppBuddy *buddy, const gchar *resource)
{
	gchar *tmp;

	g_return_if_fail(buddy != NULL);

	tmp = buddy->resource;
	buddy->resource = g_strdup(resource);
	g_free(tmp);
}

void
xmpp_buddy_set_photo(XmppBuddy *buddy, const gchar *photo)
{
	gchar *tmp;

	g_return_if_fail(buddy != NULL);

	tmp = buddy->photo;
	buddy->photo = g_strdup(photo);
	g_free(tmp);
}

void
xmpp_buddy_set_status(XmppBuddy *buddy, const gchar *status)
{
	gchar *tmp;

	g_return_if_fail(buddy != NULL);
	
	tmp = buddy->status;
	buddy->status = g_strdup(status);
	g_free(tmp);

	hybrid_blist_set_buddy_mood(buddy->buddy, status);
}

void
xmpp_buddy_set_show(XmppBuddy *buddy, const gchar *show)
{
	gint state = 0;

	g_return_if_fail(buddy != NULL);
	g_return_if_fail(show != NULL);

	if (g_ascii_strcasecmp(show, "away") == 0) {
		state = HYBRID_STATE_AWAY;

	} else if (g_ascii_strcasecmp(show, "avaiable") == 0) {
		g_print("avaiable\n");
		state = HYBRID_STATE_ONLINE;

	} else if (g_ascii_strcasecmp(show, "dnd") == 0) {
		state = HYBRID_STATE_BUSY;

	} else if (g_ascii_strcasecmp(show, "unavailable") == 0) {
		state = HYBRID_STATE_OFFLINE;
	}

	hybrid_blist_set_buddy_state(buddy->buddy, state);
}

void
xmpp_buddy_set_group_name(XmppBuddy *buddy, const gchar *group)
{
	gchar *tmp;

	g_return_if_fail(buddy != NULL);

	tmp = buddy->group;
	buddy->group = g_strdup(group);
	g_free(tmp);
}

gint
xmpp_buddy_set_group(XmppBuddy *buddy, const gchar *group)
{
	IqRequest *iq;
	xmlnode *node;

	g_return_val_if_fail(buddy != NULL, HYBRID_ERROR);
	g_return_val_if_fail(group != NULL, HYBRID_ERROR);

	iq = iq_request_create(buddy->stream, IQ_TYPE_SET);

	node = xmlnode_new_child(iq->node, "query");
	xmlnode_new_namespace(node, NULL, ROSTER_NAMESPACE);
	node = xmlnode_new_child(node, "item");
	xmlnode_new_prop(node, "jid", buddy->jid);
	node = xmlnode_new_child(node, "group");
	xmlnode_set_content(node, group);

	if (iq_request_send(iq) != HYBRID_OK) {

		iq_request_destroy(iq);
		return HYBRID_ERROR;
	}

	iq_request_destroy(iq);

	return HYBRID_OK;
}

gint
xmpp_buddy_alias(XmppBuddy *buddy, const gchar *alias)
{
	IqRequest *iq;
	xmlnode *node;

	g_return_val_if_fail(buddy != NULL, HYBRID_ERROR);
	g_return_val_if_fail(alias != NULL, HYBRID_ERROR);

	iq = iq_request_create(buddy->stream, IQ_TYPE_SET);

	node = xmlnode_new_child(iq->node, "query");
	xmlnode_new_namespace(node, NULL, ROSTER_NAMESPACE);
	node = xmlnode_new_child(node, "item");
	xmlnode_new_prop(node, "jid", buddy->jid);
	xmlnode_new_prop(node, "name", alias);

	if (iq_request_send(iq) != HYBRID_OK) {

		iq_request_destroy(iq);
		return HYBRID_ERROR;
	}

	iq_request_destroy(iq);

	return HYBRID_OK;
}

gint
xmpp_buddy_get_info(XmppStream *stream, const gchar *jid,
				trans_callback callback, gpointer user_data)
{
	IqRequest *iq;
	xmlnode *node;

	g_return_val_if_fail(stream != NULL, HYBRID_ERROR);
	g_return_val_if_fail(jid != NULL, HYBRID_ERROR);

	iq = iq_request_create(stream, IQ_TYPE_GET);

	xmlnode_new_prop(iq->node, "to", jid);
	node = xmlnode_new_child(iq->node, "vCard");
	xmlnode_new_namespace(node, NULL, "vcard-temp");

	iq_request_set_callback(iq, callback, user_data);

	if (iq_request_send(iq) != HYBRID_OK) {

		hybrid_debug_error("xmpp", "get buddy info failed");
		iq_request_destroy(iq);

		return HYBRID_ERROR;
	}

	iq_request_destroy(iq);

	return HYBRID_OK;
}

XmppBuddy*
xmpp_buddy_find(const gchar *jid)
{
	XmppBuddy *buddy;
	gchar *tmp, *pos;

	g_return_val_if_fail(jid != NULL, NULL);

	if (!xmpp_buddies) {
		return NULL;
	}

	tmp = g_strdup(jid);

	for (pos = tmp; *pos; pos ++) {
		*pos = g_ascii_tolower(*pos);
	}

	buddy = g_hash_table_lookup(xmpp_buddies, tmp);
	g_free(tmp);

	return buddy;
}

void
xmpp_buddy_destroy(XmppBuddy *buddy)
{
	if (buddy) {
		g_hash_table_remove(xmpp_buddies, buddy);

		if (g_hash_table_size(xmpp_buddies) == 0) {
			g_hash_table_destroy(xmpp_buddies);
			xmpp_buddies = NULL;
		}

		g_free(buddy->jid);
		g_free(buddy->name);
		g_free(buddy->status);
		g_free(buddy->group);

		g_free(buddy);
	}
}

void
xmpp_buddy_clear(void)
{
	GHashTableIter hash_iter;
	gpointer key;
	XmppBuddy *buddy;

	g_hash_table_iter_init(&hash_iter, xmpp_buddies);

	while (g_hash_table_iter_next(&hash_iter, &key, (gpointer*)&buddy)) {
		xmpp_buddy_destroy(buddy);
	}
}

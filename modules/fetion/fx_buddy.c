#include "blist.h"
#include "xmlnode.h"
#include "eventloop.h"
#include "util.h"

#include "fetion.h"
#include "fx_account.h"
#include "fx_buddy.h"

static gchar *generate_subscribe_body(void);
static gchar *generate_get_info_body(const gchar *userid);
static gchar *generate_buddy_move_body(const gchar *userid,
				const gchar *groupid);

fetion_buddy*
fetion_buddy_create(void)
{
	fetion_buddy *buddy;

	buddy = g_new0(fetion_buddy, 1);

	return buddy;
}

gint
fetion_buddy_scribe(fetion_account *ac)
{
	gchar *res, *body;
	fetion_sip *sip;
	sip_header *eheader;

	sip = ac->sip;
	fetion_sip_set_type(sip, SIP_SUBSCRIPTION);

	eheader = sip_event_header_create(SIP_EVENT_PRESENCE);
	fetion_sip_add_header(sip, eheader);

	body = generate_subscribe_body();

	res = fetion_sip_to_string(sip, body);

	g_free(body);

	hybrid_debug_info("fetion", "send:\n%s", res);

	if (send(ac->sk, res, strlen(res), 0) == -1) { 
		g_free(res);

		return HYBRID_ERROR;
	}

	g_free(res);

	return HYBRID_OK;
}

gint
fetion_buddy_get_info(fetion_account *ac, const gchar *userid,
		TransCallback callback, gpointer data)
{
	fetion_transaction *trans;
	fetion_sip *sip;
	sip_header *eheader;
	gchar *body;
	gchar *res;

	g_return_val_if_fail(ac != NULL, HYBRID_ERROR);
	g_return_val_if_fail(userid != NULL, HYBRID_ERROR);

	sip = ac->sip;

	fetion_sip_set_type(sip, SIP_SERVICE);
	eheader = sip_event_header_create(SIP_EVENT_GETCONTACTINFO);

	trans = transaction_create();
	transaction_set_callid(trans, sip->callid);
	transaction_set_userid(trans, userid);
	transaction_set_callback(trans, callback);
	transaction_set_data(trans, data);
	transaction_add(ac, trans);

	fetion_sip_add_header(sip, eheader);
	body = generate_get_info_body(userid);
	res = fetion_sip_to_string(sip, body);
	g_free(body);

	hybrid_debug_info("fetion", "send:\n%s", res);

	if (send(ac->sk, res, strlen(res), 0) == -1) {
		g_free(res);
		return HYBRID_ERROR;
	}

	g_free(res);

	return HYBRID_OK;
}

gint
fetion_buddy_move_to(fetion_account *ac, const gchar *userid,
		const gchar *groupid)
{
	fetion_sip *sip;
	sip_header *eheader;
	gchar *res, *body;

	g_return_val_if_fail(ac != NULL, HYBRID_ERROR);
	g_return_val_if_fail(userid != NULL, HYBRID_ERROR);
	g_return_val_if_fail(groupid != NULL, HYBRID_ERROR);

	sip = ac->sip;

	fetion_sip_set_type(sip, SIP_SERVICE);

	eheader = sip_event_header_create(SIP_EVENT_SETCONTACTINFO);

	fetion_sip_add_header(sip , eheader);

	body = generate_buddy_move_body(userid, groupid);

	res = fetion_sip_to_string(sip , body);
	g_free(body);

	hybrid_debug_info("fetion", "%s moved to group %s, send:\n%s",
					userid, groupid, res);

	if (send(ac->sk, res, strlen(res), 0) == -1) {
		return HYBRID_ERROR;
	}

	g_free(res);

	return HYBRID_OK;
}

fetion_buddy*
fetion_buddy_parse_info(fetion_account *ac, 
		const gchar *userid, const gchar *sipmsg)
{
	xmlnode *root;
	xmlnode *node;
	gchar *pos;
	gchar *temp;
	gchar *value;
	fetion_buddy *buddy;
	gint code;

	code = fetion_sip_get_code(sipmsg);

	if (code != 200) {
		hybrid_debug_error("fetion", "get information with code:%d", code);
		return NULL;
	}

	if (!(pos = strstr(sipmsg, "\r\n\r\n"))) {
		goto get_info_error;
	}

	pos += 4;

	if (!(root = xmlnode_root(pos, strlen(pos)))) {
		goto get_info_error;
	}

	if (!(node = xmlnode_find(root, "contact"))) {
		xmlnode_free(root);
		goto get_info_error;
	}

	if (!(buddy = fetion_buddy_find_by_userid(ac, userid))) {
		xmlnode_free(root);
		goto get_info_error;
	}

	if (xmlnode_has_prop(node, "sid")) {
		value = xmlnode_prop(node, "sid");
		g_free(buddy->sid);
		buddy->sid = g_strdup(value);
		g_free(value);
	}

	if (xmlnode_has_prop(node, "mobile-no")) {
		value = xmlnode_prop(node, "mobile-no");
		g_free(buddy->mobileno);
		buddy->mobileno = g_strdup(value);
		g_free(value);
	}

	if (xmlnode_has_prop(node, "impresa")) {
		value = xmlnode_prop(node, "impresa");
		g_free(buddy->mood_phrase);
		buddy->mood_phrase = g_strdup(value);
		g_free(value);
	}

	if (xmlnode_has_prop(node, "nickname")) {
		value = xmlnode_prop(node, "nickname");
		g_free(buddy->nickname);
		buddy->nickname = g_strdup(value);
		g_free(value);
	}

	if (xmlnode_has_prop(node, "gender")) {
		value = xmlnode_prop(node, "gender");
		buddy->gender = atoi(value);
		g_free(value);
	}

	if (xmlnode_has_prop(node, "carrier-region")) {
		value = xmlnode_prop(node, "carrier-region");

		for (pos = value; *pos && *pos != '.'; pos ++);
		g_free(buddy->country);
		buddy->country = g_strndup(value, pos - value);

		for (pos ++, temp = pos; *pos && *pos != '.'; pos ++);
		g_free(buddy->province);
		buddy->province = g_strndup(temp, pos - temp);

		for (pos ++, temp = pos; *pos && *pos != '.'; pos ++);
		g_free(buddy->city);
		buddy->city = g_strndup(temp, pos - temp);

	}

	xmlnode_free(node);

	return buddy;

get_info_error:
	hybrid_debug_error("fetion", "invalid get-info response");
	return NULL;
}

fetion_buddy*
fetion_buddy_find_by_userid(fetion_account *ac, const gchar *userid)
{
	GSList *pos;
	fetion_buddy *buddy;

	for (pos = ac->buddies; pos; pos = pos->next) {
		buddy = (fetion_buddy*)pos->data;

		if (g_strcmp0(buddy->userid, userid) == 0) {
			return buddy;
		}
	}

	return 0;
}

static gchar*
generate_subscribe_body(void)
{
	xmlnode *root;
	xmlnode *node;
	gchar xml_raw[] = "<args></args>";

	root = xmlnode_root(xml_raw, strlen(xml_raw));

	node = xmlnode_new_child(root, "subscription");
	xmlnode_new_prop(node, "self", "v4default;mail-count");
	xmlnode_new_prop(node, "buddy", "v4default");
	xmlnode_new_prop(node, "version", "0");

	return xmlnode_to_string(root);
}

void
fetion_buddy_destroy(fetion_buddy *buddy)
{
	if (buddy) {
		g_free(buddy->userid);
		g_free(buddy->sipuri);
		g_free(buddy->sid);
		g_free(buddy->mobileno);
		g_free(buddy->mood_phrase);
		g_free(buddy->carrier);
		g_free(buddy->localname);
		g_free(buddy->groups);
		g_free(buddy->portrait_crc);
		g_free(buddy->country);
		g_free(buddy->province);
		g_free(buddy->city);
		g_free(buddy);
	}
}

void
fetion_buddies_init(fetion_account *ac)
{
	GSList *pos;
	gchar *start, *stop, *id;
	fetion_buddy *buddy;
	HybridGroup *imgroup;
	HybridBuddy *imbuddy;

	for (pos = ac->buddies; pos; pos = pos->next) {
		buddy = (fetion_buddy*)pos->data;

		start = buddy->groups;
		stop = buddy->groups;

		while (*stop) {
			for (; *stop && *stop != ';'; stop ++);
			id = g_strndup(start, stop - start);

			imgroup = hybrid_blist_find_group(ac->account, id);

			imbuddy = hybrid_blist_add_buddy(ac->account, imgroup,
					buddy->userid, buddy->localname);

			if (*(imbuddy->name) == '\0') {
				hybrid_blist_set_buddy_name(imbuddy, buddy->sid);
			}

			g_free(id);

			if (*stop) {
				stop ++;
				start = stop;

			} else {
				break;
			}
		}
	}
}

/**
 * Callback function to handle the portrait receive event.
 */
static gboolean
portrait_recv_cb(gint sk, gpointer user_data)
{
	gchar buf[BUF_LENGTH];
	gint n;
	gchar *pos;
	HybridBuddy *imbuddy;
	portrait_trans *trans = (portrait_trans*)user_data;

	if ((n = recv(sk, buf, sizeof(buf), 0)) == -1) {
		hybrid_debug_error("fetion", "get portrait for \'%s\':%s",
				trans->buddy ? trans->buddy->sid : trans->ac->sid, 
				strerror(errno));
		return FALSE;
	}

	buf[n] = '\0';

	if (n == 0) {

		if (trans->portrait_type == PORTRAIT_TYPE_BUDDY) {
			imbuddy = hybrid_blist_find_buddy(trans->ac->account,
					trans->buddy->userid);
		}

		if (hybrid_get_http_code(trans->data) != 200) {
			/* 
			 * Note that we got no portrait, but we still need
			 * to set buddy icon, just for the portrait checksum, we 
			 * set it default to "0" instead of leaving it NULL,
			 * so that in the next login, we just check the changes
			 * of the buddy's checksum to determine whether to fetch a 
			 * portrait from the server. 
			 */
			if (trans->portrait_type == PORTRAIT_TYPE_BUDDY) {
				hybrid_blist_set_buddy_icon(imbuddy, NULL, 0,
						trans->buddy->portrait_crc);

			} else {
				printf("####################################3333\n");
				hybrid_account_set_icon(trans->ac->account, NULL,
						0, trans->ac->portrait_crc);
			}

			goto pt_fin;
		}

		trans->data_len = hybrid_get_http_length(trans->data);

		if (!(pos = strstr(trans->data, "\r\n\r\n"))) {
			goto pt_fin;
		}

		pos += 4;

		if (trans->portrait_type == PORTRAIT_TYPE_BUDDY) { /**< buddy portrait */
			hybrid_blist_set_buddy_icon(imbuddy, (guchar*)pos,
					trans->data_len, trans->buddy->portrait_crc);

		} else {
			hybrid_account_set_icon(trans->ac->account, (guchar*)pos,
					trans->data_len, trans->ac->portrait_crc);
		}

		goto pt_fin;

	} else {
		trans->data = realloc(trans->data, trans->data_size + n);
		memcpy(trans->data + trans->data_size, buf, n);
		trans->data_size += n;
	}

	return TRUE;

pt_fin:
	g_free(trans->data);
	g_free(trans);

	return FALSE;
}

gboolean
portrait_conn_cb(gint sk, gpointer user_data)
{
	portrait_data *data = (portrait_data*)user_data;
	portrait_trans *trans;
	gchar *http_string;
	gchar *encoded_sipuri;
	gchar *encoded_ssic;
	gchar *uri;

	trans = g_new0(portrait_trans, 1);
	trans->buddy = data->buddy;
	trans->ac = data->ac;
	trans->portrait_type = data->portrait_type;

	g_free(data);

	if (trans->portrait_type == PORTRAIT_TYPE_BUDDY) {
		encoded_sipuri = g_uri_escape_string(trans->buddy->sipuri, NULL, TRUE);

	} else {
		encoded_sipuri = g_uri_escape_string(trans->ac->sipuri, NULL, TRUE);
	}

	encoded_ssic = g_uri_escape_string(trans->ac->ssic, NULL, TRUE);

	uri = g_strdup_printf("/%s/getportrait.aspx", trans->ac->portrait_host_path);

	http_string = g_strdup_printf("GET %s?Uri=%s"
			  "&Size=120&c=%s HTTP/1.1\r\n"
			  "User-Agent: IIC2.0/PC "PROTO_VERSION"\r\n"
			  "Accept: image/pjpeg;image/jpeg;image/bmp;"
			  "image/x-windows-bmp;image/png;image/gif\r\n"
			  "Host: %s\r\nConnection: Close\r\n\r\n",
			  uri, encoded_sipuri, encoded_ssic,
			  trans->ac->portrait_host_name);

	g_free(encoded_sipuri);
	g_free(encoded_ssic);
	g_free(uri);

	if (send(sk, http_string, strlen(http_string), 0) == -1) {
		hybrid_debug_error("fetion", "download portrait for \'%s\':%s",
				trans->buddy->sid, strerror(errno));
		g_free(http_string);
		return FALSE;
	}

	g_free(http_string);

	hybrid_event_add(sk, HYBRID_EVENT_READ, portrait_recv_cb, trans);

	return FALSE;
}

void
fetion_update_portrait(fetion_account *ac, fetion_buddy *buddy)
{
	portrait_data *data;
	HybridBuddy *hybrid_buddy;
	const gchar *checksum;

	g_return_if_fail(ac != NULL);
	g_return_if_fail(buddy != NULL);

	data = g_new0(portrait_data, 1);
	data->buddy = buddy;
	data->ac = ac;
	data->portrait_type = PORTRAIT_TYPE_BUDDY;

	if (!(hybrid_buddy = hybrid_blist_find_buddy(ac->account, buddy->userid))) {
		hybrid_debug_error("fetion", "FATAL, update portrait,"
				" unable to find a buddy.");
		return;
	}

	checksum = hybrid_blist_get_buddy_checksum(hybrid_buddy);

	if (checksum != NULL && g_strcmp0(checksum, buddy->portrait_crc) == 0) {
		hybrid_debug_info("fetion", "portrait for %s(%s) up to date",
		buddy->nickname && *(buddy->nickname) != '\0' ? buddy->nickname : buddy->userid,
		buddy->portrait_crc);
		return;
	}

	hybrid_proxy_connect(ac->portrait_host_name, 80, portrait_conn_cb, data);
}


static gchar*
generate_get_info_body(const gchar *userid)
{
	xmlnode *root;
	xmlnode *node;

	gchar body[] = "<args></args>";

	root = xmlnode_root(body, strlen(body));
	node = xmlnode_new_child(root, "contact");
	xmlnode_new_prop(node, "user-id", userid);

	return xmlnode_to_string(root);
}

static gchar*
generate_buddy_move_body(const gchar *userid, const gchar *groupid)
{
	const gchar *body;
	xmlnode *root;
	xmlnode *node;
	
	body = "<args></args>";

	root = xmlnode_root(body, strlen(body));
	
	node = xmlnode_new_child(root, "contacts");
	node = xmlnode_new_child(node, "contact");

	xmlnode_new_prop(node, "user-id", userid);
	xmlnode_new_prop(node, "buddy-lists", groupid);

	return xmlnode_to_string(root);
}

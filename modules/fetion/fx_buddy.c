#include "blist.h"
#include "xmlnode.h"
#include "eventloop.h"
#include "util.h"

#include "fetion.h"
#include "fx_account.h"
#include "fx_buddy.h"

static gchar *generate_subscribe_body(void);

fetion_buddy*
fetion_buddy_create(void)
{
	fetion_buddy *buddy;

	buddy = g_new0(fetion_buddy, 1);

	return buddy;
}

gint
fetion_buddy_scribe(gint sk, fetion_account *ac)
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

	hybird_debug_info("fetion", "send:\n%s", res);

	if (send(sk, res, strlen(res), 0) == -1) { 
		g_free(res);

		return HYBIRD_ERROR;
	}

	g_free(res);

	return HYBIRD_OK;
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
		g_free(buddy);
	}
}

void
fetion_buddies_init(fetion_account *ac)
{
	GSList *pos;
	gchar *start, *stop, *id;
	fetion_buddy *buddy;
	HybirdGroup *imgroup;
	HybirdBuddy *imbuddy;

	for (pos = ac->buddies; pos; pos = pos->next) {
		buddy = (fetion_buddy*)pos->data;

		start = buddy->groups;
		stop = buddy->groups;

		while (*stop) {
			for (; *stop && *stop != ';'; stop ++);
			id = g_strndup(start, stop - start);

			imgroup = hybird_blist_find_group(ac->account, id);

			imbuddy = hybird_blist_add_buddy(ac->account, imgroup,
					buddy->userid, buddy->localname);

			if (*(imbuddy->name) == '\0') {
				hybird_blist_set_buddy_name(imbuddy, buddy->sid);
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
	HybirdBuddy *imbuddy;
	portrait_trans *trans = (portrait_trans*)user_data;

	if ((n = recv(sk, buf, sizeof(buf), 0)) == -1) {
		hybird_debug_error("fetion", "get portrait for \'%s\':%s",
				trans->buddy->sid, strerror(errno));
		return FALSE;
	}

	buf[n] = '\0';

	if (n == 0) {
		imbuddy = hybird_blist_find_buddy(trans->ac->account,
				trans->buddy->userid);

		if (hybird_get_http_code(trans->data) != 200) {
			/* 
			 * Note that we got no portrait, but we still need
			 * to set buddy icon, just for the portrait checksum, we 
			 * set it default to "0" instead of leaving it NULL,
			 * so that in the next login, we just check the changes
			 * of the buddy's checksum to determine whether to fetch a 
			 * portrait from the server. 
			 */
			hybird_blist_set_buddy_icon(imbuddy, NULL, 0,
					trans->buddy->portrait_crc);
			goto pt_fin;
		}

		trans->data_len = hybird_get_http_length(trans->data);

		if (!(pos = strstr(trans->data, "\r\n\r\n"))) {
			goto pt_fin;
		}

		pos += 4;

		hybird_blist_set_buddy_icon(imbuddy, (guchar*)pos,
				trans->data_len, trans->buddy->portrait_crc);

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

/**
 * Callback function to handle the portriat connection event. 
 * It start a http GET request, the full string is like:
 *
 * GET /HDS_S00/getportrait.aspx?Uri=sip%3A642781687%40fetion.com.cn
 * %3Bp%3D6543&Size=120&c=CBIOAACvnz7yxTih9INomOTTPvbcRQgbv%2BWSdvSlGM5711%2BZL9
 * GUQL4ohBdnDsU1uq0IU9piw2w8wRHIztMX6JmkQzB%2B0nEcndn0kYSKtSDg3JDaj3s%2BZePi9SU
 * HO9BIHaZ5vOsAAA%3D%3D HTTP/1.1
 * User-Agent: IIC2.0/PC 4.0.2510
 * Accept: image/pjpeg;image/jpeg;image/bmp;image/x-windows-bmp;image/png;image/gif
 * Host: hdss1fta.fetion.com.cn
 * Connection: Keep-Alive
 */
static gboolean
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

	g_free(data);

	encoded_sipuri = g_uri_escape_string(trans->buddy->sipuri, NULL, TRUE);
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

	//hybird_debug_info("fetion", "send:\n%s", http_string);

	if (send(sk, http_string, strlen(http_string), 0) == -1) {
		hybird_debug_error("fetion", "download portrait for \'%s\':%s",
				trans->buddy->sid, strerror(errno));
		g_free(http_string);
		return FALSE;
	}

	g_free(http_string);

	hybird_event_add(sk, HYBIRD_EVENT_READ, portrait_recv_cb, trans);

	return FALSE;
}

void
fetion_update_portrait(fetion_account *ac, fetion_buddy *buddy)
{
	portrait_data *data;
	HybirdBuddy *hybird_buddy;
	const gchar *checksum;

	g_return_if_fail(ac != NULL);
	g_return_if_fail(buddy != NULL);

	data = g_new0(portrait_data, 1);
	data->buddy = buddy;
	data->ac = ac;

	if (!(hybird_buddy = hybird_blist_find_buddy(ac->account, buddy->userid))) {
		hybird_debug_error("fetion", "FATAL, update portrait,"
				" unable to find a buddy.");
		return;
	}

	checksum = hybird_blist_get_buddy_checksum(hybird_buddy);

	if (checksum != NULL && g_strcmp0(checksum, buddy->portrait_crc) == 0) {
		hybird_debug_info("fetion", "portrait for %s(%s) up to date",
				checksum, buddy->portrait_crc);
		return;
	}

	hybird_proxy_connect(ac->portrait_host_name, 80, portrait_conn_cb, data);
}

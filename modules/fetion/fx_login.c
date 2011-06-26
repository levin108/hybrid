#include <glib.h>
#include "util.h"
#include "connect.h"
#include "eventloop.h"
#include "xmlnode.h"

#include "fetion.h"
#include "fx_trans.h"
#include "fx_login.h"
#include "fx_account.h"
#include "fx_group.h"
#include "fx_buddy.h"
#include "fx_config.h"

static gchar *hash_password_v1(const guchar *b0, gint b0len,
		const guchar *password,	gint psdlen);
static gchar *hash_password_v2(const gchar *userid, const gchar *passwordhex);
static gchar *hash_password_v4(const gchar *userid, const gchar *password);
static gchar *generate_cnouce();
static guchar *strtohex(const gchar *in, gint *len);
static gchar *hextostr(const guchar *in, gint len);
static gchar *generate_configuration_body(fetion_account *ac);
static gint parse_configuration(fetion_account *ac, const gchar *cfg);
static gboolean sipc_reg_action(gint sk, gpointer user_data);
static gint parse_sipc_reg_response(const gchar *reg_response, gchar **nonce,
		gchar **key);
static gchar *generate_aes_key();
static gchar *generate_response(const gchar *nonce_raw, const gchar *userid,
		const gchar *password, const gchar *publickey, const gchar *aeskey_raw);
static gint sipc_aut_action(gint sk, fetion_account *ac, const gchar *response);
static gchar *generate_auth_body(fetion_account *ac);
static void parse_sipc_resp(fetion_account *ac, const gchar *pos, gint len);
/**
 * Parse the ssi authentication response string. then we can
 * get the following information: sipuri/mobileno/sid/ssic.
 */
static gint
parse_ssi_response(fetion_account *ac, const gchar *response)
{
	gchar *prop;
	xmlnode *node;
	xmlnode *root = xmlnode_root(response, strlen(response));

	if (!root) {
		goto ssi_term;
	}

	prop = xmlnode_prop(root, "status-code"); 
	if (g_strcmp0(prop, "200") != 0) {
		g_free(prop);
		goto ssi_term;
	}

	g_free(prop);

	node = xmlnode_find(root, "user");

	prop = xmlnode_prop(node, "uri");
	fetion_account_set_sipuri(ac, prop);
	g_free(prop);

	if (!ac->sid || *(ac->sid) == '\0') {
		g_free(ac->sid);
		ac->sid = get_sid_from_sipuri(ac->sipuri);
		fetion_sip_set_from(ac->sip, ac->sid);
	}

	prop = xmlnode_prop(node, "mobile-no");
	fetion_account_set_mobileno(ac, prop);
	g_free(prop);

	prop = xmlnode_prop(node, "user-id");
	fetion_account_set_userid(ac, prop);
	g_free(prop);

#if 0
	node = xmlnode_find(root, "credential");
	prop = xmlnode_prop(node, "c");
	fetion_account_set_ssic(ac, prop);
	g_free(prop);
#endif

	return HYBRID_OK;
ssi_term:
	hybrid_account_error_reason(ac->account, _("ssi authencation"));
	xmlnode_free(root);
	return HYBRID_ERROR;
}

/**
 * Callback function to handle the cfg read event.
 */
static gboolean
cfg_read_cb(gint sk, gpointer user_data)
{
	gchar buf[BUF_LENGTH];
	gint n;
	gint length = 0;
	gchar *pos;
	fetion_account *ac = (fetion_account*)user_data;

	if ((n = recv(sk, buf, sizeof(buf), 0)) == -1) {
		hybrid_account_error_reason(ac->account, _("read cfg failed"));
		return FALSE;
	}

	buf[n] = '\0';

	if (n != 0) {

		/* larger the recv buffer, and copy the new
		 * received bytes to the buffer */
		length = ac->buffer ? strlen(ac->buffer) : 0;
		ac->buffer = g_realloc(ac->buffer, length + n + 1);
		memcpy(ac->buffer + length, buf, n + 1);

		return TRUE;

	} else { /* receive complete, start process */

		if (hybrid_get_http_code(ac->buffer) != 200) {
			goto error;
		}

		if (!(pos = g_strrstr(ac->buffer, "\r\n\r\n"))) {
			goto error;
		}

		pos += 4;

		if (strlen(pos) != hybrid_get_http_length(ac->buffer)) {
			goto error;
		}

		if (parse_configuration(ac, pos) != HYBRID_OK) {
			g_free(ac->buffer);
			ac->buffer = NULL;
			goto error;
		}

		g_free(ac->buffer);
		ac->buffer = NULL;

		/* now we start sipc register */

		hybrid_proxy_connect(ac->sipc_proxy_ip, ac->sipc_proxy_port,
				sipc_reg_action, ac);
	}

	return FALSE;
error:
	hybrid_account_error_reason(ac->account, _("read cfg failed"));
	return FALSE;
}

/**
 * Callback function to handle the cfg connect event.
 */
static gboolean
cfg_connect_cb(gint sk, gpointer user_data)
{
	gchar *http;
	gchar *body;
	fetion_account *ac = (fetion_account*)user_data;

	body = generate_configuration_body(ac);
	http = g_strdup_printf("POST /nav/getsystemconfig.aspx HTTP/1.1\r\n"
				   "User-Agent: IIC2.0/PC "PROTO_VERSION"\r\n"
				   "Host: %s\r\n"
				   "Connection: Close\r\n"
				   "Content-Length: %d\r\n\r\n%s",
				   NAV_SERVER, strlen(body), body);

	g_free(body);

	hybrid_debug_info("fetion", "send:\n%s", http);

	if (send(sk, http, strlen(http), 0) == -1) {
		hybrid_account_error_reason(ac->account, "downloading cfg error");
		g_free(http);

		return FALSE;
	}

	g_free(http);

	hybrid_event_add(sk, HYBRID_EVENT_READ, cfg_read_cb, ac);

	return FALSE;
}

/**
 * Callback function to handle the ssi read event. We get the response
 * string from the ssi server here.
 */
static gboolean
ssi_auth_cb(HybridSslConnection *ssl, gpointer user_data)
{
	gchar buf[BUF_LENGTH];
	gchar *pos, *pos1;
	gint ret;
	fetion_account *ac = (fetion_account*)user_data;

	ret = hybrid_ssl_read(ssl, buf, sizeof(buf));

	if (ret == -1 || ret == 0) {
		hybrid_debug_error("ssi", "ssi server response error");
		return FALSE;
	}

	buf[ret] = '\0';

	hybrid_debug_info("fetion", "recv:\n%s", buf);

	if (hybrid_get_http_code(buf) != 200) {
		goto ssi_auth_err;
	}

	if (!(pos = strstr(buf, "ssic="))) {
		goto ssi_auth_err;
	}

	pos += 5;

	for (pos1 = pos; *pos1 && *pos1 != ';'; pos1 ++);

	ac->ssic = g_strndup(pos, pos1 - pos);

	if (!(pos = g_strrstr(buf, "\r\n\r\n"))) {
		goto ssi_auth_err;
	}

	pos += 4;

	if (strlen(pos) != hybrid_get_http_length(buf)) {
		goto ssi_auth_err;
	}

	if (parse_ssi_response(ac, pos) != HYBRID_OK) {
		goto ssi_auth_err;
	}

	/* now we will download the configuration */
	hybrid_proxy_connect(NAV_SERVER, 80, cfg_connect_cb, ac);

	return FALSE;

ssi_auth_err:
	hybrid_account_error_reason(ac->account, _("ssi authentication failed"));
	return FALSE;
}

gboolean 
ssi_auth_action(HybridSslConnection *isc, gpointer user_data)
{
	gchar *password;
	gchar no_url[URL_LENGTH];
	gchar verify_url[URL_LENGTH];
	gchar ssl_buf[BUF_LENGTH];
	gint pass_type;
	fetion_account *ac = (fetion_account*)user_data;
	
	hybrid_debug_info("fetion", "ssi authencating");
	password = hash_password_v4(ac->userid, ac->password);

	if (ac->mobileno) {
		g_snprintf(no_url, sizeof(no_url) - 1, "mobileno=%s", ac->mobileno);

	} else {
		g_snprintf(no_url, sizeof(no_url) - 1, "sid=%s", ac->sid);
	}

	*verify_url = '\0';
	/**
	 * if the verification is not NULL ,it means we need to add the 
	 * confirm code in the request url.
	 */
	if (ac->verification != NULL && ac->verification->code != NULL) {
		g_snprintf(verify_url, sizeof(verify_url) - 1,
						"&pid=%s&pic=%s&algorithm=%s",
						ac->verification->guid,
						ac->verification->code,
						ac->verification->algorithm);
	}

	pass_type = (ac->userid == NULL || *(ac->userid) == '\0' ? 1 : 2);

	g_snprintf(ssl_buf, sizeof(ssl_buf) - 1,
			"GET /ssiportal/SSIAppSignInV4.aspx?%s"
			"&domains=fetion.com.cn%s&v4digest-type=%d&v4digest=%s\r\n"
			"User-Agent: IIC2.0/pc "PROTO_VERSION"\r\n"
			"Host: %s\r\n"
			"Cache-Control: private\r\n"
			"Connection: Keep-Alive\r\n\r\n",
			no_url, verify_url, pass_type, password, SSI_SERVER);

	g_free(password);

	hybrid_debug_info("fetion", "send:\n%s", ssl_buf);

	/* write the request to ssl connection, and a callback function
	 * to handle the read event. */
	hybrid_ssl_write(isc, ssl_buf, strlen(ssl_buf));
	hybrid_ssl_event_add(isc, ssi_auth_cb, ac);

	return FALSE;
}

/**
 * Callback function to handle the sipc reg response.
 */
static gboolean
sipc_reg_cb(gint sk, gpointer user_data)
{
	gchar buf[BUF_LENGTH];
	gchar *digest;
	gchar *nonce, *key, *aeskey;
	gchar *response;
	fetion_account *ac = (fetion_account*)user_data;
	gint n;

	if ((n = recv(sk, buf, sizeof(buf), 0)) == -1) {
		hybrid_account_error_reason(ac->account, _("sipc reg error."));
		return FALSE;
	}

	buf[n] = '\0';

	hybrid_debug_info("fetion", "recv:\n%s", buf);

	/* parse response, we need the key and nouce */
	digest = sip_header_get_attr(buf, "W");

	if (parse_sipc_reg_response(digest, &nonce, &key) != HYBRID_OK) {
		g_free(digest);
		return FALSE;
	}

	aeskey = generate_aes_key();

	response = generate_response(nonce, ac->userid, ac->password, key, aeskey);

	sipc_aut_action(sk, ac, response);

	g_free(digest);
	g_free(nonce);
	g_free(key);
	g_free(aeskey);
	g_free(response);

	return FALSE;
}

/**
 * This function starts to register to the sipc server, since we have
 * got the ip address and port from the cfg string in the last step.
 */
static gboolean
sipc_reg_action(gint sk, gpointer user_data)
{
	gchar *sipmsg;
	gchar *cnouce = generate_cnouce();
	fetion_account *ac = (fetion_account*)user_data;
	fetion_sip *sip = ac->sip;

	hybrid_debug_info("fetion", "sipc registeration action");

	fetion_sip_set_type(sip, SIP_REGISTER);

	sip_header *cheader = sip_header_create("CN", cnouce);
	sip_header *client = sip_header_create("CL",
			"type=\"pc\" ,version=\""PROTO_VERSION"\"");

	fetion_sip_add_header(sip, cheader);
	fetion_sip_add_header(sip, client);

	g_free(cnouce);

	sipmsg = fetion_sip_to_string(sip, NULL);

	hybrid_debug_info("fetion", "start registering to sip server(%s:%d)",
			ac->sipc_proxy_ip, ac->sipc_proxy_port);

	hybrid_debug_info("fetion", "send:\n%s", sipmsg);

	if (send(sk, sipmsg, strlen(sipmsg), 0) == -1) {
		hybrid_account_error_reason(ac->account, "sipc reg error");
		g_free(sipmsg);
		return FALSE;
	}

	hybrid_event_add(sk, HYBRID_EVENT_READ, sipc_reg_cb, ac);

	g_free(sipmsg);

	return FALSE;
}

/**
 * Callback function to handle the pushed message.
 */
static gboolean
push_cb(gint sk, gpointer user_data)
{
	gchar sipmsg[BUF_LENGTH];
	gchar *pos;
	gchar *h;
	gchar *msg;
	fetion_account *ac = (fetion_account*)user_data;
	gint n;
	guint len, data_len;

	if ((n = recv(sk, sipmsg, sizeof(sipmsg), 0)) == -1) {
		hybrid_account_error_reason(ac->account, "connection terminated");
		return FALSE;
	}

	sipmsg[n] = '\0';

	data_len = ac->buffer ? strlen(ac->buffer) : 0;
	ac->buffer = (gchar*)realloc(ac->buffer, data_len + n + 1);
	memcpy(ac->buffer + data_len, sipmsg, n + 1);

recheck:
	data_len = ac->buffer ? strlen(ac->buffer) : 0;

	if ((pos = strstr(ac->buffer, "\r\n\r\n"))) {
		pos += 4;
		h = (gchar*)g_malloc0(data_len - strlen(pos) + 1);
		memcpy(h, ac->buffer, data_len - strlen(pos));
		h[data_len - strlen(pos)] = '\0';

		if (strstr(h, "L: ")) {
			len = fetion_sip_get_length(ac->buffer);

			if (len <= strlen(pos)) {
				msg = (gchar*)g_malloc0(strlen(h) + len + 1);
				memcpy(msg, ac->buffer, strlen(h) + len);
				msg[strlen(h) + len] = '\0';
				/* A message is complete, process it. */
				process_pushed(ac, msg);

				memmove(ac->buffer, ac->buffer + strlen(msg), data_len - strlen(msg));
				ac->buffer = (gchar*)realloc(ac->buffer, data_len - strlen(msg) + 1);
				ac->buffer[data_len - strlen(msg)] = '\0';

				g_free(msg);
				g_free(h);
				msg = NULL;
				h = NULL;

				goto recheck;
			}

		} else {
			/* A message is complete, process it. */
			process_pushed(ac, h);

			memmove(ac->buffer, ac->buffer + strlen(h), data_len - strlen(h));
			ac->buffer = (gchar*)realloc(ac->buffer, data_len - strlen(h) + 1);
			ac->buffer[data_len - strlen(h)] = '\0';

			g_free(h);
			h = NULL;
			goto recheck;
		}
		g_free(h);
	} 

	return TRUE;
}

/**
 * Callback function to handle the read event after
 * rendered sipc authentication.
 */
static gint
sipc_auth_cb(fetion_account *ac, const gchar *sipmsg,
		fetion_transaction *trans)
{
	gint code;
	gint length;
	gchar *pos;

	code = fetion_sip_get_code(sipmsg);

	if (code == FETION_SIP_OK) { /**< ok, we got the contact list */

		/* update the portrait. */
		fetion_account_update_portrait(ac);

		length = fetion_sip_get_length(sipmsg);
		pos = strstr(ac->buffer, "\r\n\r\n") + 4;
		parse_sipc_resp(ac, pos, length);

		/* set the nickname of the hybrid account. */
		hybrid_account_set_nickname(ac->account, ac->nickname);

		hybrid_account_set_state(ac->account, HYBRID_STATE_INVISIBLE);

		/* set the connection status. */
		hybrid_account_set_connection_status(ac->account,
				HYBRID_CONNECTION_CONNECTED);

		/* init group list */
		fetion_groups_init(ac);

		/* init buddy list */
		fetion_buddies_init(ac);

		/* start scribe the pushed msg */
		fetion_buddy_scribe(ac);

	} else {
		hybrid_debug_error("fetion", "sipc authentication error.");
		g_free(ac->buffer);
		ac->buffer = NULL;

		return FALSE;
	}

	return 0;
}

/**
 * Start sipc authencation with the response string.
 *
 * @param response It is generated by generate_response().
 */
static gint
sipc_aut_action(gint sk, fetion_account *ac, const gchar *response)
{
	gchar *sipmsg;
	gchar *body;
	sip_header *aheader;
	sip_header *akheader;
	sip_header *ackheader;
	fetion_transaction *trans;

	fetion_sip *sip = ac->sip;
	ac->sk = sk;

	hybrid_debug_info("fetion", "sipc authencation action");

	body = generate_auth_body(ac);

	fetion_sip_set_type(sip, SIP_REGISTER);

	aheader = sip_authentication_header_create(response);
	akheader = sip_header_create("AK", "ak-value");

	trans = transaction_create();
	transaction_set_callid(trans, sip->callid);
	transaction_set_callback(trans, sipc_auth_cb);
	transaction_add(ac, trans);
	
	fetion_sip_add_header(sip, aheader);
	fetion_sip_add_header(sip, akheader);

	if(ac->verification != NULL && ac->verification->algorithm != NULL)	{
		ackheader = sip_ack_header_create(ac->verification->code,
										  ac->verification->algorithm,
										  ac->verification->type,
										  ac->verification->guid);
		fetion_sip_add_header(sip , ackheader);
	}
	//fetion_verification_free(user->verification);
	//user->verification = NULL;
	

	sipmsg = fetion_sip_to_string(sip, body);

	g_free(body);

	hybrid_debug_info("fetion", "Start sipc authentication , with ak-value");
	hybrid_debug_info("fetion", "send:\n%s", sipmsg);

	if(send(sk, sipmsg, strlen(sipmsg), 0) == -1) {
		hybrid_debug_error("fetion", "send sipc auth request:%s\n",
				strerror(errno));
		g_free(sipmsg);

		return HYBRID_ERROR; 
	}
	g_free(sipmsg);

	/* now we start to handle the pushed messages */
	ac->source = hybrid_event_add(sk, HYBRID_EVENT_READ, push_cb, ac);

	return 0;
}

/**
 * The first step to hash the password. We got two unsigned char arrays
 * as input, the first is generated in hash_password_v2(), the second is
 * some kind of unsigned char password, maybe generated by other functions,
 * we join the two arrays together, and make SHA1 on it, as we know, the 
 * result of SHA1 is an unsigned char array, then we converted it into 
 * a signed char array using hextostr(), and then return.
 *
 * NOTE: the function will finally be called by hash_password_v4()
 */
static gchar*
hash_password_v1(const guchar *b0, gint b0len, const guchar *password,
		gint psdlen) 
{
	guchar tmp[20];
	gchar *res;
	SHA_CTX ctx;
	guchar *dst = (guchar*)g_malloc0(b0len + psdlen + 1);

	memset(tmp, 0, sizeof(tmp));
	memcpy(dst, b0, b0len);
	memcpy(dst + b0len, password, psdlen);

	SHA1_Init(&ctx);
	SHA1_Update(&ctx, dst, b0len + psdlen);
	SHA1_Final(tmp, &ctx);

	g_free(dst);
	res = hextostr(tmp , 20);

	return res;
}

/**
 * Another hash function. 
 *
 * @param userid The user ID allocated by fetion server.
 * @param passwordhex The return value of hash_password_v1().
 *
 * @return The hashed value.
 */
static gchar*
hash_password_v2(const gchar *userid, const gchar *passwordhex)
{
	gint id = atoi(userid);
	gchar *res;
	guchar *bid = (guchar*)(&id);
	guchar ubid[4];
	gint bpsd_len;
	guchar *bpsd = strtohex(passwordhex , &bpsd_len);

	memcpy(ubid , bid , 4);

	res = hash_password_v1(ubid , sizeof(id) , bpsd , bpsd_len);
	g_free(bpsd);

	return res;
}

/**
 * The function generate the hashed password we need both in
 * ssi authentication message and sipc authencation message.
 * when we start ssi authentication, we haven't got our user ID,
 * for it is returned from the server after ssi authencation.
 * so we make it NULL when we make ssi authentication, after that,
 * we get a user ID, and then we can use it to generate a hashed
 * password array for sipc authentication.
 *
 * @param userid The user ID.
 * @param password The raw password.
 *
 * @return The hashed password.
 */
static gchar*
hash_password_v4(const gchar *userid, const gchar *password)
{
	const gchar *domain = "fetion.com.cn:";
	gchar *res, *dst;
	guchar *udomain = (guchar*)g_malloc0(strlen(domain));
	guchar *upassword = (guchar*)g_malloc0(strlen(password));

	memcpy(udomain, (guchar*)domain, strlen(domain));
	memcpy(upassword, (guchar*)password, strlen(password));
	res = hash_password_v1(udomain, strlen(domain), upassword, strlen(password));
	g_free(udomain);
	g_free(upassword);

	if (userid == NULL || *userid == '\0') {
		return res;
	}

	dst = hash_password_v2(userid , res);
	g_free(res);

	return dst;
}

/**
 * Convert the input hex-like char array to unsigned char array,
 * Two char bytes can be converted into one uchar byte.
 *
 * @param in The char array to convert.
 * @param len The number of bytes of the input array.
 *
 * @return The result uchar array.
 */
static guchar*
strtohex(const gchar *in, gint *len)
{
	guchar *out = (guchar*)g_malloc0(strlen(in)/2 );
	gint i = 0, j = 0, k = 0, length = 0;
	gchar tmp[3] = { 0, };
	gint inlength;
	inlength = (gint)strlen(in);

	while (i < inlength) {
		tmp[k++] = in[i++];
		tmp[k] = '\0';

		if(k == 2) {
			out[j++] = (guchar)strtol(tmp , (gchar**)NULL , 16);
			k = 0;
			length ++;
		}
	}

	if (len != NULL) {
		*len = length;
	}

	return out;
}

/**
 * Convert the input unsigned char array to hex-like char array,
 * like 'EF3A4B', One uchar byte can be converted into two char bytes.
 *
 * @param in The unsigned char array to convert.
 * @param len The number of bytes of the input array.
 *
 * @return The result char array.
 */
static gchar*
hextostr(const guchar *in, gint len) 
{
	gchar *res = (gchar*)g_malloc0(len * 2 + 1);
	gint reslength;
	gint i = 0;

	while (i < len) {
		sprintf(res + i * 2, "%02x" , in[i]);
		i ++;
	}

	i = 0;
	reslength = (gint)strlen(res);

	while (i < reslength) {
		res[i] = g_ascii_toupper(res[i]);
		i ++;
	};

	return res;
}

/**
 * Generate the xml body of the cfg-downloading request. We will get
 * the following output:
 *
 * <config>
 *     <user mobile-no="152********"/>
 *     <client type="PC" version="4.3.0980" platform="W5.1"/>
 *     <servers version="0"/>
 *     <parameters version="0"/>
 *     <hints version="0"/>
 *  </config>
 */
static gchar*
generate_configuration_body(fetion_account *ac)
{
	xmlnode *root;
	xmlnode *node;
	gchar root_node[] = "<config></config>";

	g_return_val_if_fail(ac != NULL, NULL);

	root = xmlnode_root(root_node, strlen(root_node));

	node = xmlnode_new_child(root, "user");

	if (ac->mobileno && *(ac->mobileno) != '\0') {
		xmlnode_new_prop(node, "mobile-no", ac->mobileno);

	} else {
		xmlnode_new_prop(node, "sid", ac->sid);
	}

	node = xmlnode_new_child(root, "client");
	xmlnode_new_prop(node, "type", "PC");
	xmlnode_new_prop(node, "version", PROTO_VERSION);
	xmlnode_new_prop(node, "platform", "W5.1");

	node = xmlnode_new_child(root, "servers");
	xmlnode_new_prop(node, "version", "0");

	node = xmlnode_new_child(root, "parameters");
	xmlnode_new_prop(node, "version", "0");

	node = xmlnode_new_child(root, "hints");
	xmlnode_new_prop(node, "version", "0");

	return xmlnode_to_string(root);
}

/**
 * Parse the configuration information. We will get:
 * sipc-proxy,get-url, servers-version, parameters-version,
 * and hints-version
 */
static gint
parse_configuration(fetion_account *ac, const gchar *cfg)
{
	xmlnode *node;
	xmlnode *root;
	gchar *value;
	gchar *pos, *pos1;

	g_return_val_if_fail(cfg != NULL, 0);

	root = xmlnode_root(cfg, strlen(cfg));

	if (!root) {
		goto cfg_parse_err;
	}

	if ((node = xmlnode_find(root, "servers"))) {
		ac->cfg_server_version = xmlnode_prop(node, "version");
	}

	if ((node = xmlnode_find(root, "parameters"))) {
		ac->cfg_param_version = xmlnode_prop(node, "version");
	}

	if ((node = xmlnode_find(root, "hints"))) {
		ac->cfg_hint_version = xmlnode_prop(node, "hints");
	}

	if (!(node = xmlnode_find(root, "sipc-proxy"))) {
		goto cfg_parse_err;
	}

	value = xmlnode_content(node);

	for (pos = value; *pos && *pos != ':'; pos ++ );

	if (*pos == '\0') {
		g_free(value);
		goto cfg_parse_err;
	}

	ac->sipc_proxy_ip = g_strndup(value, pos - value);
	ac->sipc_proxy_port = atoi(pos + 1);

	g_free(value);

	if (!(node = xmlnode_find(root, "get-uri"))) {
		goto cfg_parse_err;
	}

	value = xmlnode_content(node);
	
	for (pos1 = value; *pos1 && *pos1 != '/'; pos1 ++);
	if (*pos1 == '\0' || *(pos1 + 1) != '/') {
		goto cfg_parse_err;
	}

	pos1 += 2;

	for (pos = pos1; *pos && *pos != '/'; pos ++);
	if (*pos == '\0') {
		goto cfg_parse_err;
	}

	ac->portrait_host_name = g_strndup(pos1, pos - pos1);

	pos1 = pos + 1;

	for (pos = pos1; *pos && *pos != '/'; pos ++);
	if (*pos == '\0') {
		goto cfg_parse_err;
	}

	ac->portrait_host_path = g_strndup(pos1, pos - pos1);

	xmlnode_free(root);

	return HYBRID_OK;

cfg_parse_err:
	xmlnode_free(root);
	hybrid_debug_error("fetion", "parse cfg body");
	return HYBRID_ERROR;
}

/**
 * Parset the sipc reg response, get two important attribute: nonce and key.
 */
static gint
parse_sipc_reg_response(const gchar *reg_response, gchar **nonce, gchar **key)
{
	gchar nonce_flag[] = "nonce=\"";
	gchar key_flag[] = "key=\"";
	gchar *pos, *cur;

	g_return_val_if_fail(reg_response != NULL, HYBRID_ERROR);

	if (!(pos = g_strrstr(reg_response, nonce_flag))) {
		goto parse_sipc_error;
	}

	pos += strlen(nonce_flag);

	for (cur = pos; *cur && *cur != '\"'; cur ++);

	if (*cur == '\0') {
		goto parse_sipc_error;
	}

	*nonce = g_strndup(pos, cur - pos);

	if (!(pos = g_strrstr(reg_response, key_flag))) {
		goto parse_sipc_error;
	}

	pos += strlen(key_flag);

	for (cur = pos; *cur && *cur != '\"'; cur ++);

	if (*cur == '\0') {
		goto parse_sipc_error;
	}

	*key = g_strndup(pos, cur - pos);

	return HYBRID_OK;

parse_sipc_error:
	hybrid_debug_error("fetion", "parse sipc register response");
	return HYBRID_ERROR;
	
}

/**
 * Generate a 64 bytes random char array for password hash.
 */
static gchar*
generate_aes_key()
{
	return g_strdup_printf(
			"%04X%04X%04X%04X%04X%04X%04X%04X"
			"%04X%04X%04X%04X%04X%04X%04X%04X", 
			rand() & 0xFFFF, rand() & 0xFFFF, 
			rand() & 0xFFFF, rand() & 0xFFFF,
			rand() & 0xFFFF, rand() & 0xFFFF,
			rand() & 0xFFFF, rand() & 0xFFFF,
			rand() & 0xFFFF, rand() & 0xFFFF, 
			rand() & 0xFFFF, rand() & 0xFFFF,
			rand() & 0xFFFF, rand() & 0xFFFF,
			rand() & 0xFFFF, rand() & 0xFFFF);
}

/**
 * Generate a 32 bytes random char array for sipc registeration.
 */
static gchar*
generate_cnouce()
{
	return g_strdup_printf(
			"%04X%04X%04X%04X%04X%04X%04X%04X", 
			rand() & 0xFFFF, rand() & 0xFFFF, 
			rand() & 0xFFFF, rand() & 0xFFFF,
			rand() & 0xFFFF, rand() & 0xFFFF,
			rand() & 0xFFFF, rand() & 0xFFFF);
}


static gchar* 
generate_response(const gchar *nouce, const gchar *userid,
		const gchar *password, const gchar *publickey, const gchar *aeskey_raw)
{
	gchar *psdhex = hash_password_v4(userid, password);
	gchar modulus[257];
	gchar exponent[7];
	gint ret, flen;
	BIGNUM *bnn, *bne;
	guchar *out;
	guchar *nonce, *aeskey, *psd, *res;
	gint nonce_len, aeskey_len, psd_len;
	RSA *r = RSA_new();

	memset(modulus, 0, sizeof(modulus));
	memset(exponent, 0, sizeof(exponent));

	memcpy(modulus, publickey, 256);
	memcpy(exponent, publickey + 256, 6);

	nonce = (guchar*)g_malloc0(strlen(nouce) + 1);
	memcpy(nonce, (guchar*)nouce, strlen(nouce));
	nonce_len = strlen(nouce);

	psd = strtohex(psdhex, &psd_len);

	aeskey = strtohex(aeskey_raw, &aeskey_len);

	res = (guchar*)g_malloc0(nonce_len + aeskey_len + psd_len + 1);
	memcpy(res, nonce, nonce_len);
	memcpy(res + nonce_len, psd, psd_len);
	memcpy(res + nonce_len + psd_len, aeskey, aeskey_len);

	bnn = BN_new();
	bne = BN_new();
	BN_hex2bn(&bnn, modulus);
	BN_hex2bn(&bne, exponent);
	r->n = bnn;	r->e = bne;	r->d = NULL;

//	RSA_print_fp(stdout, r, 5);
	flen = RSA_size(r);
	out =  (guchar*)g_malloc0(flen);
	hybrid_debug_info("fetion", "start encrypting response");
	ret = RSA_public_encrypt(nonce_len + aeskey_len + psd_len,
			res, out, r, RSA_PKCS1_PADDING);

	if (ret < 0) {
		hybrid_debug_info("fetion", "encrypt response failed!");
		g_free(res); 
		g_free(aeskey);
		g_free(psd);
		g_free(nonce);
		return NULL;
	}

	RSA_free(r);
	hybrid_debug_info("fetion", "encrypting reponse success");
	g_free(res); 
	g_free(aeskey);
	g_free(psd);
	g_free(nonce);

	return hextostr(out , ret);
}

/**
 * Generate the sipc authentication request message body.
 */
static gchar* 
generate_auth_body(fetion_account *ac)
{
	gchar root_raw[] = "<args></args>";
	xmlnode *node;
	xmlnode *subnode;
	xmlnode *root = xmlnode_root(root_raw, strlen(root_raw));

	node = xmlnode_new_child(root, "device");
	xmlnode_new_prop(node, "machine-code", "001676C0E351");

	node = xmlnode_new_child(root, "caps");
	xmlnode_new_prop(node, "value", "1ff");

	node = xmlnode_new_child(root, "events");
	xmlnode_new_prop(node, "value", "7f");

	node = xmlnode_new_child(root, "user-info");
	xmlnode_new_prop(node, "mobile-no", ac->mobileno);
	xmlnode_new_prop(node, "user-id", ac->userid);
	
	subnode = xmlnode_new_child(node, "personal");
	xmlnode_new_prop(subnode, "version", "0");
	xmlnode_new_prop(subnode, "attributes", "v4default");

	subnode = xmlnode_new_child(node, "custom-config");
	xmlnode_new_prop(subnode, "version", "0");

	subnode = xmlnode_new_child(node, "contact-list");
	xmlnode_new_prop(subnode, "version", "0");
	xmlnode_new_prop(subnode, "buddy-attributes", "v4default");

	node = xmlnode_new_child(root, "credentials");
	xmlnode_new_prop(node, "domains", "fetion.com.cn");

	node = xmlnode_new_child(root, "presence");
	subnode = xmlnode_new_child(node, "basic");
	xmlnode_new_prop(subnode, "value", "0");
	xmlnode_new_prop(subnode, "desc", "");

	return xmlnode_to_string(root);
}

/**
 * parse the sipc authentication response, we can get the basic 
 * information and the contact list of this account.
 */
static void
parse_sipc_resp(fetion_account *ac, const gchar *body, gint len)
{
	xmlnode *root;
	xmlnode *node;
	gchar *temp, *temp1;
	gchar *pos, *stop;
	fetion_group *group;
	fetion_buddy *buddy;

	gboolean has_ungroup = FALSE;

	g_return_if_fail(ac != NULL);
	g_return_if_fail(body != NULL);
	g_return_if_fail(len != 0);

	root = xmlnode_root(body, len);

	/* login info */
	node = xmlnode_find(root, "client");
	ac->last_login_ip = xmlnode_prop(root, "last-login-ip");
	ac->public_ip = xmlnode_prop(root, "public-ip");
	ac->last_login_time = xmlnode_prop(root, "last-login-time");

	/* personal info */
	node = xmlnode_find(root, "personal");
	ac->nickname = xmlnode_prop(node, "nickname");
	ac->mood_phrase = xmlnode_prop(node, "impresa");
	ac->personal_version = xmlnode_prop(node, "version");
	ac->portrait_crc = xmlnode_prop(node, "portrait-crc");
	temp = xmlnode_prop(node, "carrier-region");

	/* region */
	if (temp) {
		for (stop = temp, pos = temp; *stop && *stop != '.'; stop ++);
		ac->country = g_strndup(pos, stop - pos);

		for (pos = stop + 1, stop ++; *stop && *stop != '.'; stop ++);
		ac->province = g_strndup(pos, stop - pos);

		for (pos = stop + 1, stop ++; *stop && *stop != '.'; stop ++);
		ac->city = g_strndup(pos, stop - pos);
	}

	g_free(temp);

	/* contact list version */
	node = xmlnode_find(root, "contact-list");
	fetion_config_save_buddies(ac, node);
	ac->contact_list_version = xmlnode_prop(node, "version");

	/* group list */
	node = xmlnode_find(root, "buddy-lists");
	node = xmlnode_child(node);

	while (node) {
		temp = xmlnode_prop(node, "name");
		temp1 = xmlnode_prop(node, "id");

		group = fetion_group_create(atoi(temp1), temp);
		ac->groups = g_slist_append(ac->groups, group);

		g_free(temp);
		g_free(temp1);

		node = node->next;
	}

	/* contact list  */
	node = xmlnode_find(root, "buddies");
	node = xmlnode_child(node);

	while (node) {
		buddy = fetion_buddy_create();
		buddy->userid = xmlnode_prop(node, "i");
		buddy->sipuri = xmlnode_prop(node, "u");
		buddy->localname = xmlnode_prop(node, "n");
		buddy->groups = xmlnode_prop(node, "l");
		buddy->sid = get_sid_from_sipuri(buddy->sipuri);

		ac->buddies = g_slist_append(ac->buddies, buddy);

		/* ungrouped */
		if (*(buddy->groups) == '\0' || buddy->groups[0] == '0') {
			g_free(buddy->groups);
			buddy->groups = g_strdup("0");

			if (!has_ungroup) { /**< add an "ungroup" group */
				group = fetion_group_create(0, _("Ungrouped"));
				ac->groups = g_slist_append(ac->groups, group);

				has_ungroup = TRUE;
			}
		}

		node = node->next;
	}

	/* custom config */
	node = xmlnode_find(root, "custom-config");
	ac->custom_config = xmlnode_content(node);
	ac->custom_config_version = xmlnode_prop(node, "version");

	xmlnode_free(root);
}

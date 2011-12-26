/***************************************************************************
 *   Copyright (C) 2011 by levin                                           *
 *   levin108@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 ***************************************************************************/

#include <glib.h>
#include "util.h"
#include "connect.h"
#include "eventloop.h"
#include "xmlnode.h"
#include "confirm.h"

#include "fetion.h"
#include "fx_trans.h"
#include "fx_login.h"
#include "fx_account.h"
#include "fx_group.h"
#include "fx_buddy.h"
#include "fx_config.h"

#define VERIFY_TYPE_SSI 1
#define VERIFY_TYPE_SIP 2

struct verify_data {
    gint                 type;  /* ssi verify || sipc verify */
    fetion_account      *ac;    /* common data */
    HybridSslConnection *ssl;   /* used by ssi verify */
    gint                 sipc_conn; /* used by sipc verify */
    gchar                response[BUF_LENGTH]; /* used by sipc verify */
    gchar               *data;
} verify_data;

static gchar *hash_password_v1(const guchar *b0, gint b0len,
        const guchar *password,    gint psdlen);
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
static gint parse_ssi_fail_resp(fetion_account *ac, const gchar *response);
static gboolean pic_download_cb(gint sk, gpointer user_data);
static gboolean pic_read_cb(gint sk, gpointer user_data);
static void pic_code_ok_cb(HybridAccount *ac, const gchar *code, gpointer user_data);
static void pic_code_cancel_cb(HybridAccount *ac, gpointer user_data);
static gint parse_sipc_verification(fetion_account *ac, const gchar *resp);
/**
 * Parse the ssi authentication response string. then we can
 * get the following information: sipuri/mobileno/sid/ssic.
 */
static gint
parse_ssi_response(fetion_account *ac, const gchar *response)
{
    gchar   *prop;
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
static gint
parse_sipc_verification(fetion_account *ac, const gchar *resp)
{
    gchar *w;
    gchar *start;
    gchar *end;

    ac->verification = fetion_verification_create();

    w = sip_header_get_attr(resp, "W");

    for (start = w; *start != '\0' && *start != '\"'; start ++);

    if (!*start) {
        g_free(w);
        return HYBRID_ERROR;
    }

    start ++;

    for (end = start; *end != '\0' && *end != '\"'; end ++);

    if (!*end) {
        g_free(w);
        return HYBRID_ERROR;
    }

    ac->verification->algorithm = g_strndup(start, end - start);

    for (start = end + 1; *start != '\0' && *start != '\"'; start ++);

    if (!*start) {
        g_free(w);
        return HYBRID_ERROR;
    }

    start ++;

    for (end = start; *end != '\0' && *end != '\"'; end ++);

    if (!*end) {
        g_free(w);
        return HYBRID_ERROR;
    }

    ac->verification->type = g_strndup(start, end - start);
    

    return HYBRID_OK;
}

static gint
parse_ssi_fail_resp(fetion_account *ac, const gchar *response)
{
    xmlnode      *root;
    xmlnode      *node;
    Verification *ver;
    gchar        *pos;

    ver = fetion_verification_create();

    if (!(pos = strstr(response, "\r\n\r\n"))) {
        return HYBRID_ERROR;
    }
    
    pos += 4;
    
    root = xmlnode_root(pos, strlen(pos));

    if (!(node = xmlnode_find(root, "results"))) {
        return HYBRID_ERROR;
    }

    if (xmlnode_has_prop(node, "desc")) {
        ver->desc = xmlnode_prop(node, "desc");
    }

    if (!(node = xmlnode_find(root, "verification"))) {
        return HYBRID_ERROR;
    }

    if (xmlnode_has_prop(node, "algorithm")) {
        ver->algorithm = xmlnode_prop(node, "algorithm");
    }

    ac->verification = ver;

    return HYBRID_OK;
}

/**
 * Callback function to handle the cfg read event.
 */
static gboolean
cfg_read_cb(gint sk, gpointer user_data)
{
    gchar           buf[BUF_LENGTH];
    gint            n;
    gint            length = 0;
    gchar          *pos;
    fetion_account *ac     = (fetion_account*)user_data;

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

        hybrid_debug_info("fetion", "cfg recv:\n%s", ac->buffer);

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
    gchar          *http;
    gchar          *body;
    fetion_account *ac = (fetion_account*)user_data;

    hybrid_account_set_connection_string(ac->account, _("Downloading configure file..."));

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
        hybrid_account_error_reason(ac->account, _("downloading cfg error"));
        g_free(http);

        return FALSE;
    }

    g_free(http);

    ac->source = hybrid_event_add(sk, HYBRID_EVENT_READ, cfg_read_cb, ac);

    return FALSE;
}

static void
pic_code_ok_cb(HybridAccount *account, const gchar *code, gpointer user_data)
{
    fetion_account *ac = (fetion_account*)user_data;
    
    hybrid_debug_info("fetion", "pic code %s inputed.", code);

    g_free(ac->verification->code);
    ac->verification->code = g_strdup(code);

    if (VERIFY_TYPE_SSI == verify_data.type) {
        hybrid_ssl_connect(SSI_SERVER, 443, ssi_auth_action, ac);
        
    } else {
        printf("%s\n", verify_data.response);
        sipc_aut_action(verify_data.sipc_conn, ac, verify_data.response);
    }
}
static void
pic_code_cancel_cb(HybridAccount *ac, gpointer user_data)
{

}

static gboolean
pic_read_cb(gint sk, gpointer user_data)
{
    gint     n, len;
    gchar    sipmsg[BUF_LENGTH];
    gchar   *code, *pos;
    guchar  *pic;
    gint     piclen;
    xmlnode *root;
    xmlnode *node;
    
    fetion_account *ac = (fetion_account*)user_data;

    len    = ac->buffer ? strlen(ac->buffer) : 0;

    if((n = recv(sk, sipmsg, strlen(sipmsg), 0)) == -1) {
        return -1;
    }
    
    sipmsg[n] = 0;
    
    if(n == 0) {
           g_source_remove(ac->source);
        ac->source = 0;
        close(sk);
        
        if(! ac->buffer) {
            return 0;
        }

        hybrid_debug_info("fetion", "read message resp:\n%s", ac->buffer);

        if (200 != hybrid_get_http_code(ac->buffer)) {
            goto read_pic_err;
        }

        if(!(pos = strstr(ac->buffer, "\r\n\r\n"))) {
            goto read_pic_err;
        }

        pos += 4;

        if (!(root = xmlnode_root(pos, strlen(pos)))) {
            goto read_pic_err;
        }

        if (!(node = xmlnode_find(root, "pic-certificate"))) {
            xmlnode_free(root);
            goto read_pic_err;
        }

        if (!xmlnode_has_prop(node, "id") || !xmlnode_has_prop(node, "pic")) {
            xmlnode_free(root);
            goto read_pic_err;
        }

        ac->verification->guid = xmlnode_prop(node, "id");
        code                   = xmlnode_prop(node, "pic");
        
        pic = hybrid_base64_decode(code, &piclen);

        hybrid_confirm_window_create(ac->account, pic, piclen,
                                     pic_code_ok_cb, pic_code_cancel_cb, ac);

        g_free(code);
        g_free(pic);
        xmlnode_free(root);
        g_free(ac->buffer);
        ac->buffer = (gchar*)0;

        return FALSE;
    }

    ac->buffer = (gchar*)realloc(ac->buffer, len + n + 1);
    memcpy(ac->buffer + len, sipmsg, n + 1);

    return TRUE;

 read_pic_err:
    hybrid_debug_error("fetion", "read pic code error.");

    g_free(ac->buffer);
    ac->buffer = (gchar *)0;

    hybrid_account_error_reason(ac->account, _("read pic code error."));
    return FALSE;
}

/**
 * Callback function to handle the pic-code download event.
 */
static gboolean
pic_download_cb(gint sk, gpointer user_data)
{
    gchar          *cookie = NULL;
    gchar          *http;
    fetion_account *ac     = (fetion_account*)user_data;

    hybrid_debug_info("fetion", "start downloading pic-cocde.");

    if(ac->ssic) {
        cookie = g_strdup_printf("Cookie: ssic=%s\r\n", ac->ssic);
    }

    http = g_strdup_printf("GET /nav/GetPicCodeV4.aspx?algorithm=%s HTTP/1.1\r\n"
                           "%sHost: %s\r\n"
                           "User-Agent: IIC2.0/PC "PROTO_VERSION"\r\n"
                           "Connection: close\r\n\r\n",
                           ac->verification->algorithm == NULL ? "" : ac->verification->algorithm,
                           ac->ssic == NULL ? "" : cookie, NAV_SERVER);

    hybrid_debug_info("fetion", "download pic request:\n%s", http);

    if (send(sk, http, strlen(http), 0) == -1) {

        g_free(cookie);
        g_free(http);
        
        return FALSE;
    }

    ac->buffer = (char*)0;

       ac->source = hybrid_event_add(sk, HYBRID_EVENT_READ, pic_read_cb, user_data);

    g_free(cookie);
    g_free(http);

    return FALSE;
}

/**
 * Callback function to handle the ssi read event. We get the response
 * string from the ssi server here.
 */
static gboolean
ssi_auth_cb(HybridSslConnection *ssl, gpointer user_data)
{
    gchar           buf[BUF_LENGTH];
    gchar          *pos;
    gchar          *pos1;
    gint            ret;
    gint            code;
    fetion_account *ac = (fetion_account*)user_data;
    
    ret = hybrid_ssl_read(ssl, buf, sizeof(buf));

    if (ret == -1 || ret == 0) {
        hybrid_debug_error("ssi", "ssi server response error");
        return FALSE;
    }

    buf[ret] = '\0';

    hybrid_ssl_connection_destory(ssl);

    hybrid_debug_info("fetion", "recv:\n%s", buf);

    code = hybrid_get_http_code(buf);

    if (421 == code || 420 == code) {            /* confirm code needed. */
        if (HYBRID_ERROR == parse_ssi_fail_resp(ac, buf)) {
            goto ssi_auth_err;
        }

        verify_data.ssl     = ssl;
        verify_data.type = VERIFY_TYPE_SSI;

        hybrid_proxy_connect(NAV_SERVER, 80, pic_download_cb, ac);
        
        return FALSE;
    }

    if (200 != code) {
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

    /*
     * First of all, we load the account's version information from the disk,
     * so that we can use it for authenticating, if the server find the versions
     * are up-to-date, it would return a brief response message in stead of the
     * full version, so that we can use the information store locally. This method
     * makes account logining more faster.
     */
    fetion_config_load_account(ac);

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
    gchar  no_url[URL_LENGTH];
    gchar  verify_url[URL_LENGTH];
    gchar  ssl_buf[BUF_LENGTH];
    gint   pass_type;

    fetion_account *ac = (fetion_account*)user_data;

    hybrid_account_set_connection_string(ac->account, _("Start SSI authenticating..."));

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
    fetion_verification_destroy(ac->verification);
    ac->verification = NULL;

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
    gchar  buf[BUF_LENGTH];
    gchar *digest;
    gchar *nonce, *key, *aeskey;
    gchar *response;
    gint   n;

    fetion_account *ac = (fetion_account*)user_data;

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

    /* fill verify_data for pic confirm */
    strncpy(verify_data.response, response, sizeof(verify_data.response));

    /* now we start to handle the pushed messages */
    ac->source = hybrid_event_add(sk, HYBRID_EVENT_READ, hybrid_push_cb, ac);

    /* start sipc authencation action. */
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
    gchar          *sipmsg;
    gchar          *cnouce = generate_cnouce();
    fetion_account *ac     = (fetion_account*)user_data;
    fetion_sip     *sip    = ac->sip;

    hybrid_debug_info("fetion", "sipc registeration action");

    hybrid_account_set_connection_string(ac->account,
            _("start registering to the sipc server."));

    /* Now we start to register to the sipc server. */
    fetion_sip_set_type(sip, SIP_REGISTER);

    sip_header *cheader = sip_header_create("CN", cnouce);
    sip_header *client  = sip_header_create("CL", "type=\"pc\""
                                            " ,version=\""PROTO_VERSION"\"");

    fetion_sip_add_header(sip, cheader);
    fetion_sip_add_header(sip, client);

    g_free(cnouce);

    sipmsg = fetion_sip_to_string(sip, NULL);

    hybrid_debug_info("fetion", "start registering to sip server(%s:%d)",
            ac->sipc_proxy_ip, ac->sipc_proxy_port);

    hybrid_debug_info("fetion", "send:\n%s", sipmsg);

    if (send(sk, sipmsg, strlen(sipmsg), 0) == -1) {
        hybrid_account_error_reason(ac->account, _("sipc reg error"));
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
gboolean
hybrid_push_cb(gint sk, gpointer user_data)
{
    gchar           sipmsg[BUF_LENGTH];
    gchar          *pos;
    gchar          *h;
    gchar          *msg;
    fetion_account *ac = (fetion_account*)user_data;
    gint            n;
    guint           len, data_len;

    if ((n = recv(sk, sipmsg, sizeof(sipmsg), 0)) == -1) {
        hybrid_account_error_reason(ac->account, _("connection terminated"));
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
    gint   code;
    gint   length;
    gchar *pos;

    code = fetion_sip_get_code(sipmsg);

    hybrid_debug_info("fetion", "sipc recv:\n%s", sipmsg);

    if (code == FETION_SIP_OK) { /**< ok, we got the contact list */

        /* update the portrait. */
        fetion_account_update_portrait(ac);

        length = fetion_sip_get_length(sipmsg);
        pos = strstr(ac->buffer, "\r\n\r\n") + 4;

        parse_sipc_resp(ac, pos, length);

        /* set the nickname of the hybrid account. */
        hybrid_account_set_nickname(ac->account, ac->nickname);

        /* set the mood phrase of the hybrid account. */
        hybrid_account_set_status_text(ac->account, ac->mood_phrase);

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

    } else if (420 == code || 421 == code) {

        if (HYBRID_ERROR == parse_sipc_verification(ac, sipmsg)) {
            hybrid_account_error_reason(ac->account,
                                        _("Fetion Protocol ERROR."));
            return FALSE;
        }

        hybrid_debug_error("fetion", "sipc authentication need Verification.");

        verify_data.sipc_conn = ac->sk;
        verify_data.type      = VERIFY_TYPE_SIP;

        hybrid_proxy_connect(NAV_SERVER, 80, pic_download_cb, ac);

        g_free(ac->buffer);
        ac->buffer = NULL;

        return HYBRID_OK;
    } else {
        g_free(ac->buffer);
        ac->buffer = NULL;

        return HYBRID_ERROR;
    }

    return HYBRID_ERROR;
}

/**
 * Start sipc authencation with the response string.
 *
 * @param response It is generated by generate_response().
 */
static gint
sipc_aut_action(gint sk, fetion_account *ac, const gchar *response)
{
    gchar              *sipmsg;
    gchar              *body;
    sip_header         *aheader;
    sip_header         *akheader;
    sip_header         *ackheader;
    fetion_transaction *trans;
    fetion_sip         *sip = ac->sip;

    ac->sk = sk;

    hybrid_debug_info("fetion", "sipc authencation action");

    hybrid_account_set_connection_string(ac->account,
            _("start sipc authenticating..."));

    body = generate_auth_body(ac);

    fetion_sip_set_type(sip, SIP_REGISTER);

    aheader     = sip_authentication_header_create(response);
    akheader = sip_header_create("AK", "ak-value");

    trans = transaction_create();
    transaction_set_callid(trans, sip->callid);
    transaction_set_callback(trans, sipc_auth_cb);
    transaction_add(ac, trans);
    
    fetion_sip_add_header(sip, aheader);
    fetion_sip_add_header(sip, akheader);

    if(ac->verification != NULL && ac->verification->algorithm != NULL)    {
        ackheader = sip_ack_header_create(ac->verification->code,
                                          ac->verification->algorithm,
                                          ac->verification->type,
                                          ac->verification->guid);
        fetion_sip_add_header(sip , ackheader);
    }
    fetion_verification_destroy(ac->verification);
    ac->verification = NULL;
    
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
    guchar   tmp[20];
    gchar   *res;
    SHA_CTX  ctx;
    guchar  *dst = (guchar*)g_malloc0(b0len + psdlen + 1);

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
    gint    id   = atoi(userid);
    gchar  *res;
    guchar *bid  = (guchar*)(&id);
    guchar  ubid[4];
    gint    bpsd_len;
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
    const gchar *domain    = "fetion.com.cn:";
    gchar       *res, *dst;
    guchar      *udomain   = (guchar*)g_malloc0(strlen(domain));
    guchar      *upassword = (guchar*)g_malloc0(strlen(password));

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
    guchar *out  = (guchar*)g_malloc0(strlen(in)/2 );
    gint    i    = 0, j = 0, k = 0, length = 0;
    gchar tmp[3] = { 0, };
    gint    inlength;
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
    gint   reslength;
    gint   i   = 0;

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
    gchar   *res;

    g_return_val_if_fail(ac != NULL, NULL);

    root = xmlnode_root(root_node, strlen(root_node));

    node = xmlnode_new_child(root, "user");

    if (ac->mobileno && *(ac->mobileno) != '\0') {
        xmlnode_new_prop(node, "mobile-no", ac->mobileno);

    } else {
        xmlnode_new_prop(node, "sid", ac->sid);
    }

    node = xmlnode_new_child(root, "client");
    xmlnode_new_prop(node, "type", "HYBRID");
    xmlnode_new_prop(node, "version", PROTO_VERSION);
    xmlnode_new_prop(node, "platform", "W5.1");

    node = xmlnode_new_child(root, "servers");
    xmlnode_new_prop(node, "version", ac->cfg_server_version);

    node = xmlnode_new_child(root, "parameters");
    xmlnode_new_prop(node, "version", ac->cfg_param_version);

    node = xmlnode_new_child(root, "hints");
    xmlnode_new_prop(node, "version", ac->cfg_hint_version);

    res = xmlnode_to_string(root);

    xmlnode_free(root);

    return res;
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
    gchar   *value;
    gchar   *version;
    gchar   *pos, *pos1;

    g_return_val_if_fail(cfg != NULL, 0);

    root = xmlnode_root(cfg, strlen(cfg));

    if (!root) {
        goto cfg_parse_err;
    }

    if ((node = xmlnode_find(root, "servers"))) {
        version = xmlnode_prop(node, "version");
        if (g_strcmp0(version, ac->cfg_server_version) != 0) {
            g_free(ac->cfg_server_version);
            ac->cfg_server_version = version;

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

        } else {
            g_free(version);
        }
    }

    if ((node = xmlnode_find(root, "parameters"))) {
        g_free(ac->cfg_param_version);
        ac->cfg_param_version = xmlnode_prop(node, "version");
    }

    if ((node = xmlnode_find(root, "hints"))) {
        g_free(ac->cfg_hint_version);
        ac->cfg_hint_version = xmlnode_prop(node, "version");
    }

    if ((node = xmlnode_find(root, "client"))) {
        g_free(ac->cfg_client_version);
        ac->cfg_client_version = xmlnode_prop(node, "version");
    }

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
    gchar key_flag[]   = "key=\"";
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
    return g_strdup_printf("%04X%04X%04X%04X%04X%04X%04X%04X"
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
    return g_strdup_printf("%04X%04X%04X%04X%04X%04X%04X%04X", 
                           rand() & 0xFFFF, rand() & 0xFFFF, 
                           rand() & 0xFFFF, rand() & 0xFFFF,
                           rand() & 0xFFFF, rand() & 0xFFFF,
                           rand() & 0xFFFF, rand() & 0xFFFF);
}


static gchar* 
generate_response(const gchar *nouce, const gchar *userid,
        const gchar *password, const gchar *publickey, const gchar *aeskey_raw)
{
    gchar  *psdhex = hash_password_v4(userid, password);
    gchar   modulus[257];
    gchar   exponent[7];
    gint    ret, flen;
    BIGNUM *bnn, *bne;
    guchar *out;
    guchar *nonce, *aeskey, *psd, *res;
    gint    nonce_len, aeskey_len, psd_len;
    RSA    *r      = RSA_new();

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
    r->n = bnn;    r->e = bne;    r->d = NULL;

    RSA_print_fp(stdout, r, 5);
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
    gchar   *res;
    xmlnode *root    = xmlnode_root(root_raw, strlen(root_raw));

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
    xmlnode_new_prop(subnode, "version", ac->personal_version);
    xmlnode_new_prop(subnode, "attributes", "v4default");

    subnode = xmlnode_new_child(node, "custom-config");
    xmlnode_new_prop(subnode, "version", ac->custom_config_version);

    subnode = xmlnode_new_child(node, "contact-list");
    xmlnode_new_prop(subnode, "version", ac->contact_list_version);
    xmlnode_new_prop(subnode, "buddy-attributes", "v4default");

    node = xmlnode_new_child(root, "presence");
    subnode = xmlnode_new_child(node, "basic");
    xmlnode_new_prop(subnode, "value", "0");
    xmlnode_new_prop(subnode, "desc", "");

    res = xmlnode_to_string(root);

    xmlnode_free(root);

    return res;
}

/**
 * Get the contact list from the xmlnode with name 'contact-list',
 * note that this node can either be a child node of the sipc 
 * response xml message , or a child node of the local xml file.
 */
static void
get_contact_list(fetion_account *ac, xmlnode *contact_node)
{
    gchar        *temp;
    gchar        *temp1;
    xmlnode      *node;
    fetion_group *group;
    fetion_buddy *buddy;
    gboolean      has_ungroup = FALSE;

    g_return_if_fail(ac != NULL);
    g_return_if_fail(contact_node != NULL);

    /* group list */
    node = xmlnode_find(contact_node, "buddy-lists");
    node = xmlnode_child(node);

    while (node) {
        temp  = xmlnode_prop(node, "name");
        temp1 = xmlnode_prop(node, "id");

        group      = fetion_group_create(atoi(temp1), temp);
        ac->groups = g_slist_append(ac->groups, group);

        g_free(temp);
        g_free(temp1);

        node = node->next;
    }

    /* contact list  */
    node = xmlnode_find(contact_node, "buddies");
    node = xmlnode_child(node);

    while (node) {
        buddy            = fetion_buddy_create();
        buddy->userid    = xmlnode_prop(node, "i");
        buddy->sipuri    = xmlnode_prop(node, "u");
        buddy->localname = xmlnode_prop(node, "n");
        buddy->groups    = xmlnode_prop(node, "l");
        buddy->sid       = get_sid_from_sipuri(buddy->sipuri);

        if (xmlnode_has_prop(node, "r")) {

            temp = xmlnode_prop(node, "r");
            buddy->status = atoi(temp);
            g_free(temp);

        } else {
            buddy->status = 0;
        }

        ac->buddies = g_slist_append(ac->buddies, buddy);

        /* ungrouped */
        if (*(buddy->groups) == '\0' || buddy->groups[0] == '0') {
            g_free(buddy->groups);
            buddy->groups = g_strdup("0");

            if (!has_ungroup) { /**< add an "ungroup" group */
                group      = fetion_group_create(0, _("Ungrouped"));
                ac->groups = g_slist_append(ac->groups, group);

                has_ungroup = TRUE;
            }
        }

        node = node->next;
    }
}
/*
 * Get the personal information from the xmlnode with name 'personal',
 * note that this node can either be a child node of the sipc 
 * response xml message , or a child node of the local xml file.
 */
static void
get_personal(fetion_account *ac, xmlnode *node)
{
    gchar *pos;
    gchar *temp;
    gchar *stop;
    
    g_return_if_fail(ac != NULL);
    g_return_if_fail(node != NULL);

    ac->nickname     = xmlnode_prop(node, "nickname");
    ac->mood_phrase  = xmlnode_prop(node, "impresa");
    ac->portrait_crc = xmlnode_prop(node, "portrait-crc");
    temp             = xmlnode_prop(node, "carrier-region");

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
    gchar   *version;

    g_return_if_fail(ac != NULL);
    g_return_if_fail(body != NULL);
    g_return_if_fail(len != 0);

    root = xmlnode_root(body, len);

    /* login info */
    node                = xmlnode_find(root, "client");
    ac->last_login_ip   = xmlnode_prop(root, "last-login-ip");
    ac->public_ip       = xmlnode_prop(root, "public-ip");
    ac->last_login_time = xmlnode_prop(root, "last-login-time");

    /* personal info */
    node    = xmlnode_find(root, "personal");
    version = xmlnode_prop(node, "version");

    if (g_strcmp0(version, ac->personal_version) == 0) {
        /* load from disk. */
        g_free(version);

        xmlnode *personal_root;
        xmlnode *personal_node;

        if (!(personal_root = fetion_config_load_personal(ac))) {
            hybrid_debug_error("fetion", "invalid personal.xml");
            xmlnode_free(root);
            return;
        } 

        if (!(personal_node = xmlnode_find(personal_root, "personal"))) {
            hybrid_debug_error("fetion", "invalid personal.xml");
            xmlnode_free(personal_root);
            xmlnode_free(root);
            return;
        }
        get_personal(ac, personal_node);
        xmlnode_free(personal_root);
        
    } else {
        /* update the version */
        g_free(ac->personal_version);
        ac->personal_version = version;
        /* get the personal information */
        get_personal(ac, node);
        /* save the personal information */
        fetion_config_save_personal(ac, node);
    }


    /* contact list version */
    node    = xmlnode_find(root, "contact-list");
    version = xmlnode_prop(node, "version");
    
    if (g_strcmp0(version, ac->contact_list_version) == 0) { 
        /* load from disk. */
        g_free(version);

        xmlnode *contact_root;
        xmlnode *contact_node;

        if (!(contact_root = fetion_config_load_buddies(ac))) {
            hybrid_debug_error("fetion", "invalid buddies.xml");
            xmlnode_free(root);
            return;
        }

        if (!(contact_node = xmlnode_find(contact_root, "contact-list"))) {
            hybrid_debug_error("fetion", "invalid buddies.xml");
            xmlnode_free(contact_root);
            xmlnode_free(root);
            return;
        }

        get_contact_list(ac, contact_node);
        xmlnode_free(contact_root);

    } else {

        /* the cache is out-of-data, drop it. */
        hybrid_account_clear_buddy(ac->account);

        /* update the version */
        g_free(ac->contact_list_version);
        ac->contact_list_version = version;
        /* get the contact list */
        get_contact_list(ac, node);
        /* save the contact list */
        fetion_config_save_buddies(ac, node);
    }


    /* custom config */
    node = xmlnode_find(root, "custom-config");
    version = xmlnode_prop(node, "version");

    if (g_strcmp0(version, ac->custom_config_version) != 0) {
        g_free(ac->custom_config);
        g_free(ac->custom_config_version);
        ac->custom_config = xmlnode_content(node);
        ac->custom_config_version = version;
    }

    xmlnode_free(root);

    /*
     * OK, now we need to save the account's version information, so
     * next time we can use it to register to sipc server.
     */
    fetion_config_save_account(ac);
}

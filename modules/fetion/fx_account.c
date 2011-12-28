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

#include "fx_account.h"
#include "fx_buddy.h"

static gchar *generate_set_state_body(gint state);
static gchar *generate_keep_alive_body();
static gchar *generate_modify_name_body(fetion_account *account,
                    const gchar *name);
static gchar *generate_modify_status_body(fetion_account *account,
                    const gchar *status);

/**
 * List of existing channels, whose element is fetion_account.
 * An element should be removed when User-Left message received,
 * and the whole list should be destroyed when this fetion account
 * was closed. Only cloned fetion_account is stored in this list.
 */
GSList *channel_list = NULL;

fetion_account*
fetion_account_create(HybridAccount *account, const gchar *no, const gchar *password)
{
    fetion_account *ac;

    g_return_val_if_fail(no != NULL, NULL);
    g_return_val_if_fail(password != NULL, NULL);

    ac = g_new0(fetion_account, 1);

    ac->account = account;

    if (strlen(no) == 11) { /* mobile no */
        ac->mobileno = g_strdup(no);
    } else {
        ac->sid = g_strdup(no);
    }

    ac->channel_ready = TRUE;

    ac->sip      = fetion_sip_create(ac);
    ac->password = g_strdup(password);

    ac->cfg_server_version = g_strdup("0");
    ac->cfg_param_version  = g_strdup("0");
    ac->cfg_hint_version   = g_strdup("0");
    ac->cfg_client_version = g_strdup("2.0.0.0");

    ac->personal_version      = g_strdup("0");
    ac->contact_list_version  = g_strdup("0");
    ac->custom_config_version = g_strdup("0");

    return ac;
}

fetion_account*
fetion_account_clone(fetion_account *account)
{
    fetion_account *ac;

    g_return_val_if_fail(account != NULL, NULL);

    ac           = g_new0(fetion_account, 1);
    ac->account  = account->account;
    ac->buddies  = account->buddies;
    ac->groups   = account->groups;
    ac->sid      = g_strdup(account->sid);
    ac->mobileno = g_strdup(account->mobileno);
    ac->sipuri   = g_strdup(account->sipuri);

    channel_list = g_slist_append(channel_list, ac);

    fetion_sip_create(ac);

    ac->channel_ready = FALSE;

    return ac;
}

gint
fetion_account_update_state(fetion_account *ac, gint state)
{

    sip_header *eheader;
    fetion_sip *sip = ac->sip;
    gchar *body;
    gchar *res;

    fetion_sip_set_type(sip, SIP_SERVICE);

    eheader = sip_event_header_create(SIP_EVENT_SETPRESENCE);
    fetion_sip_add_header(sip, eheader);

    body = generate_set_state_body(state);
    res = fetion_sip_to_string(sip, body);
    g_free(body);

    hybrid_debug_info("fetion",
            "user state changed to %d,send:\n%s", state, res);

    if (send(ac->sk, res, strlen(res), 0) == -1) {
        g_free(res);

        return HYBRID_ERROR;
    }

    ac->state = state;

    g_free(res);

    return HYBRID_OK;
}

/**
 * Callback function to handle the response of the modify-info request,
 * we will get the new nickname,status,custom config string,etc.
 */
static gint
modify_info_cb(fetion_account *account, const gchar *sipmsg,
        fetion_transaction *trans)
{
    xmlnode *root;
    xmlnode *node;
    gchar   *value;
    gchar   *pos;

    hybrid_debug_info("fetion", "modify info,recv:\n%s", sipmsg);

    if (fetion_sip_get_code(sipmsg) != 200) {
        goto modify_info_err;
    }

    if (!(pos = strstr(sipmsg, "\r\n\r\n"))) {
        goto modify_info_err;
    }

    pos += 4;

    if (!(root = xmlnode_root(pos, strlen(pos)))) {
        goto modify_info_err;
    }

    if ((node = xmlnode_find(root, "personal"))) {

        if (xmlnode_has_prop(node, "impresa")) {

            value = xmlnode_prop(node, "impresa");
            fetion_account_set_mood(account, value);
            hybrid_account_set_status_text(account->account, value);
            g_free(value);
        }

        if (xmlnode_has_prop(node, "nickname")) {

            value = xmlnode_prop(node, "nickname");
            fetion_account_set_nickname(account, value);
            hybrid_account_set_nickname(account->account, value);
            g_free(value);
        }
    }

    if ((node = xmlnode_find(root, "custom-config"))) {

        value = xmlnode_content(node);
        fetion_account_set_custom_config(account, value);
        g_free(value);
    }

    return HYBRID_OK;

modify_info_err:
    hybrid_debug_error("fetion", "modify info error.");
    return HYBRID_ERROR;
}

gint
fetion_account_modify_name(fetion_account *account, const gchar *name)
{
    sip_header         *eheader;
    fetion_sip         *sip;
    gchar              *body;
    gchar              *sip_text;
    fetion_transaction *trans;

    g_return_val_if_fail(account != NULL, HYBRID_ERROR);
    g_return_val_if_fail(name != NULL, HYBRID_ERROR);

    sip = account->sip;

    fetion_sip_set_type(sip, SIP_SERVICE);

    eheader = sip_event_header_create(SIP_EVENT_SETUSERINFO);
    fetion_sip_add_header(sip, eheader);

    trans = transaction_create();
    transaction_set_callid(trans, sip->callid);
    transaction_set_callback(trans, modify_info_cb);
    transaction_add(account, trans);

    body = generate_modify_name_body(account, name);

    sip_text = fetion_sip_to_string(sip, body);

    g_free(body);

    hybrid_debug_info("fetion", "modify nickname,send:\n%s", sip_text);

    if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {
        g_free(sip_text);

        return HYBRID_ERROR;
    }

    g_free(sip_text);

    return HYBRID_OK;
}

gint
fetion_account_modify_status(fetion_account *account, const gchar *status)
{
    sip_header         *eheader;
    fetion_sip         *sip;
    gchar              *body;
    gchar              *sip_text;
    fetion_transaction *trans;

    g_return_val_if_fail(account != NULL, HYBRID_ERROR);
    g_return_val_if_fail(status != NULL, HYBRID_ERROR);

    sip = account->sip;

    fetion_sip_set_type(sip, SIP_SERVICE);

    eheader = sip_event_header_create(SIP_EVENT_SETUSERINFO);
    fetion_sip_add_header(sip, eheader);

    trans = transaction_create();
    transaction_set_callid(trans, sip->callid);
    transaction_set_callback(trans, modify_info_cb);
    transaction_add(account, trans);

    body = generate_modify_status_body(account, status);

    sip_text = fetion_sip_to_string(sip, body);

    g_free(body);

    hybrid_debug_info("fetion", "modify status,send:\n%s", sip_text);

    if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {
        g_free(sip_text);

        return HYBRID_ERROR;
    }

    g_free(sip_text);

    return HYBRID_OK;
}

gint
keep_alive_cb(fetion_account *ac, const gchar *sipmsg,
                fetion_transaction *trans)
{
    hybrid_debug_info("fetion", "keep alive, response:\n%s", sipmsg);

    return HYBRID_OK;
}

gint
fetion_account_keep_alive(fetion_account *ac)
{

    fetion_sip         *sip;
    sip_header         *eheader;
    gchar              *res;
    gchar              *body;
    fetion_transaction *trans;

    g_return_val_if_fail(ac != NULL, HYBRID_ERROR);

    sip = ac->sip;

    fetion_sip_set_type(sip, SIP_REGISTER);
    eheader = sip_event_header_create(SIP_EVENT_KEEPALIVE);
    fetion_sip_add_header(sip, eheader);

    trans = transaction_create();
    transaction_set_callid(trans, sip->callid);
    transaction_set_callback(trans, keep_alive_cb);
    transaction_add(ac, trans);

    body = generate_keep_alive_body();
    res = fetion_sip_to_string(sip , body);
    g_free(body);

    hybrid_debug_info("fetion", "keep alive,send:\n%s", res);

    if (send(ac->sk, res, strlen(res), 0) == -1) {
        g_free(res);
        return HYBRID_ERROR;
    }

    g_free(res);

    return HYBRID_OK;
}

void
fetion_account_destroy(fetion_account *ac)
{
    if (ac) {
        g_free(ac->sid);
        g_free(ac->mobileno);
        g_free(ac->password);
        g_free(ac->sipuri);
        g_free(ac->userid);
        g_free(ac->ssic);
        g_free(ac->nickname);
        g_free(ac->mood_phrase);
        g_free(ac->portrait_crc);
        g_free(ac->country);
        g_free(ac->province);
        g_free(ac->city);
        g_free(ac->sms_online_status);
        g_free(ac->last_login_ip);
        g_free(ac->last_login_time);
        g_free(ac->public_ip);
        g_free(ac->sipc_proxy_ip);
        g_free(ac->portrait_host_path);
        g_free(ac->portrait_host_name);
        g_free(ac->cfg_server_version);
        g_free(ac->cfg_param_version);
        g_free(ac->cfg_hint_version);
        g_free(ac->cfg_client_version);
        g_free(ac->personal_version);
        g_free(ac->contact_list_version);
        g_free(ac->custom_config_version);
        g_free(ac->custom_config);
        g_free(ac->ssic);

        g_free(ac->who);

        fetion_sip_destroy(ac->sip);
        g_free(ac);
    }
}

Verification*
fetion_verification_create()
{
    return g_new0(Verification, 1);
}

void
fetion_verification_destroy(Verification *verification)
{
    if (verification) {
        g_free(verification->desc);
        g_free(verification->text);
        g_free(verification->algorithm);
        g_free(verification->type);
        g_free(verification->tips);
    }
}

gint
fetion_account_update_portrait(fetion_account *ac)
{
    portrait_data *data;
    const gchar   *checksum;

    g_return_val_if_fail(ac != NULL, HYBRID_ERROR);

    data                = g_new0(portrait_data, 1);
    data->ac            = ac;
    data->portrait_type = PORTRAIT_TYPE_ACCOUNT;

    checksum = hybrid_account_get_checksum(ac->account);

    if (checksum != NULL && g_strcmp0(checksum, ac->portrait_crc) == 0) {
        return HYBRID_OK;
    }

    hybrid_proxy_connect(ac->portrait_host_name, 80, portrait_conn_cb, data);

    return HYBRID_OK;
}

static gchar*
generate_set_state_body(gint state)
{
    xmlnode *root;
    xmlnode *node;
    gchar   *s;
    gchar data[] = "<args></args>";

    root = xmlnode_root(data, strlen(data));
    node = xmlnode_new_child(root, "presence");
    node = xmlnode_new_child(node, "basic");

    s = g_strdup_printf("%d", state);
    xmlnode_new_prop(node, "value", s);
    g_free(s);

    s = xmlnode_to_string(root);

    xmlnode_free(root);

    return s;
}


static gchar*
generate_keep_alive_body()
{
    xmlnode *root;
    xmlnode *node;
    gchar *body;
    gchar data[] = "<args></args>";

    root = xmlnode_root(data, strlen(data));
    node = xmlnode_new_child(root, "credentials");
    xmlnode_new_prop(node, "domains", "fetion.com.cn");

    body = xmlnode_to_string(root);

    xmlnode_free(root);

    return body;
}

static gchar*
generate_modify_name_body(fetion_account *account, const gchar *name)
{
    xmlnode *root;
    xmlnode *cnode;
    xmlnode *node;
    gchar   *xml_text;

    root = xmlnode_create("args");

    cnode = xmlnode_new_child(root, "userinfo");
    node  = xmlnode_new_child(cnode, "personal");
    xmlnode_new_prop(node, "nickname", name);
    xmlnode_new_prop(node, "version", account->personal_version);
    node = xmlnode_new_child(cnode, "custom-config");
    xmlnode_new_prop(node, "type", "PC");
    xmlnode_new_prop(node, "version", account->custom_config_version);
    xmlnode_set_content(node, account->custom_config);

    xml_text = xmlnode_to_string(root);

    xmlnode_free(root);

    return xml_text;
}

static gchar*
generate_modify_status_body(fetion_account *account, const gchar *status)
{
    xmlnode *root;
    xmlnode *cnode;
    xmlnode *node;
    gchar   *xml_text;

    root = xmlnode_create("args");

    cnode = xmlnode_new_child(root, "userinfo");
    node = xmlnode_new_child(cnode, "personal");
    xmlnode_new_prop(node, "impresa", status);
    xmlnode_new_prop(node, "version", account->personal_version);
    node = xmlnode_new_child(cnode, "custom-config");
    xmlnode_new_prop(node, "type", "PC");
    xmlnode_new_prop(node, "version", account->custom_config_version);
    xmlnode_set_content(node, account->custom_config);

    xml_text = xmlnode_to_string(root);

    xmlnode_free(root);

    return xml_text;
}

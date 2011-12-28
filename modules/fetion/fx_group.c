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

#include "util.h"
#include "blist.h"

#include "fx_trans.h"
#include "fx_group.h"

static gchar *generate_group_edit_body(const gchar *group_id, const gchar *group_name);
static gchar *generate_group_add_body(const gchar *name);
static gchar *generate_group_remove_body(const gchar *groupid);

fetion_group*
fetion_group_create(gint id, const gchar *name)
{
    fetion_group *group;

    g_return_val_if_fail(name != NULL, NULL);

    group = g_new0(fetion_group, 1);
    group->group_id = id;
    group->group_name = g_strdup(name);

    return group;
}

void
fetion_group_destroy(fetion_group *group)
{
    if (group) {
        g_free(group->group_name);
        g_free(group);
    }
}

static gint
group_edit_cb(fetion_account *ac, const gchar *sipmsg,
            fetion_transaction *trans)
{
    hybrid_debug_info("fetion", "edit group, recv:\n%s", sipmsg);

    return HYBRID_OK;
}

gint
fetion_group_edit(fetion_account *account, const gchar *id,
                        const gchar *name)
{
    fetion_sip *sip;
    sip_header *eheader;
    fetion_transaction *trans;
    gchar *body;
    gchar *sip_text;

    g_return_val_if_fail(account != NULL, HYBRID_ERROR);
    g_return_val_if_fail(id != NULL, HYBRID_ERROR);
    g_return_val_if_fail(name != NULL, HYBRID_ERROR);

    sip = account->sip;

    fetion_sip_set_type(sip, SIP_SERVICE);

    eheader = sip_event_header_create(SIP_EVENT_SETBUDDYLISTINFO);
    fetion_sip_add_header(sip, eheader);

    trans = transaction_create();
    transaction_set_callid(trans, sip->callid);
    transaction_set_callback(trans, group_edit_cb);
    transaction_add(account, trans);

    body = generate_group_edit_body(id, name);
    sip_text = fetion_sip_to_string(sip, body);
    g_free(body);

    hybrid_debug_info("fetion", "rename group,send\n%s", sip_text);

    if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {

        hybrid_debug_info("fetion", "rename group failed");

        return HYBRID_ERROR;
    }

    g_free(sip_text);

    return HYBRID_OK;
}

static gint
group_remove_cb(fetion_account *ac, const gchar *sipmsg,
                fetion_transaction *trans)
{
    hybrid_debug_info("fetion", "remove buddy, recv:\n%s", sipmsg);

    return HYBRID_OK;
}

gint
fetion_group_remove(fetion_account *account, const gchar *groupid)
{
    fetion_sip *sip;
    sip_header *eheader;
    gchar *body;
    gchar *sip_text;
    fetion_transaction *trans;

    g_return_val_if_fail(account != NULL, HYBRID_ERROR);
    g_return_val_if_fail(groupid != NULL, HYBRID_ERROR);

    sip = account->sip;

    fetion_sip_set_type(sip, SIP_SERVICE);

    eheader = sip_event_header_create(SIP_EVENT_DELETEBUDDYLIST);
    fetion_sip_add_header(sip, eheader);

    trans = transaction_create();
    transaction_set_callid(trans, sip->callid);
    transaction_set_callback(trans, group_remove_cb);
    transaction_add(account, trans);

    body = generate_group_remove_body(groupid);
    sip_text = fetion_sip_to_string(sip, body);
    g_free(body);

    hybrid_debug_info("fetion", "remove group, send:\n%s", sip_text);

    if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {

        hybrid_debug_info("fetion", "remove group error");

        return HYBRID_ERROR;
    }

    g_free(sip_text);

    return HYBRID_OK;
}

static gint
group_add_cb(fetion_account *account, const gchar *sipmsg,
            fetion_transaction *trans)
{
    gint code;
    gchar *pos;
    xmlnode *root;
    xmlnode *node;
    gchar *group_name;
    gchar *group_id;

    hybrid_debug_info("fetion", "group add, recv:\n%s", sipmsg);

    if ((code = fetion_sip_get_code(sipmsg)) != 200) {
        goto group_add_error;
    }

    if (!(pos = strstr(sipmsg, "\r\n\r\n"))) {
        goto group_add_error;
    }

    pos += 4;

    if (!(root = xmlnode_root(pos, strlen(pos)))) {
        goto group_add_error;
    }

    if (!(node = xmlnode_find(root, "buddy-list"))) {
        goto group_add_error;
    }

    if (!xmlnode_has_prop(node, "id") || !xmlnode_has_prop(node, "name")) {
        goto group_add_error;
    }

    group_id = xmlnode_prop(node, "id");
    group_name = xmlnode_prop(node, "name");

    hybrid_blist_add_group(account->account, group_id, group_name);

    return HYBRID_OK;

group_add_error:

    hybrid_debug_error("fetion", "group add error: %d", code);

    /* TODO popup warning box. */

    return HYBRID_ERROR;
}

gint
fetion_group_add(fetion_account *account, const gchar *name)
{
    fetion_sip *sip;
    sip_header *eheader;
    gchar *body;
    gchar *sip_text;
    fetion_transaction *trans;

    g_return_val_if_fail(account != NULL, HYBRID_ERROR);
    g_return_val_if_fail(name != NULL, HYBRID_ERROR);

    sip = account->sip;

    fetion_sip_set_type(sip, SIP_SERVICE);

    eheader = sip_event_header_create(SIP_EVENT_CREATEBUDDYLIST);
    fetion_sip_add_header(sip, eheader);

    trans = transaction_create();
    transaction_set_callid(trans, sip->callid);
    transaction_set_callback(trans, group_add_cb);
    transaction_add(account, trans);

    body = generate_group_add_body(name);
    sip_text = fetion_sip_to_string(sip , body);
    g_free(body);

    hybrid_debug_info("fetion", "add group, send:\n%s", sip_text);

    if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {

        hybrid_debug_error("fetion", "add group failed");

        return HYBRID_ERROR;
    }

    g_free(sip_text);

    return HYBRID_OK;
}

void
fetion_groups_init(fetion_account *ac)
{
    GSList *pos;
    HybridGroup *hgroup;
    fetion_group *group;
    gchar buf[BUF_LENGTH];

    for (pos = ac->groups; pos; pos = pos->next) {

        group = (fetion_group*)pos->data;

        g_snprintf(buf, sizeof(buf) - 1, "%d", group->group_id);
        hgroup = hybrid_blist_add_group(ac->account, buf, group->group_name);

        /* The group named 'Ungrouped' can't be renamed. */
        if (group->group_id == 0) {
            hybrid_blist_set_group_renamable(hgroup, FALSE);
        }
    }
}

static gchar*
generate_group_edit_body(const gchar *group_id, const gchar *group_name)
{
    const gchar *body;
    xmlnode *root;
    xmlnode *node;
    gchar *res;

    body = "<args></args>";

    root = xmlnode_root(body, strlen(body));

    node = xmlnode_new_child(root, "contacts");
    node = xmlnode_new_child(node, "buddy-lists");
    node = xmlnode_new_child(node, "buddy-list");

    xmlnode_new_prop(node, "id", group_id);
    xmlnode_new_prop(node, "name", group_name);

    res = xmlnode_to_string(root);

    xmlnode_free(root);

    return res;
}

static gchar*
generate_group_add_body(const gchar *name)
{
    const gchar *body;
    xmlnode *root;
    xmlnode *node;
    gchar *res;

    body = "<args></args>";

    root = xmlnode_root(body, strlen(body));

    node = xmlnode_new_child(root, "contacts");
    node = xmlnode_new_child(node, "buddy-lists");
    node = xmlnode_new_child(node, "buddy-list");

    xmlnode_new_prop(node, "name", name);

    res = xmlnode_to_string(root);

    xmlnode_free(root);

    return res;
}

static gchar*
generate_group_remove_body(const gchar *groupid)
{
    const gchar *body;
    xmlnode *root;
    xmlnode *node;
    gchar *res;

    body = "<args></args>";

    root = xmlnode_root(body, strlen(body));

    node = xmlnode_new_child(root, "contacts");
    node = xmlnode_new_child(node, "buddy-lists");
    node = xmlnode_new_child(node, "buddy-list");

    xmlnode_new_prop(node, "id", groupid);

    res = xmlnode_to_string(root);

    xmlnode_free(root);

    return res;
}

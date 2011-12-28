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

#include "fx_trans.h"

fetion_transaction*
transaction_create()
{
    fetion_transaction *trans;

    trans = g_new0(fetion_transaction, 1);

    return trans;
}

fetion_transaction*
transaction_clone(fetion_transaction *trans)
{
    fetion_transaction *new_trans;

    g_return_val_if_fail(trans != NULL, NULL);

    new_trans = g_new0(fetion_transaction, 1);

    transaction_set_userid(new_trans, trans->userid);
    transaction_set_msg(new_trans, trans->msg);
    new_trans->data = trans->data;

    return new_trans;
}

void
transaction_destroy(fetion_transaction *trans)
{
    if (trans) {
        g_free(trans->userid);
        g_free(trans->msg);
        g_free(trans);
    }
}

void
transaction_set_callid(fetion_transaction *trans, gint callid)
{
    trans->callid = callid;
}

void
transaction_set_userid(fetion_transaction *trans, const gchar *userid)
{
    g_return_if_fail(trans != NULL);

    g_free(trans->userid);
    trans->userid = g_strdup(userid);
}

void
transaction_set_msg(fetion_transaction *trans, const gchar *msg)
{
    g_return_if_fail(trans != NULL);

    g_free(trans->msg);
    trans->msg = g_strdup(msg);
}

void
transaction_set_callback(fetion_transaction *trans,    TransCallback callback)
{
    trans->callback = callback;
}

void
transaction_set_data(fetion_transaction *trans, gpointer data)
{
    trans->data = data;
}

void
transaction_set_timeout(fetion_transaction *trans, GSourceFunc timeout_cb,
                        gpointer user_data)
{
    trans->timer = g_timeout_add_seconds(30, timeout_cb, user_data);
}

void
transaction_add(fetion_account *account, fetion_transaction *trans)
{
    account->trans_list = g_slist_append(account->trans_list, trans);
}

void
transaction_remove(fetion_account *account, fetion_transaction *trans)
{
    account->trans_list = g_slist_remove(account->trans_list, trans);
    transaction_destroy(trans);
}

void
transaction_wait(fetion_account *account, fetion_transaction *trans)
{
    account->trans_wait_list = g_slist_append(account->trans_wait_list, trans);
}

void
transaction_wakeup(fetion_account *account, fetion_transaction *trans)
{
    account->trans_wait_list = g_slist_remove(account->trans_wait_list, trans);
}

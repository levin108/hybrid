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

#ifndef HYBRID_FX_TRANS_H
#define HYBRID_FX_TRANS_H
#include <glib.h>
#include "fx_account.h"

typedef struct transaction fetion_transaction;

typedef gint (*TransCallback)(fetion_account *,
				  const gchar *, fetion_transaction *);

struct transaction {
	gint callid;
	gchar *userid;
	gchar *msg;
	gpointer data;
	guint timer;
	TransCallback callback;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The transaction context is to handle the sip response, when we
 * start a new sip request, we create a new transaction, and
 * push it into the account's transaction queue, when a response
 * comes, we iterate the queue to find the transaction to whom the
 * response belongs, and then use the transaction to process the response
 * sipmsg, note that there's an callback function in the transaction,
 * we just use it to process the response message. But how we get out
 * the right transaction to handle the specified response sipmsg, yes,
 * we get it by the call id attribute.
 *
 * @return The transaction created.
 */
fetion_transaction *transaction_create();

/**
 * Clone an existing transaction.
 *
 * @param trans The transaction to be cloned.
 *
 * @return The transaction cloned.
 */
fetion_transaction *transaction_clone(fetion_transaction *trans);

/**
 * Destroy an existing transaction.
 *
 * @param trans The transaction to destroy.
 */
void transaction_destroy(fetion_transaction *trans);

/**
 * Set the callid of the transaction.
 *
 * @param trans  The transaction.
 * @param callid The callid of the sip request message.
 */
void transaction_set_callid(fetion_transaction *trans, gint callid);

/**
 * Set the user id of the transaction.
 *
 * @param trans The transaction.
 * @param userid The user id.
 */
void transaction_set_userid(fetion_transaction *trans, const gchar *userid);

/**
 * Set the sip message of the transaction.
 *
 * @param trans The transaction.
 * @param sipmsg The response sip message for this transaction.
 */
void transaction_set_msg(fetion_transaction *trans, const gchar *msg);


/**
 * Set the callback function of the transaction.
 *
 * @param trans    The transaction.
 * @param callback The callback function to handle the response sip message.
 */
void transaction_set_callback(fetion_transaction *trans,
		TransCallback callback);

/**
 * Set a user-specified data for the callback function.
 *
 * @param trans The transaction.
 * @param data  The user-specified data.
 */
void transaction_set_data(fetion_transaction *trans, gpointer data);

/**
 * Set the timeout callback function.
 *
 * @param trans      The transaction.
 * @param timeout_cb The callback function of the timeout event.
 * @param user_data  User-specified data for callback function.
 */
void transaction_set_timeout(fetion_transaction *trans,
					GSourceFunc timeout_cb, gpointer user_data);

/**
 * Add the given transaction to the account's pending transaction list.
 *
 * @param account The account.
 * @param trans   The transaction.
 */
void transaction_add(fetion_account *account, fetion_transaction *trans);

/**
 * Remove the given transaction from the account's pending transacion list,
 * and destroy it, so you don't need to destroy it again.
 *
 * @param account The fetion account context.
 * @param trans    The transaction to remove.
 */
void transaction_remove(fetion_account *account, fetion_transaction *trans);

/**
 * Make the transaction wait to be processed until the channel is ready,
 * in this function we just put the transaction into the transaction waiting list.
 *
 * @param account The fetion account context.
 * @param trans   The transaction to wait;
 */
void transaction_wait(fetion_account *account, fetion_transaction *trans);

/**
 * Wake up the transaction to be processed, in this function we just remove
 * the transaction from the waiting list.
 *
 * @param account The fetion account context.
 * @param trans   The transaction to wakeup;
 */
void transaction_wakeup(fetion_account *account, fetion_transaction *trans);
#ifdef __cplusplus
}
#endif

#endif /* HYBRID_FX_TRANS_H */

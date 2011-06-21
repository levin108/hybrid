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
	gchar *sipmsg;
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
void transaction_set_sipmsg(fetion_transaction *trans, const gchar *sipmsg);


/**
 * Set the callback function of the transaction.
 *
 * @param trans    The transaction.
 * @param callback The callback function to handle the response sip message.
 */
void transaction_set_callback(fetion_transaction *trans,
		TransCallback callback);

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
 * @param tran    The transaction to remove.
 */
void transaction_remove(fetion_account *account, fetion_transaction *trans);
#ifdef __cplusplus
}
#endif

#endif /* HYBRID_FX_TRANS_H */

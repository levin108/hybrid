#include "fx_trans.h"

fetion_transaction*
transaction_create()
{
	fetion_transaction *trans;

	trans = g_new0(fetion_transaction, 1);
	
	return trans;
}

void
transaction_destroy(fetion_transaction *trans)
{
	if (trans) {
		g_free(trans->userid);
		g_free(trans->sipmsg);
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
transaction_set_sipmsg(fetion_transaction *trans, const gchar *sipmsg)
{
	g_return_if_fail(trans != NULL);

	g_free(trans->sipmsg);
	trans->sipmsg = g_strdup(sipmsg);
}

void
transaction_set_callback(fetion_transaction *trans,	TransCallback callback)
{
	trans->callback = callback;
}

void
transaction_set_data(fetion_transaction *trans, gpointer data)
{
	trans->data = data;
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

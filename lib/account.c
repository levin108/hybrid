#include <glib.h>
#include "account.h"

IMAccount*
im_account_create(IMModule *proto)
{
	g_return_val_if_fail(proto != NULL, NULL);

	IMAccount *ac = g_new0(IMAccount, 1);
	im_account_set_username(ac, "547264589");
	im_account_set_password(ac, "lwp1279");
	
	ac->proto = proto;

	return ac;
}

void
im_account_destroy(IMAccount *account)
{
	if (account) {
		g_free(account->username);
		g_free(account->password);
		g_free(account);
	}
}

void 
im_account_set_username(IMAccount *account, const gchar *username)
{
	g_return_if_fail(account != NULL);

	g_free(account->username);

	account->username = g_strdup(username);
}

void 
im_account_set_password(IMAccount *account, const gchar *password)
{
	g_return_if_fail(account != NULL);

	g_free(account->password);

	account->password = g_strdup(password);
}

void
im_account_error_reason(IMAccount *account, const gchar *reason)
{
	/* TODO */
	g_return_if_fail(account != NULL);

	im_account_destroy(account);
}

#include <glib.h>
#include "account.h"

HybirdAccount*
hybird_account_create(HybirdModule *proto)
{
	extern HybirdConfig *global_config;
	g_return_val_if_fail(proto != NULL, NULL);

	HybirdAccount *ac = g_new0(HybirdAccount, 1);
	hybird_account_set_username(ac, "547264589");
	hybird_account_set_password(ac, "lwp1279");
	
	ac->config = global_config;
	ac->proto = proto;

	return ac;
}

void
hybird_account_destroy(HybirdAccount *account)
{
	if (account) {
		g_free(account->username);
		g_free(account->password);
		g_free(account);
	}
}

void 
hybird_account_set_username(HybirdAccount *account, const gchar *username)
{
	g_return_if_fail(account != NULL);

	g_free(account->username);

	account->username = g_strdup(username);
}

void 
hybird_account_set_password(HybirdAccount *account, const gchar *password)
{
	g_return_if_fail(account != NULL);

	g_free(account->password);

	account->password = g_strdup(password);
}

void
hybird_account_error_reason(HybirdAccount *account, const gchar *reason)
{
	/* TODO */
	g_return_if_fail(account != NULL);

	hybird_account_destroy(account);
}

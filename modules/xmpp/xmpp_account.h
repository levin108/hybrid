#ifndef HYBRID_XMPP_ACCOUNT_H
#define HYBRID_XMPP_ACCOUNT_H
#include <glib.h>

#include "account.h"

typedef struct _XmppAccount XmppAccount;

struct _XmppAccount {
	gchar *username;
	gchar *password;

	gchar *to;

	HybridAccount *account;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a new xmpp account context.
 *
 * @param account  The hybrid account.
 * @param username Username of the account.
 * @param password Password of the account.
 * @param to       The server domain.
 *
 * @return The account created.
 */
XmppAccount *xmpp_account_create(HybridAccount *account, const gchar *username,
					const gchar *password, const gchar *to);

/**
 * Destroy an existing account.
 *
 * @param account The account to destroy.
 */
void xmpp_account_destroy(XmppAccount *account);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_XMPP_ACCOUNT_H */

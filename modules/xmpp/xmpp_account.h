#ifndef HYBRID_XMPP_ACCOUNT_H
#define HYBRID_XMPP_ACCOUNT_H
#include <glib.h>

#include "account.h"

typedef struct _XmppAccount XmppAccount;

#include "xmpp_stream.h"

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
 * Modify name for the account.
 *
 * @param stream  The stream for the account to modify.
 * @param name    The new name string.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_account_modify_name(XmppStream *stream, const gchar *name);

/**
 * Modify status for the account.
 *
 * @param stream  The stream for the account to modify.
 * @param state   The presence state.
 * @param status  The new status string.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_account_modify_status(XmppStream *stream, gint state,
		const gchar *status);

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

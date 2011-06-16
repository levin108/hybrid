#ifndef HYBIRD_ACCOUNT_H
#define HYBIRD_ACCOUNT_H

#include <glib.h>

typedef struct _HybirdAccount HybirdAccount;

#include "config.h"
#include "module.h"

struct _HybirdAccount {
	gchar *username;
	gchar *password;
	gint   state;    /**< online status */

	HybirdConfig *config;
	HybirdModule *proto;
};

enum {
	HYBIRD_STATE_INVISIBLE = 0,
	HYBIRD_STATE_OFFLINE,
	HYBIRD_STATE_BUSY,
	HYBIRD_STATE_AWAY,
	HYBIRD_STATE_ONLINE
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a new Hybird account with the specified protocol module.
 *
 * @param proto Protocol Module that the account belongs to.
 *
 * @return Account created. need to be destroyed after use.
 */
HybirdAccount *hybird_account_create(HybirdModule *proto);

/**
 * Destroy an account.
 *
 * @param account The account to destroy.
 */
void hybird_account_destroy(HybirdAccount *account);

/**
 * Set the account's username.
 *
 * @param account The account.
 * @param username The username.
 */
void hybird_account_set_username(HybirdAccount *account, const gchar *username);

/**
 * Set the account's password.
 *
 * @param account The account.
 * @param password The password.
 */
void hybird_account_set_password(HybirdAccount *account, const gchar *password);

/**
 * Set the account's state.
 *
 * @param account The account.
 * @param state The state.
 */
void hybird_account_set_state(HybirdAccount *account, gint state);

/**
 * Close an account and give an error notification.
 * 
 * @param account The account to close.
 * @param reason The error reason message.
 */
void hybird_account_error_reason(HybirdAccount *account, const gchar *reason);

#ifdef __cplusplus
}
#endif

#endif

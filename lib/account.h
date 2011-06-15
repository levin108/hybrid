#ifndef IM_ACCOUNT_H
#define IM_ACCOUNT_H

#include <glib.h>

typedef struct _IMAccount IMAccount;

#include "module.h"

struct _IMAccount {
	gchar *username;
	gchar *password;
	gint   state;    /**< online status */

	IMModule *proto;
};

enum {
	IM_STATE_INVISIBLE = 0,
	IM_STATE_OFFLINE,
	IM_STATE_BUSY,
	IM_STATE_AWAY,
	IM_STATE_ONLINE
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a new IM account with the specified protocol module.
 *
 * @param proto Protocol Module that the account belongs to.
 *
 * @return Account created. need to be destroyed after use.
 */
IMAccount *im_account_create(IMModule *proto);

/**
 * Destroy an account.
 *
 * @param account The account to destroy.
 */
void im_account_destroy(IMAccount *account);

/**
 * Set the account's username.
 *
 * @param account The account.
 * @param username The username.
 */
void im_account_set_username(IMAccount *account, const gchar *username);

/**
 * Set the account's password.
 *
 * @param account The account.
 * @param password The password.
 */
void im_account_set_password(IMAccount *account, const gchar *password);

/**
 * Set the account's state.
 *
 * @param account The account.
 * @param state The state.
 */
void im_account_set_state(IMAccount *account, gint state);

/**
 * Close an account and give an error notification.
 * 
 * @param account The account to close.
 * @param reason The error reason message.
 */
void im_account_error_reason(IMAccount *account, const gchar *reason);

#ifdef __cplusplus
}
#endif

#endif

#ifndef HYBIRD_ACCOUNT_H
#define HYBIRD_ACCOUNT_H

#include <glib.h>

typedef struct _HybirdAccount HybirdAccount;
typedef enum _HybirdConnectionStatusType HybirdConnectionStatusType;

#include "config.h"
#include "module.h"

struct _HybirdAccount {
	gchar *username;
	gchar *password;
	gint   state;    /**< online status. */
	gint   connect_state; /**< connection status. */

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

enum _HybirdConnectionStatusType {
	HYBIRD_CONNECTION_CONNECTING,
	HYBIRD_CONNECTION_CONNECTED,
	HYBIRD_CONNECTION_CLOSED
};

#define HYBIRD_IS_CONNECTING(h) ((h)->connect_state == HYBIRD_CONNECTION_CONNECTING)
#define HYBIRD_IS_CONNECTED(h)  ((h)->connect_state == HYBIRD_CONNECTION_CONNECTED)
#define HYBIRD_IS_CLOSED(h)     ((h)->connect_state == HYBIRD_CONNECTION_CLOSED)


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the account context. Load the existing account.
 */
void hybird_account_init(void);

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

/**
 * Set the connection status. If the status was changed to CONNECTED,
 * then the local buddy list stored on the disk would be loaded. Make sure
 * to set the status to CONNECTED after logining successfully, orelse you can
 * not add buddies using hybird_blist_add_buddy().
 *
 * @param The account.
 * @param status The new connection status.
 */
void hybird_account_set_connection_status(HybirdAccount *account,
		HybirdConnectionStatusType status);

#ifdef __cplusplus
}
#endif

#endif

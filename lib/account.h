#ifndef HYBRID_ACCOUNT_H
#define HYBRID_ACCOUNT_H

#include <glib.h>

typedef struct _HybridAccount HybridAccount;
typedef enum _HybridConnectionStatusType HybridConnectionStatusType;

#include "util.h"
#include "config.h"
#include "module.h"

struct _HybridAccount {
	gchar *username;
	gchar *password;
	gchar *nickname;
	gint   state;    /**< online status. */
	gint   connect_state; /**< connection status. */
	gint   keep_alive_source; /**< source id of the keep alive. */

	guchar *icon_data; /**< binary data of the icon. */
	gint icon_data_len; /**< size of the binary data. */
	gchar *icon_crc; /**< checksum of the icon */
	gchar *icon_name; /**< file name of the local file. */

	GtkWidget *account_menu;

	gpointer protocol_data;

	GSList *action_list; /* list of action menus. */

	GHashTable *buddy_list;
	GHashTable *group_list;

	HybridConfig *config;
	HybridModule *proto;
};

enum {
	HYBRID_STATE_OFFLINE = 0,
	HYBRID_STATE_INVISIBLE,
	HYBRID_STATE_BUSY,
	HYBRID_STATE_AWAY,
	HYBRID_STATE_ONLINE
};

enum _HybridConnectionStatusType {
	HYBRID_CONNECTION_CONNECTING,
	HYBRID_CONNECTION_CONNECTED,
	HYBRID_CONNECTION_CLOSED
};

#define HYBRID_IS_CONNECTING(h) ((h)->connect_state == HYBRID_CONNECTION_CONNECTING)
#define HYBRID_IS_CONNECTED(h)  ((h)->connect_state == HYBRID_CONNECTION_CONNECTED)
#define HYBRID_IS_CLOSED(h)     ((h)->connect_state == HYBRID_CONNECTION_CLOSED)


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the account context. Load the existing account.
 */
void hybrid_account_init(void);

/**
 * Get an account from the account list, if there's
 * no matching node found, create one in the memory, 
 * and append it to the account count.
 *
 * @param proto_name The name of the protocol.
 * @param username   The username of the account.
 *
 * @return The Account found or created.
 */
HybridAccount *hybrid_account_get(const gchar *proto_name,
		const gchar *username);

/**
 * Synchronize the account information in the memory with 
 * that in the local cache file which is in fact a XML file.
 * The function first find the matching 'account' node, if found,
 * update it, orelse create a new one.
 *
 * @param account The account to update.
 */
void hybrid_account_update(HybridAccount *account);

/**
 * If account with the given protoname and username exists, remove
 * the account from the account list and remove the revelent node
 * in accounts.xml, if it doesn't exist, do nothing.
 *
 * @param protoname The name of the protocol.
 * @param username   The username of the account to remove.
 */
void hybrid_account_remove(const gchar *protoname, const gchar *username);

/**
 * Create a new Hybrid account with the specified protocol module.
 *
 * @param proto Protocol Module that the account belongs to.
 *
 * @return Account created. need to be destroyed after use.
 */
HybridAccount *hybrid_account_create(HybridModule *proto);

/**
 * Destroy an account.
 *
 * @param account The account to destroy.
 */
void hybrid_account_destroy(HybridAccount *account);

/**
 * Get the account's protocol specified data.
 *
 * @param account The account.
 *
 * @return The protocol specified data of this account.
 */
gpointer hybrid_account_get_protocol_data(HybridAccount *account);

/**
 * Set the account's protocol specified data.
 *
 * @param account       The account.
 * @param protocol_data The procotol data.
 */
void hybrid_account_set_protocol_data(HybridAccount *account,
		gpointer protocol_data);

/**
 * Set the account's username.
 *
 * @param account  The account.
 * @param username The username.
 */
void hybrid_account_set_username(HybridAccount *account, const gchar *username);

/**
 * Set the account's password.
 *
 * @param account  The account.
 * @param password The password.
 */
void hybrid_account_set_password(HybridAccount *account, const gchar *password);

/**
 * Set the account's state.
 *
 * @param account The account.
 * @param state The state.
 */
void hybrid_account_set_state(HybridAccount *account, gint state);

/**
 * Set the account's nickname.
 *
 * @param account  The account.
 * @param nickname The nickname.
 */
void hybrid_account_set_nickname(HybridAccount *account, const gchar *nickname);

/**
 * Get the checksum of the account's icon.
 * 
 * @param account The account.
 *
 * @return The checksum.
 */
const gchar* hybrid_account_get_checksum(HybridAccount *account);

/**
 * Set the account's icon.
 *
 * @param account   The account.
 * @param icon_data     The binary data of the account's icon.
 * @param icon_data_len The size of the binary data.
 * @param icon_crc      The checksum of the icon.
 */
void hybrid_account_set_icon(HybridAccount *account, const guchar *icon_data,
		gint icon_data_len, const gchar *icon_crc);

/**
 * Close an account. Remove it from the blist panel. Free the 
 * memory of the buddis and groups. But dont free the memory of 
 * the account. The call the protocol close callback function.
 *
 * @param account The account the close.
 */
void hybrid_account_close(HybridAccount *account);

/**
 * Close an account and give an error notification.
 * 
 * @param account The account to close.
 * @param reason The error reason message.
 */
void hybrid_account_error_reason(HybridAccount *account, const gchar *reason);

/**
 * Set the connection status. If the status was changed to CONNECTED,
 * then the local buddy list stored on the disk would be loaded. Make sure
 * to set the status to CONNECTED after logining successfully, orelse you can
 * not add buddies using hybrid_blist_add_buddy().
 *
 * @param The account.
 * @param status The new connection status.
 */
void hybrid_account_set_connection_status(HybridAccount *account,
		HybridConnectionStatusType status);

/**
 * Get the human readable name of the given presence state.
 *
 * @return The name of the presence state.
 */
const gchar *hybrid_get_presence_name(gint presence_state);

#ifdef __cplusplus
}
#endif

#endif

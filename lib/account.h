/***************************************************************************
 *   Copyright (C) 2011 by levin                                           *
 *   levin108@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 ***************************************************************************/

#ifndef HYBRID_ACCOUNT_H
#define HYBRID_ACCOUNT_H

#include <glib.h>

typedef struct _HybridAccountVariable    HybridAccountVariable;
typedef struct _HybridAccount            HybridAccount;
typedef enum _HybridConnectionStatusType HybridConnectionStatusType;
typedef enum _HybridAccountVariableType  HybridAccountVariableType;

#include "util.h"
#include "config.h"
#include "module.h"

struct _HybridAccount {
    gchar *username;
    gchar *password;
    gchar *nickname;
    gchar *status_text;
    gint   state;               /**< online status. */
    gint   connect_state;       /**< connection status. */
    gint   keep_alive_source;   /**< source id of the keep alive. */

    gint enabled;               /**< whether the account is enabled. */

    guchar *icon_data;          /**< binary data of the icon. */
    gint    icon_data_len;      /**< size of the binary data. */
    gchar  *icon_crc;           /**< checksum of the icon */
    gchar  *icon_name;          /**< file name of the local file. */

    GtkWidget   *account_menu;
    GtkWidget   *enable_menu;
    GtkWidget   *enable_menu_id;
    GtkWidget   *change_state_menu;
    GtkWidget   *login_panel;
    GtkWidget   *login_tips;
    GtkTreeIter  login_iter;

    gpointer protocol_data;

    GSList *action_list;        /* list of action menus. */
    GSList *option_list;        /* list of login options. */

    GHashTable *buddy_list;
    GHashTable *group_list;

    HybridConfig *config;
    HybridModule *proto;
};

enum _HybridAccountVariableType {
	VARIABLE_TYPE_STRING,
	VARIABLE_TYPE_BOOLEAN,
	VARIABLE_TYPE_INTEGER,
};

struct _HybridAccountVariable {
	HybridAccountVariableType  type;
	gchar					  *title;
	gchar					  *name;
	gchar					  *str_value;
	gboolean				   bool_value;
	gint					   int_value;
	GtkWidget				  *widget;
};

enum {
	HYBRID_STATE_OFFLINE = 0,
	HYBRID_STATE_INVISIBLE,
	HYBRID_STATE_AWAY,
	HYBRID_STATE_BUSY,
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
 * Clear the buddies belong to this account,
 * both in the local cache file and in the buddy treeview
 *
 * @param account The account.
 */
void hybrid_account_clear_buddy(HybridAccount *account);

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
 * Set the account's status text.
 *
 * @param account The account.
 * @param text    The status text.
 */
void hybrid_account_set_status_text(HybridAccount *account, const gchar *text);

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
 * Set whether the account is enabled.
 *
 * @param account The account.
 * @param enabled Whether the account is enabled.
 */
void hybrid_account_set_enabled(HybridAccount *account, gboolean enabled);

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
 * Enable an account, it will create a login tips panel in the
 * bottom of the buddy list, and then call the protocol login function.
 *
 * @param account The account.
 */
void hybrid_account_enable(HybridAccount *account);

/**
 * Close an account. Remove it from the blist panel. Free the
 * memory of the buddis and groups. But dont free the memory of
 * the account. The call the protocol close callback function.
 *
 * @param account The account the close.
 */
void hybrid_account_close(HybridAccount *account);

/**
 * Close all the accounts enabled.
 */
void hybrid_account_close_all();

/**
 * Enable all the accounts.
 */
void hybrid_account_enable_all();

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
 * Note that before calling this function, you should make sure that you have
 * set nickname,mood phrase, and state for the account.
 *
 * @param account The account.
 * @param status  The new connection status.
 */
void hybrid_account_set_connection_status(HybridAccount *account,
                                          HybridConnectionStatusType status);

/**
 * Set the status string of the current connection, it will be displayed in
 * the login panel in the bottom of the buddy list.
 *
 * @param account The account.
 * @param string  The status string.
 */
void hybrid_account_set_connection_string(HybridAccount *account,
                                          const gchar *string);

/**
 * Get the human readable name of the given presence state.
 *
 * @return The name of the presence state.
 */
const gchar *hybrid_get_presence_name(gint presence_state);

/**
 * Create an variable object.
 *
 * @param type The account variable type.
 * @param name The name of the variable.
 *
 * @return The variable object created.
 */
HybridAccountVariable *hybrid_variable_create(HybridAccountVariableType	 type,
											  const gchar				*var_name,
											  const gchar				*var_title);

/**
 * Destroy an variable object.
 *
 * @param variable The variable object to destroy.
 */
void hybrid_variable_destroy(HybridAccountVariable *variable);

/**
 * Set the default value for a string variable.
 *
 * @param var           The variable object.
 * @param defalut_value The default value of the variable.
 */
void hybrid_variable_set_string_default(HybridAccountVariable *var,
                                        const gchar           *defalut_value);

/**
 * Set the default value for an integer variable.
 *
 * @param var           The variable object.
 * @param defalut_value The default value of the variable.
 */
void hybrid_variable_set_integer_default(HybridAccountVariable *var,
                                         gint                   default_value);
/**
 * Set the default value for a boolean variable.
 *
 * @param var           The variable object.
 * @param defalut_value The default value of the variable.
 */
void hybrid_variable_set_bool_default(HybridAccountVariable *var,
                                      gboolean               defalut_value);

/**
 * Set the string value for a user-defined variable.
 *
 * @param account The account context.
 * @param name    The name of the string variable.
 * @param value   The value of the variable.
 */
void hybrid_account_set_string_variable(HybridAccount *account,
										const gchar	  *name,
										const gchar	  *value);

/**
 * Get the value of an user-defined string variable.
 *
 * @param account The account context.
 * @param name    The name of the string variable.
 *
 * @return The value of the string variable.
 */
const gchar* hybrid_account_get_string_variable(HybridAccount *account,
												const gchar	  *name);

/**
 * Set the boolean value for a user-defined variable.
 *
 * @param account The account context.
 * @param name    The name of the boolean variable.
 * @param value   The value of the boolean variable.
 */
void hybrid_account_set_bool_variable(HybridAccount	*account,
									  const gchar	*name,
									  gboolean		 value);

/**
 * Get the value of an user-defined boolean variable.
 *
 * @param account The account context.
 * @param name    The name of the boolean value.
 *
 * @return The value of the boolean variable.
 */
gboolean hybrid_account_get_bool_variable(HybridAccount	*account,
										  const gchar	*name);


/**
 * Set the integer value for a user-defined variable.
 *
 * @param account The account context.
 * @param name    The name of the integer variable.
 * @param value   The value of the integer variable.
 */
void hybrid_account_set_int_variable(HybridAccount *account,
									 const gchar   *name,
									 gint			value);

 /**
 * Get the value of an user-defined integer variable.
 *
 * @param account The account context.
 * @param name    The name of the integer value.
 *
 * @return The value of the integer variable.
 */
gint hybrid_account_get_int_variable(HybridAccount *account,
									 const gchar   *name);

#ifdef __cplusplus
}
#endif

#endif

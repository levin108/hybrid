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

#ifndef HYBRID_XMPP_ACCOUNT_H
#define HYBRID_XMPP_ACCOUNT_H
#include <glib.h>

#include "account.h"
#include "xmlnode.h"


typedef struct _XmppAccount XmppAccount;

#include "xmpp_stream.h"

struct _XmppAccount {
	gchar *username;
	gchar *password;

	gchar *to;

	GHashTable *buddies;

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
 * Process the account information.
 *
 * @param stream The stream for the account.
 * @param root   Root node of the xml context.
 *
 * @param HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_account_process_info(XmppStream *stream, xmlnode *root);

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
 * Modify photo for the account.
 *
 * @param stream   The stream for the account to modify.
 * @param filename The filename of the photo.
 *
 * @return HYBRID_ERROR or HYBRID_OK in case of an error.
 */
gint xmpp_account_modify_photo(XmppStream *stream, const gchar *filename);

/**
 * Modify full name for the account.
 *
 * @param stream The stream for the account to modify.
 * @param name   The full name of the account.
 *
 * @return HYBRID_ERROR or HYBRID_OK in case of an error.
 */
gint xmpp_account_modify_name(XmppStream *stream, const gchar *name);

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

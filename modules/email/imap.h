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
#ifndef HYBRID_IMAP_H
#define HYBRID_IMAP_H

#include "gtkutils.h"
#include "connect.h"
#include "util.h"

typedef struct _hybrid_imap hybrid_imap;
typedef struct _imap_trans  imap_trans;
typedef gboolean (*trans_callback)(hybrid_imap *imap, const gchar *msg, gpointer user_data);

struct _hybrid_imap {
    HybridAccount       *account;
    HybridConnection    *conn;
    HybridSslConnection *ssl;
    gchar               *username; /* Username to the IMAP server. */
    gchar               *password; /* Password to the IMAP server. */
    gboolean             use_tls; /* Whether to use tls connection. */
    gint                 current_tag; /* IMAP tag, we make it start from 100. */
    gchar               *buffer; /* Buffer to read data from server. */
    gint                 buffer_length; /* Length of the avaiable data in the buffer. */
    gchar               *email_addr; /* The full E-mail address of the account. */
    gchar               *imap_server; /* IMAP server address. */
    gint                 imap_port; /* IMAP server port. */
    GSList              *trans_list; /* The pending transaction list. */
    gint                 unread; /* The count of unread mails. */
    gint                 tmp_unread;

    guint conn_read_source;
    guint mail_check_interval;
    guint mail_check_source;
};

struct _imap_trans {
    hybrid_imap    *imap;
    gint            tag;
    trans_callback  callback;
    gpointer        user_data;
};

/**
 * Create an IMAP object to handle the imap context.
 *
 * @param account The hybrid account.
 *
 * @return The IMAP object created.
 */
hybrid_imap *hybrid_imap_create(HybridAccount *account);

/**
 * Destroy an IMAP object.
 *
 * @param imap The IMAP object to destroy.
 */
void hybrid_imap_destroy(hybrid_imap *imap);

/**
 * Authenticating to the IMAP server.
 *
 * @param imap     The IMAP context object.
 *
 * @return HYBRID_OK if success or HYBRID_ERROR in case of an error.
 */
gint hybrid_imap_auth(hybrid_imap *imap);

/**
 * Create an imap transaction, and add it to the pending transaction list.
 *
 * @param imap     The IMAP context object.
 * @param cb       The callback function called when transaction was processed.
 * @param user_data User-specified data for the callback function.
 *
 * @return The transaction created.
 */
imap_trans *imap_trans_create(hybrid_imap    *imap,
                              trans_callback  cb,
                              gpointer        user_data);

/**
 * Destroy an imap transaction.
 *
 * @param trans The transaction to destroy;
 */
void imap_trans_destroy(imap_trans *trans);

#endif /* HYBRID_IMAP_H */

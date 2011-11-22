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
#include "util.h"
#include "module.h"
#include "account.h"
#include "connect.h"

#include "imap.h"

static gboolean imap_read_cb(gint sk, hybrid_imap *imap);
static gboolean ssl_connect_cb(HybridSslConnection *ssl, hybrid_imap *imap);

static gboolean
imap_read_cb(gint sk, hybrid_imap *imap)
{
    gchar       buf[BUF_LENGTH];
    gint        n;
    gchar      *pos, *stop, *msg = NULL;
    gchar      *cmd_end          = NULL;
    gint        len              = 0;
    gboolean    last             = FALSE;
    GSList     *cur;
    imap_trans *trans;
    
    memset(buf, 0, BUF_LENGTH);

    if (-1 == (n = hybrid_ssl_read(imap->ssl, buf, BUF_LENGTH - 1))) {
        hybrid_debug_error("imap", "read from imap ssl connection error.");
        goto imap_read_err;
    }

    if (0 == n) {
        hybrid_debug_error("imap", "imap ssl connection closed.");
        goto imap_read_err;
    }
    buf[n] = '\0';

    imap->buffer         = (gchar*)realloc(imap->buffer, imap->buffer_length + n);
    memcpy(imap->buffer + imap->buffer_length, buf, n);
    imap->buffer_length += n;

    stop = imap->buffer;
    
 recheck:

    if (!stop) return TRUE;
    
    if ((pos = strstr(stop, "\r\n"))) {

        stop    = pos + 2;
        if (last) {
            len = stop - imap->buffer;
            msg = g_strndup(imap->buffer, len);
            goto read_ok;
        }

        if (*stop != '*' && *stop != '\0' && *stop != '\r') {
            last  = TRUE;
            pos  += 1;
            cmd_end = stop;
            goto recheck;
            
        } else if (*stop == '\0' || *stop == '\r'){
            return TRUE;
        } else {
            goto recheck;
        }
    } else {
        return TRUE;
    }

 read_ok:

    memmove(imap->buffer, imap->buffer + len, imap->buffer_length - len);
    imap-> buffer_length -= len;

    if (0 == imap->buffer_length) {
        g_free(imap->buffer);
        imap->buffer = NULL;
    } else {
        imap->buffer = (gchar*)realloc(imap->buffer,
                                       imap->buffer_length);
    }

    if (*cmd_end != 'A') {
        hybrid_debug_error("imap", "unknown imap message.");
        g_free(msg);
        return TRUE;
    }

    cmd_end += 1;

    for (pos = cmd_end; *pos != ' ' && *pos != '\0'; pos ++);

    if ('\0' == *pos) {
        hybrid_debug_error("imap", "unknown imap message.");
        g_free(msg);
        return TRUE;
    }

    stop = g_strndup(cmd_end, pos - cmd_end);

    for (cur = imap->trans_list; cur; cur = cur->next) {
        trans = (imap_trans*)cur->data;

        if (atoi(stop) == trans->tag) {
            trans->callback(imap, msg, trans->user_data);
            break;
        }
    }

    g_free(stop);
    g_free(msg);
    
    return TRUE;

 imap_read_err:

    hybrid_account_error_reason(imap->account, "Connection Closed.");
    return FALSE;
}


static gboolean
ssl_connect_cb(HybridSslConnection *ssl, hybrid_imap *imap)
{
    HybridAccount *account;

    account   = imap->account;
    imap->ssl = ssl;
    
    hybrid_debug_info("imap", "IMAP server connected.");
    hybrid_account_set_connection_string(account, _("IMAP Authenticating."));

    imap->conn_read_source = 
        hybrid_event_add(ssl->sk, HYBRID_EVENT_READ,
                         EVENT_CALLBACK(imap_read_cb), imap);

    if (HYBRID_OK != hybrid_imap_auth(imap)) {
        hybrid_account_error_reason(account, _("IMAP Authenticate failed."));
        return FALSE;
    }
    
    return FALSE;
}

gboolean
email_login(HybridAccount *account)
{
    hybrid_imap *imap;

    if (!(imap = hybrid_imap_create(account))) {
        hybrid_account_error_reason(account,
                                    "Username must be a valid email address.");
        return FALSE;
    }

    hybrid_account_set_protocol_data(account, imap);

    hybrid_debug_info("imap", "connecting to imap server %s:%d",
                      imap->imap_server, imap->imap_port);

    hybrid_account_set_connection_string(account,
                                         _("Connecting to IMAP server."));
        
    hybrid_ssl_connect(imap->imap_server,
                       imap->imap_port,
                       SSL_CALLBACK(ssl_connect_cb),
                       imap);
    
    return TRUE;
}

gboolean
email_close(HybridAccount *account)
{
    hybrid_imap *imap;

    imap = hybrid_account_get_protocol_data(account);

    if (!imap) return FALSE;

    if (imap->conn_read_source > 0) {
        g_source_remove(imap->conn_read_source);
    }
    
    if (imap->mail_check_source > 0) {
        g_source_remove(imap->mail_check_source);
    }

    hybrid_imap_destroy(imap);
    
    return FALSE;
}

GSList*
email_options()
{
    GSList                *list = NULL;
    HybridAccountVariable *var;

    var  = hybrid_variable_create(VARIABLE_TYPE_STRING,
                                 "imap_server",
                                 _("IMAP Server:"));
    list = g_slist_append(list, var);

    var  = hybrid_variable_create(VARIABLE_TYPE_INTEGER,
                                 "imap_port",
                                 _("Port:"));
    hybrid_variable_set_integer_default(var, 993);
    list = g_slist_append(list, var);
    
    var = hybrid_variable_create(VARIABLE_TYPE_INTEGER,
                                 "check_interval",
                                 _("Check Interval\n(seconds):"));
    hybrid_variable_set_integer_default(var, 30);
    list = g_slist_append(list, var);

    var = hybrid_variable_create(VARIABLE_TYPE_BOOLEAN,
                                 "use_tls",
                                 _("Use TLS"));
    hybrid_variable_set_bool_default(var, TRUE);
    list = g_slist_append(list, var);

    return list;
}

HybridEmailOps mail_ops = {
    email_login,                /* email login */
    email_close,                /* email close*/
};

HybridModuleInfo module_info = {
    "email",                    /**< name */
    "levin108",                 /**< author */
    N_("email notifier"),       /**< summary */
    /* description */
    N_("hybrid plugin implementing "
       "IMAP protocol for email notification."), 
    "http://basiccoder.com",    /**< homepage */
    "0","1",                    /**< major version, minor version */
    "email",                    /**< icon name */
    
    MODULE_TYPE_EMAIL,
    NULL,                       //
    &mail_ops,
    NULL,                       /**< actions */
    &email_options,
    
};

void 
email_module_init(HybridModule *module)
{

}

HYBRID_MODULE_INIT(email_module_init, &module_info);

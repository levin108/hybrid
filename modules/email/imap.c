#include "imap.h"
#include "notify.h"
#include "gtkutils.h"

#ifdef USE_LIBNOTIFY
#include <libnotify/notify.h>

//static NotifyNotification *notification = NULL;
#endif


static gint check_resp_ok(const gchar *msg);
static gboolean check_mail(gpointer user_data);
static void process_unread_mails(hybrid_imap *imap, gint unread);
    
hybrid_imap*
hybrid_imap_create(HybridAccount *account)
{
    hybrid_imap *imap;
    const gchar *username;
    const gchar *password;
    const gchar *imap_server;
    gboolean     use_tls;
    gint         imap_port;
    gint         check_interval;
    const gchar *pos;

    g_return_val_if_fail(account != NULL, NULL);
    
    username       = account->username;
    password       = account->password;
    imap_server    = hybrid_account_get_string_variable(account,
                                                        "imap_server");
    imap_port      = hybrid_account_get_int_variable(account,
                                                     "imap_port");
    use_tls        = hybrid_account_get_bool_variable(account,
                                                      "use_tls");
    check_interval = hybrid_account_get_int_variable(account,
                                                     "check_interval");

    for (pos = username; '@' != *pos && '\0' != *pos; pos ++);

    if ('\0' == *pos) return NULL;

    imap                      = g_new0(hybrid_imap, 1);
    imap->account             = account;
    imap->current_tag         = 100;
    imap->use_tls             = use_tls;
    imap->username            = g_strndup(username, pos - username);
    imap->password            = g_strdup(password);
    imap->mail_check_interval = check_interval;
    imap->imap_server         = g_strdup(imap_server);
    imap->imap_port           = imap_port;
    imap->email_addr          = g_strdup(username);

    return imap;
}

void
hybrid_imap_destroy(hybrid_imap *imap)
{
    if (imap) {
        hybrid_ssl_connection_destory(imap->ssl);

        g_free(imap->username);
        g_free(imap->password);

        g_free(imap);
    }
}

#ifdef USE_LIBNOTIFY
void action_cb(NotifyNotification *notify, char *action, hybrid_imap *imap)
{
    imap->unread = imap->tmp_unread;
}
#endif

static void
process_unread_mails(hybrid_imap *imap, gint unread)
{
    if (imap->unread == unread || unread == 0) {
        if (unread == 0) {
            imap->unread = 0;
        }
        return;
    }

#ifdef USE_LIBNOTIFY
    gchar       *summary;
    GdkPixbuf   *pixbuf;
    const gchar *title = _("New Mail.");
    NotifyNotification *notification = NULL;

    summary = g_strdup_printf(_("<b>%s</b>\n<span size='xx-large' "
                              "foreground='red'><b>%d</b></span> "
                                "<span size='large'>unread mails.</span>"),
                              imap->email_addr, unread);
#ifdef LIBNOTIFY_OLD
    notification = notify_notification_new(title, summary, NULL, NULL);
#else
    notification = notify_notification_new(title, summary, NULL);
#endif
    notify_notification_set_timeout(notification, 5000);

    imap->tmp_unread = unread;
    notify_notification_clear_actions(notification);
    notify_notification_add_action(notification, "test", _("I Know"),
                                   NOTIFY_ACTION_CALLBACK(action_cb),
                                   imap, NULL);

    pixbuf = hybrid_create_proto_icon(imap->account->proto->info->name, 48);
    if (pixbuf) {
        notify_notification_set_icon_from_pixbuf(notification, pixbuf);
        g_object_unref(pixbuf);
    }

    notify_notification_show(notification, NULL);
    g_free(summary);
    
#endif
}

gboolean
check_mail_cb(hybrid_imap *imap, const gchar *msg, gpointer user_data)
{
    const gchar *pos;
    const gchar *cur;
    gchar       *count_str;
    gint         count_int;
    
    hybrid_debug_info("imap", "recv:\n%s", msg);

    if (HYBRID_OK == check_resp_ok(msg)) {
        if ((pos = strstr(msg, "UNSEEN "))) {
            pos += 7;
            for (cur = pos; '\0' != *cur && ')' != *cur; cur ++);

            if ('\0' != *cur) {
                count_str = g_strndup(pos, cur - pos);
                count_int = atoi(count_str);
                g_free(count_str);

                process_unread_mails(imap, count_int);
            }
        }
    } else {
        hybrid_debug_error("imap", "check unread mail error.");
    }

    imap->mail_check_source = g_timeout_add_seconds(imap->mail_check_interval,
                                                    check_mail, imap);
    
    return FALSE;
}

static gboolean
check_mail(gpointer user_data)
{
    hybrid_imap *imap;
    gchar       *cmd;

    imap = (hybrid_imap*)user_data;

    cmd = g_strdup_printf("A%d STATUS INBOX (UNSEEN)\r\n",
                          imap->current_tag);

    hybrid_debug_info("imap", "send:\n%s", cmd);

    imap_trans_create(imap, check_mail_cb, NULL);

    if (hybrid_ssl_write(imap->ssl, cmd, strlen(cmd)) <= 0) {
        hybrid_account_error_reason(imap->account,
                                    _("IMAP Connection Closed."));
    }

    g_free(cmd);

    if (imap->mail_check_source > 0) {
        g_source_remove(imap->mail_check_source);
        imap->mail_check_source = 0;
    }

    return FALSE;
}

static gint
check_resp_ok(const gchar *msg)
{
    const gchar *pos;
    const gchar *cur;
    
    if ('A' == *msg) {
        pos = msg;
    } else {
        pos = strstr(msg, "\r\nA");
        assert(pos != NULL);
        pos += 3;
    }

    for (cur = pos; *cur != '\0' && *cur != ' '; cur ++);
    if ('\0' == *cur) {
        return HYBRID_ERROR;
    }
    cur ++;
    for (pos = cur + 1; *pos != '\0' && *pos != ' '; pos ++);
    if ('\0' == *pos) {
        return HYBRID_ERROR;
    }

    if (strncmp("OK", cur, pos - cur) == 0) {
        return HYBRID_OK;
    } else {
        return HYBRID_ERROR;
    }
}

gboolean
imap_auth_cb(hybrid_imap *imap, const gchar *msg, gpointer user_data)
{
    hybrid_debug_info("imap", "recv:\n%s", msg);

    if (HYBRID_OK == check_resp_ok(msg)) {
        hybrid_account_set_connection_string(imap->account,
                                             _("IMAP OK."));
        hybrid_account_set_connection_status(imap->account,
                                             HYBRID_CONNECTION_CONNECTED);
        
        hybrid_account_set_state(imap->account, HYBRID_STATE_ONLINE);

        imap->mail_check_source = g_timeout_add_seconds(
                                        imap->mail_check_interval,
                                        check_mail, imap);
        return FALSE;
    }

    hybrid_account_error_reason(imap->account,
                                _("IMAP Authenticate Failed."
                                  " Check your username and password."));
    
    return FALSE;
}

gint
hybrid_imap_auth(hybrid_imap *imap)
{
    gchar               *cmd;
    HybridSslConnection *ssl;

    ssl = imap->ssl;
    cmd = g_strdup_printf("A%d LOGIN %s %s\r\n",
                          imap->current_tag,
                          imap->username,
                          imap->password);

    hybrid_debug_info("imap", "send:\n%s", cmd);
    
    imap_trans_create(imap, imap_auth_cb, NULL);

    if (hybrid_ssl_write(ssl, cmd, strlen(cmd)) <= 0) {

        hybrid_debug_error("imap", "write to IMAP ssl connection error.");
        g_free(cmd);
        return HYBRID_ERROR;
    }

    g_free(cmd);

    return HYBRID_OK;
}

imap_trans*
imap_trans_create(hybrid_imap    *imap,
                  trans_callback  cb,
                  gpointer        user_data)
{
    imap_trans *trans;

    g_return_val_if_fail(imap != NULL, NULL);

    trans            = g_new0(imap_trans, 1);
    trans->imap      = imap;
    trans->tag       = imap->current_tag ++;
    trans->callback  = cb;
    trans->user_data = user_data;

    imap->trans_list = g_slist_append(imap->trans_list, trans);

    return trans;
}
    
void
imap_trans_destroy(imap_trans *trans)
{
    hybrid_imap *imap;

    if (!trans) {
        return;
    }

    imap             = trans->imap;
    trans->tag       = imap->current_tag;
    imap->trans_list = g_slist_remove(imap->trans_list, trans);

    g_free(trans);
}

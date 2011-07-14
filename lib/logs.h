#ifndef HYBRID_LOGS_H
#define HYBRID_LOGS_H
#include <glib.h>
#include "util.h"
#include "account.h"
#include "xmlnode.h"

typedef struct _HybridLogs HybridLogs;

struct _HybridLogs {
	gchar *log_path;
	gchar *id;
	time_t time;
	xmlnode *root;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the logs context.
 *
 * HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint hybrid_logs_init(void);

/**
 * Create a log context.
 *
 * @param account The log for which account.
 * @param id      The id of the chat window.
 *
 * @return The log context created.
 */
HybridLogs *hybrid_logs_create(HybridAccount *account,
		const gchar *id);

/**
 * Write a log.
 *
 * @param log     The log context.
 * @param name    The name of the message sender.
 * @param msg     The content of the message.
 * @param sendout Whether the message is sent out or received.
 */
gint hybrid_logs_write(HybridLogs *log, const gchar *name, const gchar *msg,
					gboolean sendout);

/**
 * Destroy a log context.
 *
 * @param log The log context to destroy.
 */
void hybrid_logs_destroy(HybridLogs *log);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_LOGS_H */

#ifndef HYBRID_ACTION_H
#define HYBRID_ACTION_H
#include <gtk/gtk.h>

#include "account.h"

typedef struct _HybridAction HybridAction;
typedef void (*ActionCallback)(HybridAction *);

struct _HybridAction {
	ActionCallback callback;
	gchar *text;
	HybridAccount *account;
};

/**
 * Create a new action menu.
 *
 * @param account  The account context.
 * @param text     The text of the action menu.
 * @param callback The callback function of the action menu.
 *
 * @return The action menu created.
 */
HybridAction *hybrid_action_create(HybridAccount *account,
				const gchar *text, ActionCallback callback);

/**
 * Destroy an action menu.
 *
 * @param action The action menu to destroy.
 */
void hybrid_action_destroy(HybridAction *action);

/**
 * Get the account context through the action context.
 *
 * @param action The action context.
 *
 * @return The account context.
 */
HybridAccount *hybrid_action_get_account(HybridAction *action);

#endif /* HYBRID_ACTION_H */

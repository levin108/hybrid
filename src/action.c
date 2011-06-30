#include "action.h"

HybridAction*
hybrid_action_create(HybridAccount *account, const gchar *text, 
		ActionCallback callback)
{
	HybridAction *action;

	g_return_val_if_fail(account != NULL, NULL);
	g_return_val_if_fail(text != NULL, NULL);

	action = g_new0(HybridAction, 1);
	action->text     = g_strdup(text);
	action->account  = account;
	action->callback = callback;

	return action;
}

void
hybrid_action_destroy(HybridAction *action)
{
	if (action) {
		g_free(action->text);
		g_free(action);
	}
}

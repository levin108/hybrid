#ifndef _HYBRID_CONFIRM_H
#define _HYBRID_CONFIRM_H

#include <gtk/gtk.h>
#include "account.h"

typedef struct _HybridConfirmWindow						 HybridConfirmWindow;
typedef void (*HybridConfirmOkFunc)(HybridAccount		*account,
									const gchar	   		*code,
									gpointer			 user_data);
typedef	void (*HybridConfirmCancelFunc)(HybridAccount	*account,
										gpointer		 user_data);

struct _HybridConfirmWindow {
	GtkWidget					*window;
	GtkWidget					*image;
	GtkWidget					*entry;
	HybridAccount				*account;
	HybridConfirmOkFunc			 ok_func;
	HybridConfirmCancelFunc		 cancel_func;
	gpointer					 user_data;
};

/**
 * Create a confirm window.
 *
 * @param account     The hybrid account context.
 * @param ok_func     The callback function for ok button.
 * @param cancel_func The callback function for calcel button.
 * @param user_data   The user-specified data for callback functions.
 *
 * @return The confirm window created.
 */
HybridConfirmWindow *hybrid_confirm_window_create(HybridAccount					*account,
												  guchar						*pic_data,
												  gint							 pic_len,
												  HybridConfirmOkFunc			 ok_func,
												  HybridConfirmCancelFunc		 cancel_func,
												  gpointer						 user_data);

#endif /* _HYBRID_CONFIRM_H */

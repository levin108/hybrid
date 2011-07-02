#ifndef HYBRID_GROUPADD_H
#define HYBRID_GROUPADD_H
#include <gtk/gtk.h>
#include "account.h"

typedef struct _HybridGroupAddWindow HybridGroupAddWindow;

struct _HybridGroupAddWindow {
	GtkWidget *window;
	GtkWidget *account_combo;
	GtkWidget *name_entry;
	HybridAccount *account;
};

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Create an group add window.
 *
 * @param the window created.
 */
HybridGroupAddWindow *hybrid_groupadd_window_create();


#ifdef __cplusplus
}
#endif

#endif /* HYBRID_GROUPADD_ */

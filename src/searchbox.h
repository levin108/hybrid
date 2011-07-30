#ifndef HYBRID_SEARCH_H
#define HYBRID_SEARCH_H
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a search box context.
 *
 * @return The GtkEntry object created.
 */
GtkWidget *hybrid_search_box_create(void);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_SEARCH_H */

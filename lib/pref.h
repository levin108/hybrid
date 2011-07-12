#ifndef HYBRID_PREF_H
#define HYBRID_PREF_H
#include <gtk/gtk.h>
#include "util.h"
#include "xmlnode.h"

typedef struct _HybridPref HybridPref;

struct _HybridPref {
	gchar *filename;
	xmlnode *root;

	gboolean mute;
	gboolean hide_chat_buttons;
	gboolean disable_chat_tabs;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the preference context.
 *
 * HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint hybrid_pref_init(void);

/**
 * Save the context of the preference to the local disk.
 */
void hybrid_pref_save(void);

/**
 * Set the string value for a property.
 *
 * @param name  The name of the property.
 * @param value The string value of the property;
 */
void hybrid_pref_set_string(const gchar *name, const gchar *value);

/**
 * Set the gboolean value for a property.
 *
 * @param name  The name of the property.
 * @param value The string value of the property;
 */
void hybrid_pref_set_boolean(const gchar *name, const gboolean value);

/**
 * Get the bool value of a given property.
 *
 * @param name The name of the property.
 *
 * @return The value of the property.
 */
gboolean hybrid_pref_get_boolean(const gchar *name);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_PREF_H */

#ifndef Hybird_CONFIG_H
#define Hybird_CONFIG_H
#include "xmlnode.h"

typedef struct _HybirdBlistCache HybirdBlistCache;
typedef struct _HybirdConfig HybirdConfig;

struct _HybirdBlistCache {
	xmlnode *root;
};

struct HybirdConfig {
	gchar *config_path;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the absolute path of the config directory,
 * such as /home/levin/.config/hyhird
 *
 * @return The path name, needs to be freed when no longer used.
 */
gchar *hybird_config_get_path(void);

#ifdef __cplusplus
}
#endif

#endif /* Hybird_CONFIG_H */

#ifndef IM_CONFIG_H
#define IM_CONFIG_H
#include "xmlnode.h"

typedef struct _IMBlistCache IMBlistCache;
typedef struct _IMConfig IMConfig;

struct _IMBlistCache {
	xmlnode *root;
};

struct IMConfig {
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
gchar *im_config_get_path(void);

#ifdef __cplusplus
}
#endif

#endif /* IM_CONFIG_H */

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "util.h"
#include "config.h"

gchar*
hybird_config_get_path(void)
{
	gchar *home;
	gchar *config_path;
	gchar *hybird_path;
	gint e;

	if (!(home = getenv("HOME"))) {
		hybird_debug_error("config", "No environment variable named HOME\n");
		return NULL;
	}

	config_path = g_strdup_printf("%s/.config", home);

	e = mkdir(config_path, S_IRWXU|S_IRWXO|S_IRWXG);

	if (e && access(config_path, R_OK|W_OK)) {
		hybird_debug_error("config", "%s,cannot create, read or write",
				config_path);
		g_free(config_path);

		return NULL;
	}

	hybird_path = g_strdup_printf("%s/hybird", config_path);

	e = mkdir(hybird_path, S_IRWXU|S_IRWXO|S_IRWXG);

	if (e && access(hybird_path, R_OK|W_OK)) {
		hybird_debug_error("config", "%s,cannot create, read or write",
				hybird_path);
		g_free(config_path);
		g_free(hybird_path);

		return NULL;
	}

	g_free(config_path);

	return hybird_path;
}

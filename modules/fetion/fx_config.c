#include <sys/stat.h>
#include <sys/types.h>

#include "util.h"

#include "fx_config.h"

gchar*
fetion_get_config_dir(fetion_account *account)
{
	gchar *hybrid_dir;
	gchar *fetion_dir;
	gint e;

	g_return_val_if_fail(account != NULL, NULL);

	if (!(hybrid_dir = hybrid_config_get_path())) {
		return NULL;
	}

	fetion_dir = g_strdup_printf("%s/%s", hybrid_dir, account->sid);
	g_free(hybrid_dir);

	e = mkdir(fetion_dir, S_IRWXU|S_IRWXO|S_IRWXG);

	if (e && access(fetion_dir, R_OK|W_OK)) {
		hybrid_debug_error("fetion", "%s,cannot create, read or write",
				fetion_dir);
		g_free(fetion_dir);

		return NULL;
	}

	return fetion_dir;
}

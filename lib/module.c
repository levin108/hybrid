#include <dirent.h>
#include "util.h"
#include "module.h"

/* module chain stores the modules registered */
GSList *modules = NULL;

typedef gboolean (*ModuleInitFunc)(HybirdModule *module);

HybirdModule*
hybird_module_create(const gchar *path)
{
	HybirdModule *module = g_new0(HybirdModule, 1);

	module->path = g_strdup(path);

	return module;
}

void 
hybird_module_destroy(HybirdModule *module)
{
	if (module) {
		g_free(module->path);
	}

	g_free(module);
}

gint 
hybird_module_load(HybirdModule *module)
{
	GModule *gm;
	ModuleInitFunc hybird_module_init;

	g_return_val_if_fail(module != NULL, HYBIRD_ERROR);

	if (!(gm = g_module_open(module->path, G_MODULE_BIND_LOCAL))) {

		hybird_debug_error("module", g_module_error());

		return HYBIRD_ERROR;
	}

	if (!g_module_symbol(gm, "proto_module_init", 
				(gpointer*)&hybird_module_init))	{

		hybird_debug_error("module", g_module_error());

		return HYBIRD_ERROR;

	}

	hybird_module_init(module);

	return HYBIRD_OK;
}

void 
hybird_module_register(HybirdModule *module)
{
	GSList *iter;

	for (iter = modules; iter; iter = iter->next) {

		if (iter->data == module) {
			return;
		}
	}

	modules = g_slist_append(modules, module);
}

HybirdModule*
hybird_module_find(const gchar *name)
{
	GSList *iter;
	HybirdModule *temp;

	for (iter = modules; iter; iter = iter->next) {
		temp = (HybirdModule*)iter->data;

		if (g_strcmp0(temp->info->name, name) == 0) {
			return temp;
		}
	}

	return NULL;
}

gint 
hybird_module_init()
{
	GDir *dir;
	const gchar *name;
	gchar *abs_path;
	HybirdModule *module;

	hybird_debug_info("module", "initialize module");

	if ((dir = g_dir_open(MODULE_DIR, 0, NULL)) == NULL) {

		hybird_debug_error("module", "open modules directory: %s", MODULE_DIR);

		return HYBIRD_ERROR;
	}

	while ((name = g_dir_read_name(dir))) {

		abs_path = g_strjoin("/", MODULE_DIR, name, NULL);

		if (g_file_test(abs_path, G_FILE_TEST_IS_DIR)) {
			g_free(abs_path);
			continue;
		}

		if (g_str_has_suffix(abs_path, G_MODULE_SUFFIX)) {
			
			module = hybird_module_create(abs_path);

			if (hybird_module_load(module) != HYBIRD_OK) {
				hybird_module_destroy(module);
			}

		}

		g_free(abs_path);
	}

	g_dir_close(dir);

	return HYBIRD_OK;
}

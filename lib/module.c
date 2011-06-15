#include <dirent.h>
#include "util.h"
#include "module.h"

/* module chain stores the modules registered */
GSList *modules = NULL;

typedef gboolean (*ModuleInitFunc)(IMModule *module);

IMModule*
im_module_create(const gchar *path)
{
	IMModule *module = g_new0(IMModule, 1);

	module->path = g_strdup(path);

	return module;
}

void 
im_module_destroy(IMModule *module)
{
	if (module) {
		g_free(module->path);
	}

	g_free(module);
}

gint 
im_module_load(IMModule *module)
{
	GModule *gm;
	ModuleInitFunc im_module_init;

	g_return_val_if_fail(module != NULL, IM_ERROR);

	if (!(gm = g_module_open(module->path, G_MODULE_BIND_LOCAL))) {

		im_debug_error("module", g_module_error());

		return IM_ERROR;
	}

	if (!g_module_symbol(gm, "proto_module_init", 
				(gpointer*)&im_module_init))	{

		im_debug_error("module", g_module_error());

		return IM_ERROR;

	}

	im_module_init(module);

	return IM_OK;
}

void 
im_module_register(IMModule *module)
{
	GSList *iter;

	for (iter = modules; iter; iter = iter->next) {

		if (iter->data == module) {
			return;
		}
	}

	modules = g_slist_append(modules, module);
}

IMModule*
im_module_find(const gchar *name)
{
	GSList *iter;
	IMModule *temp;

	for (iter = modules; iter; iter = iter->next) {
		temp = (IMModule*)iter->data;

		if (g_strcmp0(temp->info->name, name) == 0) {
			return temp;
		}
	}

	return NULL;
}

gint 
im_module_init()
{
	GDir *dir;
	const gchar *name;
	gchar *abs_path;
	IMModule *module;

	im_debug_info("module", "initialize module");

	if ((dir = g_dir_open(MODULE_DIR, 0, NULL)) == NULL) {

		im_debug_error("module", "open modules directory: %s", MODULE_DIR);

		return IM_ERROR;
	}

	while ((name = g_dir_read_name(dir))) {

		abs_path = g_strjoin("/", MODULE_DIR, name, NULL);

		if (g_file_test(abs_path, G_FILE_TEST_IS_DIR)) {
			g_free(abs_path);
			continue;
		}

		if (g_str_has_suffix(abs_path, G_MODULE_SUFFIX)) {
			
			module = im_module_create(abs_path);

			if (im_module_load(module) != IM_OK) {
				im_module_destroy(module);
			}

		}

		g_free(abs_path);
	}

	g_dir_close(dir);

	return IM_OK;
}

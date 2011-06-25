#include "util.h"
#include "xmlnode.h"

#include "fx_util.h"

gchar*
get_province_name(const gchar *province)
{
	xmlnode *root;
	xmlnode *node;
	gchar *name;
	gchar *value;

	g_return_val_if_fail(province != NULL, NULL);

	if (!(root = xmlnode_root_from_file(FETION_RES_DIR"province.xml"))) {
		return NULL;
	}

	if (!(node = xmlnode_child(root)) || g_strcmp0(node->name, "Province")) {
		hybrid_debug_error("fetion",
				"get full province name");
		return NULL;
	}

	for (; node; node = xmlnode_next(node)) {

		if (!xmlnode_has_prop(node, "id")) {
			continue;
		}
		
		value = xmlnode_prop(node, "id");

		if (g_strcmp0(value, province) == 0) {
			name = xmlnode_content(node);

			/* found, do cleanup. */
			g_free(value);
			xmlnode_free(root);

			return name;
		}

		g_free(value);
	}

	xmlnode_free(root);

	return NULL;
}

gchar*
get_city_name(const gchar *province, const gchar *city)
{
	xmlnode *root;
	xmlnode *node;
	gchar *name;
	gchar *value;

	if (!(root = xmlnode_root_from_file(FETION_RES_DIR"city.xml"))) {
		return NULL;
	}

	if (!(node = xmlnode_child(root)) || g_strcmp0(node->name, "Province")) {
		hybrid_debug_error("fetion",
				"get full city name");
		return NULL;
	}

	for (; node; node = xmlnode_next(node)) {

		if (!xmlnode_has_prop(node, "id")) {
			continue;
		}
		
		value = xmlnode_prop(node, "id");

		if (g_strcmp0(value, province) == 0) {
			/* found, do cleanup. */
			g_free(value);

			goto province_found;
		}

		g_free(value);

	}

	xmlnode_free(root);

	return NULL;

province_found:

	if (!(node = xmlnode_child(node)) || g_strcmp0(node->name, "City")) {
		hybrid_debug_error("fetion",
				"get full city name");
		xmlnode_free(root);

		return NULL;
	}

	for (; node; node = xmlnode_next(node)) {

		if (!xmlnode_has_prop(node, "id")) {
			continue;
		}
		
		value = xmlnode_prop(node, "id");

		if (g_strcmp0(value, city) == 0) {

			name= xmlnode_content(node);

			/* found, do cleanup. */
			g_free(value);
			xmlnode_free(root);

			return name;
		}

		g_free(value);

	}

	xmlnode_free(root);

	return NULL;
}

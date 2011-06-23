#include "fx_msg.h"
#include "util.h"
#include "xmlnode.h"

gint
fetion_message_parse_sysmsg(const gchar *sipmsg, gchar **content, gchar **url)
{
	gchar *pos;
	xmlnode *root;
	xmlnode *node;

	if (!(pos = strstr(sipmsg, "\r\n\r\n"))) {
		goto sysmsg_error;
	}

	if (!(root = xmlnode_root(pos, strlen(pos)))) {
		goto sysmsg_error;
	}

	if (!(node = xmlnode_find(root, "content"))) {
		xmlnode_free(root);
		goto sysmsg_error;
	}

	*content = xmlnode_content(node);

	if ((node = xmlnode_find(root, "url"))) {
		*url = xmlnode_content(node);

	} else {
		*url = NULL;
	}

	xmlnode_free(root);

	return HYBRID_OK;

sysmsg_error:
	hybrid_debug_error("fetion", "invalid system message");
	return HYBRID_ERROR;
}

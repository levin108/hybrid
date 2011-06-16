#include "xmlnode.h"
#include "util.h"

xmlnode*
xmlnode_root(const gchar *xml_buf, gint size)
{
	xmlDoc *doc;
	xmlnode *node;

	g_return_val_if_fail(xml_buf != NULL, NULL);
	g_return_val_if_fail(size != 0, NULL);

	doc = xmlParseMemory(xml_buf, size);
	
	if (!doc) {
		hybird_debug_error("xml", "parse xml");
		return NULL;
	}

	node = g_new0(xmlnode, 1);
	node->node = xmlDocGetRootElement(doc);
	node->doc = doc;
	node->is_root = 1;
	node->next = NULL;
	node->child = NULL;
	node->name = g_strdup((gchar*)node->node->name);

	return node;
}

xmlnode*
xmlnode_root_from_file(const gchar *filepath)
{
	xmlDoc *doc;
	xmlnode *node;

	g_return_val_if_fail(filepath != NULL, NULL);

	doc = xmlParseFile(filepath);
	
	if (!doc) {
		hybird_debug_error("xml", "parse xml file");
		return NULL;
	}

	node = g_new0(xmlnode, 1);
	node->node = xmlDocGetRootElement(doc);
	node->doc = doc;
	node->is_root = 1;
	node->next = NULL;
	node->child = NULL;
	node->name = g_strdup((gchar*)node->node->name);

	return node;
}

xmlnode*
xmlnode_next(xmlnode *node)
{
	xmlnode *new;

	g_return_val_if_fail(node != NULL, NULL);
	
	new = node->next;

	return new;
}

xmlnode*
xmlnode_find(xmlnode *node, const gchar *name)
{
	xmlnode *temp, *iter;


	if (g_strcmp0(node->name, name) == 0) {
		return node;	

	} else {
		if ((iter = xmlnode_child(node)) == NULL) {
			return NULL;

		} else {
			while (iter) {
				temp = xmlnode_find(iter, name);
				if (temp) {
					return temp;
				}
				iter = xmlnode_next(iter);
			}
		}
	}

	return NULL;
}

xmlnode*
xmlnode_child(xmlnode *node)
{
	xmlnode *head = NULL;
	xmlnode *tail = NULL;
	xmlnode *new;
	xmlNode *cnode;

	if (node->child) {
		return node->child;
	}

	cnode = node->node->xmlChildrenNode;

	while (cnode) {

		new = g_new0(xmlnode, 1);
		new->node = cnode;
		new->is_root = 0;
		new->doc = node->doc;
		new->name = g_strdup((gchar*)cnode->name);
		new->child = NULL;
		new->next = NULL;

		if (!head) {
			head = new;
			tail = new;

		} else {
			tail->next = new;
			tail = new;
		}

		cnode = cnode->next;
	}

	node->child = head;

	return head;
}

gchar*
xmlnode_prop(xmlnode *node, const gchar *prop)
{
	gchar *value;
	xmlChar *temp;

	g_return_val_if_fail(node != NULL, NULL);
	g_return_val_if_fail(prop != NULL, NULL);

	temp = xmlGetProp(node->node, BAD_CAST prop);

	value = g_strdup((gchar*)temp);

	xmlFree(temp);

	return value;
}

gboolean
xmlnode_has_prop(xmlnode *node, const gchar *prop)
{
	g_return_val_if_fail(node != NULL, FALSE);
	g_return_val_if_fail(prop != NULL, FALSE);

	if (xmlHasProp(node->node, BAD_CAST prop)) {
		return TRUE;
			
	} else {
		return FALSE;
	}
}

gchar*
xmlnode_content(xmlnode *node)
{
	gchar *value;
	xmlChar *temp;

	g_return_val_if_fail(node != NULL, NULL);

	temp = xmlNodeGetContent(node->node);
	value = g_strdup((gchar*)temp);
	xmlFree(temp);

	return value;
}

xmlnode*
xmlnode_new_child(xmlnode *node, const gchar *childname)
{
	xmlnode *new;
	xmlnode *child;
	xmlnode *pos;

	g_return_val_if_fail(node != NULL, NULL);
	g_return_val_if_fail(childname != NULL, NULL);

	new = g_new0(xmlnode, 1);
	new->node = xmlNewChild(node->node, NULL, BAD_CAST childname, NULL);
	new->doc = node->doc;
	new->is_root = 0;
	new->next = NULL;
	new->child = NULL;
	new->name = g_strdup(childname);

	child = xmlnode_child(node);

	if (child) {
		pos = child;
		while (pos) {
			if (pos->next == NULL) {
				pos->next = new;
				break;
			}

			pos = pos->next;
		}

	} else {
		node->child = new;
	}

	return new;
}

gchar*
xmlnode_to_string(xmlnode *root)
{
	xmlDoc *doc;
	xmlChar *dump;
	gchar *temp;
	gchar *pos;
	gchar *res;

	g_return_val_if_fail(root != NULL, NULL);
	g_return_val_if_fail(root->is_root == 1, NULL);

	doc = root->doc;
	xmlDocDumpMemory(doc, &dump, NULL);

	temp = g_strdup((gchar*)dump);
	xmlFree(dump);
	xmlnode_free(root);

	/* now strip the head */
	pos = temp;

	while (*pos) {
		if (*pos == '?' && *(pos + 1) == '>') {
			pos += 2;
			break;
		} 

		pos ++;
	}

	res = g_strdup(pos);
	g_free(temp);

	return res;
}

void
xmlnode_new_prop(xmlnode *node, const gchar *name, const gchar *value)
{
	xmlNewProp(node->node, BAD_CAST name, BAD_CAST value);
}

void
xmlnode_set_prop(xmlnode *node, const gchar *name, const gchar *value)
{
	xmlSetProp(node->node, BAD_CAST name, BAD_CAST value);
}

gint
xmlnode_save_file(xmlnode *root, const gchar *filepath)
{
	g_return_val_if_fail(root != NULL, HYBIRD_ERROR);
	g_return_val_if_fail(root->is_root == 1, HYBIRD_ERROR);
	g_return_val_if_fail(filepath != NULL, HYBIRD_ERROR);

	if (xmlSaveFormatFileEnc(filepath, root->doc, "UTF-8", 0) == -1) {
		return HYBIRD_ERROR;
	}

	return HYBIRD_OK;
}

void
xmlnode_free(xmlnode *node)
{
	xmlnode **temp;

	if (!node) {
		return;
	}

	g_free(node->name);

	if (node->is_root) {
		xmlFreeDoc(node->doc);
		node->doc = NULL;
	}

	if (node->child) {
		xmlnode_free(node->child);
	}

	while (node) {
		temp = &node;
		node = node->next;
		xmlnode_free(*temp);
		g_free(*temp);
		*temp = NULL;
	}
}

#include "xmlnode.h"
#include "util.h"

xmlnode*
xmlnode_create(const gchar *name)
{
	xmlnode *node;

	g_return_val_if_fail(name != NULL, NULL);

	node = g_new0(xmlnode, 1);
	node->node = xmlNewNode(NULL, (xmlChar *)name);
	node->doc = NULL;
	node->is_root = 0;
	node->next = NULL;
	node->parent = NULL;
	node->child = NULL;
	node->name = g_strdup(name);

	return node;
}

xmlnode*
xmlnode_root(const gchar *xml_buf, gint size)
{
	xmlDoc *doc;
	xmlnode *node;

	g_return_val_if_fail(xml_buf != NULL, NULL);
	g_return_val_if_fail(size != 0, NULL);

	doc = xmlParseMemory(xml_buf, size);
	
	if (!doc) {
		hybrid_debug_error("xml", "parse xml");
		return NULL;
	}

	node = g_new0(xmlnode, 1);
	node->node = xmlDocGetRootElement(doc);
	node->doc = doc;
	node->is_root = 1;
	node->next = NULL;
	node->parent = NULL;
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
		hybrid_debug_error("xml", "parse xml file");
		return NULL;
	}

	node = g_new0(xmlnode, 1);
	node->node = xmlDocGetRootElement(doc);
	node->doc = doc;
	node->is_root = 1;
	node->parent = NULL;
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
		new->parent = node;
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

void
xmlnode_set_content(xmlnode *node, const gchar *content)
{
	g_return_if_fail(node != NULL);

	xmlNodeSetContent(node->node, BAD_CAST content);
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
	new->parent = node;
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

xmlnode*
xmlnode_add_child(xmlnode *parent, xmlnode *child)
{
	xmlNode *new_child;
	xmlnode *new;
	xmlnode *children;
	xmlnode *pos;

	g_return_val_if_fail(parent != NULL, NULL);
	g_return_val_if_fail(child != NULL, NULL);

	if (xmlDOMWrapCloneNode(NULL, child->doc, child->node, &new_child, 
			parent->doc, parent->node, 1, 0) != 0) {
		return NULL;
	}

	if (!(new_child = xmlAddChild(parent->node, new_child))) {
		return NULL;
	}

	new = g_new0(xmlnode, 1);
	new->node = new_child;
	new->doc = parent->doc;
	new->is_root = 0;
	new->next = NULL;
	new->child = NULL;
	new->parent = parent;
	new->name = g_strdup((gchar*)new_child->name);

	children = xmlnode_child(parent);

	if (children) {
		pos = children;
		while (pos) {
			if (pos->next == NULL) {
				pos->next = new;
				break;
			}

			pos = pos->next;
		}

	} else {
		parent->child = new;
	}

	return new;
}

void
xmlnode_remove_node(xmlnode *node)
{
	xmlnode *pos;
	xmlnode *list;
	xmlnode *rmnode;

	/* Remove the node from its parent's child list. */
	if (node->parent) {
		list = node->parent->child;
		if (list == node) {
			node->parent->child = node->next;

		} else {
			for (rmnode = list; rmnode; rmnode = rmnode->next) {
				if (rmnode->next == node) {
					rmnode->next = node->next;
					break;
				}
			}
		}
	}

	/* Remove its children recursively */
	for (pos = node->child; pos; pos = pos->next) {
		xmlnode_remove_node(pos);
	}

	/* Free the memory. */
	xmlUnlinkNode(node->node);
	
	g_free(node->name);
	g_free(node);
}

gchar*
xmlnode_to_string(xmlnode *root)
{
	gchar *res;
	xmlBuffer *buffer;

	g_return_val_if_fail(root != NULL, NULL);

	buffer = xmlBufferCreate();

	if (xmlNodeDump(buffer, NULL, root->node, 0, 0) == -1) {

		hybrid_debug_error("xml", "dump node error.");

		return NULL;
	}

	res = g_strdup((gchar*)xmlBufferContent(buffer));

	xmlBufferFree(buffer);

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
	g_return_val_if_fail(root != NULL, HYBRID_ERROR);
	g_return_val_if_fail(root->is_root == 1, HYBRID_ERROR);
	g_return_val_if_fail(filepath != NULL, HYBRID_ERROR);

	if (xmlSaveFormatFileEnc(filepath, root->doc, "UTF-8", 0) == -1) {
		return HYBRID_ERROR;
	}

	return HYBRID_OK;
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

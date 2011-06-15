#ifndef IM_XMLPARSER_H
#define IM_XMLPARSER_H
#include <glib.h>
#include <libxml/parser.h>

typedef struct _xmlnode xmlnode;

struct _xmlnode {
	xmlNode *node;
	xmlDoc  *doc;
	gint is_root;
	gchar *name;
	xmlnode *child;
	xmlnode *next;
};

#define xmlnode_next(n) ((n)->next)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the root element of the xml context.
 *
 * @param xml_buf XML data buffer.
 * @param size The number of bytes of the XML data buffer.
 *
 * @return The root xmlnode struct.
 */
xmlnode *xmlnode_root(const gchar *xml_buf, gint size);

/**
 * Get the children nodes of a xmlnode.
 *
 * @param node The node.
 *
 * @return The first of the children nodes.
 */
xmlnode *xmlnode_child(xmlnode *node);

/**
 * Find the xml node with the specified node name.
 *
 * @param node The start node.
 * @param name The name of the node to find.
 *
 * @return The node if found. NULL if there was an error. 
 */
xmlnode *xmlnode_find(xmlnode *node, const gchar *name);

/**
 * Get the value of a node to a specified attribute.
 *
 * @param node The node.
 * @param prop The attribute name.
 *
 * @return The value to the attribute.
 */
gchar *xmlnode_prop(xmlnode *node, const gchar *prop);

/**
 * Test whether the node has the specified attribute.
 *
 * @param node The node.
 * @param prop The attribute name.
 *
 * @return TRUE if has, orelse FALSE.
 */
gboolean xmlnode_has_prop(xmlnode *node, const gchar *prop);

/**
 * Get the content of a node.
 *
 * @param node The node.
 *
 * @return The content value.
 */
gchar *xmlnode_content(xmlnode *node);

/**
 * Create a new child node for the current node.
 *
 * @param node. The parent node of the node to be created.
 * @param childname. The name of the new node.
 *
 * @return The new node created.
 */
xmlnode *xmlnode_new_child(xmlnode *node, const gchar *childname);

/**
 * Generate a xml string with the xml root node context,
 * then free the xml context, no need to free it again.
 *
 * @param The xml node, and it must be a root node.
 *
 *  @return The xml string.
 */
gchar *xmlnode_to_string(xmlnode *root);

/**
 * Create a new attribute for a specified node.
 * 
 * @param node The node.
 * @param name The attribute name.
 * @param value The attribte value.
 */
void xmlnode_new_prop(xmlnode *node, const gchar *name, const gchar *value);

/**
 * Destroy the ROOT node, and destroy the xml context.
 *
 * @param node xmlnode to free.
 */
void xmlnode_free(xmlnode *node);

#ifdef __cplusplus
}
#endif

#endif

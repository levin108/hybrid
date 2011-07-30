/***************************************************************************
 *   Copyright (C) 2011 by levin                                           *
 *   levin108@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 ***************************************************************************/

#include "util.h"
#include "xmpp_parser.h"

static void
parse_text(void *user_data, const xmlChar *text, gint text_len)
{
	gchar *value;
	XmppStream *stream = (XmppStream *)user_data;

	if (!stream->node) {
		//hybrid_debug_error("xmpp", "invalid node: %s, %d\n", (gchar*)text, text_len);
		return;
	}

	value = g_strndup((gchar *)text, text_len);

	xmlnode_new_text_child(stream->node, value);

	g_free(value);
}

static void
parse_element_start(void *user_data,
				   const xmlChar *element_name, const xmlChar *prefix,
				   const xmlChar *namespace, gint nb_namespaces,
				   const xmlChar **namespaces, gint nb_attributes,
				   gint nb_defaulted, const xmlChar **attributes)
{
	gint i;
	XmppStream *stream = (XmppStream*)user_data;

	/* we should got <stream:stream id= */
	if (!stream->stream_id) {
		for (i = 0; i < nb_attributes * 5; i += 5) {

			if (xmlStrcmp(attributes[i], (xmlChar *)"id") == 0) {
				gint value_size;

				value_size = attributes[i + 4] - attributes[i + 3];
				stream->stream_id = 
					g_strndup((gchar *)attributes[i + 3], value_size);
			}
		}

		/* we don't found a id attribute in the initiate stream. */
		if (!stream->stream_id) {
			hybrid_debug_error("xmpp", "invalid initiate stream.");
			return;
		}
		
		return;
	}

	if (!stream->node) {
		stream->node = xmlnode_create((gchar *)element_name);

	} else {
		stream->node = xmlnode_new_child(stream->node, (gchar *)element_name);
	}

	xmlnode_set_prefix(stream->node, (gchar *)prefix);

	for (i = 0; i < nb_attributes * 5; i += 5) {

		gint value_size;
		gchar *name;
		gchar *value;

		value_size = attributes[i + 4] - attributes[i + 3];

		name = (gchar*)attributes[i];
		value = g_strndup((gchar *)attributes[i + 3], value_size);

		xmlnode_new_prop(stream->node, name, value);

		g_free(value);
	}

	for (i = 0; i < nb_namespaces; i ++) {
		xmlnode_new_namespace(stream->node,
				(gchar*)namespaces[i * 2], (gchar*)namespaces[i * 2 + 1]);
	}
}


static void
parse_element_end(void *user_data, const xmlChar *element_name,
				 const xmlChar *prefix, const xmlChar *namespace)
{
	XmppStream *stream;
	gchar *xml_string;

	stream = (XmppStream *)user_data;

	if (!stream->node) {
		hybrid_debug_error("xmpp",
				"got a end element but not found its start element.");
		return;
	}

	if (g_strcmp0(stream->node->name, (gchar *)element_name)) {

		hybrid_debug_error("xmpp", "invalid end element.");

		return;
	}

	if (stream->node->parent) {
		stream->node = stream->node->parent;

	} else {
		xml_string = xmlnode_to_string(stream->node);

		hybrid_debug_info("xmpp", "recv:\n%s", xml_string);

		xmpp_stream_process(stream, stream->node);

		xmlnode_free(stream->node);
		stream->node = NULL;

		g_free(xml_string);
	}
}


static xmlSAXHandler xmpp_xml_parser = {
	NULL,									/*internalSubset*/
	NULL,									/*isStandalone*/
	NULL,									/*hasInternalSubset*/
	NULL,									/*hasExternalSubset*/
	NULL,									/*resolveEntity*/
	NULL,									/*getEntity*/
	NULL,									/*entityDecl*/
	NULL,									/*notationDecl*/
	NULL,									/*attributeDecl*/
	NULL,									/*elementDecl*/
	NULL,									/*unparsedEntityDecl*/
	NULL,									/*setDocumentLocator*/
	NULL,									/*startDocument*/
	NULL,									/*endDocument*/
	NULL,									/*startElement*/
	NULL,									/*endElement*/
	NULL,									/*reference*/
	parse_text,                        		/*characters*/
	NULL,									/*ignorableWhitespace*/
	NULL,									/*processingInstruction*/
	NULL,									/*comment*/
	NULL,									/*warning*/
	NULL,									/*error*/
	NULL,									/*fatalError*/
	NULL,									/*getParameterEntity*/
	NULL,									/*cdataBlock*/
	NULL,									/*externalSubset*/
	XML_SAX2_MAGIC,							/*initialized*/
	NULL,									/*_private*/
	parse_element_start,	            	/*startElementNs*/
	parse_element_end,              		/*endElementNs*/
	NULL,                               	/*serror*/
};

void
xmpp_process_pushed(XmppStream *stream, const gchar *buffer, gint len)
{
	gint ret;

	if (stream->xml_ctxt) {

		if ((ret = xmlParseChunk(stream->xml_ctxt, buffer, len, 0))
				!= XML_ERR_OK) {

			xmlError *err = xmlCtxtGetLastError(stream->xml_ctxt);

			xmlErrorLevel level = XML_ERR_WARNING;

			if (err) {
				level = err->level;
			}

			switch (level) {
				case XML_ERR_NONE:
					hybrid_debug_info("xmpp",
							"xmlParseChunk returned info %i\n", ret);
					break;
				case XML_ERR_WARNING:
					hybrid_debug_error("xmpp",
							"xmlParseChunk returned warning %i\n", ret);
					break;
				case XML_ERR_ERROR:
					hybrid_debug_error("xmpp",
							"xmlParseChunk returned error %i\n", ret);
					break;
				case XML_ERR_FATAL:
					hybrid_debug_error("xmpp",
							"xmlParseChunk returned fatal %i\n", ret);
					break;
			}
		}

	} else {
		stream->xml_ctxt = xmlCreatePushParserCtxt(&xmpp_xml_parser,
							stream, buffer, len, NULL);
		xmlParseChunk(stream->xml_ctxt, "", 0, 0);
	}
}

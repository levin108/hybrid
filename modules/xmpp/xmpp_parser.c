#include "util.h"
#include "xmpp_parser.h"


static void
parse_element_start(void *user_data,
				   const xmlChar *element_name, const xmlChar *prefix,
				   const xmlChar *namespace, gint nb_namespaces,
				   const xmlChar **namespaces, gint nb_attributes,
				   gint nb_defaulted, const xmlChar **attributes)
{

	gint i;
	for (i = 0; i < nb_attributes * 5; i += 5) {
		printf("%s, %s, %s, %s\n", (gchar*)attributes[i], (gchar*)attributes[i+1], (gchar*)attributes[i+2], (gchar*)attributes[i+4]);
	}

	for (i = 0; i < nb_namespaces; i ++) {
		printf("%s, %s \n", (gchar*)namespaces[i * 2], (gchar*)namespaces[i * 2 + 1]);
	}
}


static void
parse_element_end(void *user_data, const xmlChar *element_name,
				 const xmlChar *prefix, const xmlChar *namespace)
{
	printf("=========== %s\n", element_name);
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
	NULL,                           		/*characters*/
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

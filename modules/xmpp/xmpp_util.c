#include "xmpp_util.h"

/**
 * Strip the '/' from a label,make it an unclosed xml label.
 */
void
xmpp_strip_end_label(gchar *xml_string)
{
	gint length;

	g_return_if_fail(xml_string != NULL);

	if ((length = strlen(xml_string)) < 2) {
		return;
	}

	if (xml_string[length - 1] == '>' && xml_string[length - 2] == '/') {
		xml_string[length - 2] = '>';
		xml_string[length - 1] = '\0';
	}
}


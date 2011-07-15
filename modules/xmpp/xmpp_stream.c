#include "xmpp_stream.h"

XmppStream*
xmpp_stream_create(void)
{
	XmppStream *stream;

	stream = g_new0(XmppStream, 1);

	stream->major_version = 1;
	stream->miner_version = 0;

	return stream;
}

void
xmpp_stream_destroy(XmppStream *stream)
{
	if (stream) {
		g_free(stream);
	}
}

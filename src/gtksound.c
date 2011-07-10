#include "util.h"
#include "gtksound.h"

#ifdef USE_GSTREAMER

#include <gst/gst.h>

static void
add_pad(GstElement *element , GstPad *pad , gpointer data)
{
	gchar *name;
	GstElement *sink = (GstElement*)data;

	name = gst_pad_get_name(pad);
	gst_element_link_pads(element, name, sink, "sink");
	g_free(name);
}

void
hybrid_sound_play_file(const gchar *filename) 
{

	GstElement *pipeline;
	GstElement *source, *parser, *sink;

	pipeline = gst_pipeline_new("audio-player");

	source = gst_element_factory_make("filesrc" , "source");

	if (!source) {
		g_warning("make filesrc element failed");
		return;
	}

	parser = gst_element_factory_make("wavparse" , "parser");

	if (!parser) {
		g_warning("make wavparse element failed");
		return;
	}

	sink = gst_element_factory_make("alsasink" , "output");

	if (!sink) {
		g_warning("make alsasink element failed");
		return;
	}

	g_object_set(G_OBJECT(source), "location",
			filename , NULL);

	gst_bin_add_many(GST_BIN(pipeline), source,
			parser , sink , NULL);

	g_signal_connect(parser, "pad-added",
			G_CALLBACK(add_pad) , sink);

	if (!gst_element_link(source , parser)) {
		g_warning("linke source to parser failed");
	}

	gst_element_set_state(pipeline , GST_STATE_PLAYING);

	g_object_unref(pipeline);
}

void
hybrid_sound_init(gint argc, gchar **argv)
{
	gst_init(&argc, &argv);
	return;
}

#else /* USE_GSTREAMER */

void
hybrid_sound_play_file(const gchar *filename)
{
	return;
}

void
hybrid_sound_init(gint argc, gchar **argv)
{
	return;
}

#endif /* USE_GSTREAMER */

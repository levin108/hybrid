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
#include "pref.h"
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

static gboolean
bus_call (GstBus     *bus,
          GstMessage *msg,
          gpointer    data)
{
    GstElement *pipeline = data;
    
    switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_EOS:
        gst_element_set_state(pipeline, GST_STATE_NULL);
        g_object_unref(pipeline);
        break;
    case GST_MESSAGE_ERROR: {
        gchar  *debug = NULL;
        GError *err   = NULL;

        gst_message_parse_error (msg, &err, &debug);

        hybrid_debug_error("sound", "Error: %s", err->message);
        g_error_free (err);

        if (debug) {
            hybrid_debug_error("sound", "Debug details: %s\n", debug);
            g_free (debug);
        }
        gst_element_set_state(pipeline, GST_STATE_NULL);
        g_object_unref(pipeline);

        break;
    }
    default:
        break;
    }

    return TRUE;
}

void
hybrid_sound_play_file(const gchar *filename)
{
    GstElement *source, *parser, *sink;
    GstElement *pipeline;
    GstBus     *bus;

    if (hybrid_pref_get_boolean(NULL, "mute")) {
        return;
    }

    pipeline = gst_pipeline_new("audio-player");

    bus = gst_pipeline_get_bus(GST_PIPELINE (pipeline));
    gst_bus_add_watch(bus, bus_call, pipeline);
    gst_object_unref(bus);

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

    if (!G_IS_OBJECT(source)) {
        g_warning("sound source is not a gtk object.");
        return;
    }

    g_object_set(G_OBJECT(source), "location",
            filename , NULL);

    gst_bin_add_many(GST_BIN(pipeline), source, parser , sink , NULL);

    g_signal_connect(parser, "pad-added",
            G_CALLBACK(add_pad) , sink);

    if (!gst_element_link(source , parser)) {
        g_warning("linke source to parser failed");
    }

    gst_element_set_state(pipeline , GST_STATE_PLAYING);
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

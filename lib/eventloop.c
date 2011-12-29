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

#include "eventloop.h"

typedef struct _EventData EventData;

struct _EventData {
    gint       fd;
    guint      result;
    input_func handler;
    gpointer   user_data;
};

static gboolean
read_event_cb(GIOChannel *source, GIOCondition condition, gpointer user_data)
{
    gboolean   ret;
    EventData *rv = (EventData*)user_data;

    ret = rv->handler(rv->fd, rv->user_data);

    if (!ret) {
        g_source_remove(rv->result);
    }

    return ret;
}

guint
hybrid_event_add(gint sk, gint event_type, input_func func, gpointer user_data)
{
    g_return_val_if_fail(sk != 0, -1);

    GIOChannel *channel;
    EventData  *data = g_new0(EventData, 1);

    data->fd        = sk;
    data->handler   = func;
    data->user_data = user_data;

    channel = g_io_channel_unix_new(sk);
    g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);

    data->result = g_io_add_watch_full(channel, G_PRIORITY_LOW,
                                       event_type, read_event_cb, data, g_free);

    g_io_channel_unref(channel);

    return data->result;
}

gboolean
hybrid_event_remove(guint source)
{
    return g_source_remove(source);
}

static gboolean
ssl_recv_cb(gint sk, gpointer user_data)
{
    HybridSslConnection *isc = (HybridSslConnection*)user_data;
    return isc->recv_cb(isc, isc->recv_data);
}

guint
hybrid_ssl_event_add(HybridSslConnection *isc, ssl_callback func,
                     gpointer user_data)
{
    g_return_val_if_fail(isc != NULL, 0);
    g_return_val_if_fail(func != NULL, 0);

    isc->recv_cb   = func;
    isc->recv_data = user_data;

    return hybrid_event_add(isc->sk, HYBRID_EVENT_READ, ssl_recv_cb, isc);
}

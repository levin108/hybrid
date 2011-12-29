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

#ifndef HYBRID_CONNECT_H
#define HYBRID_CONNECT_H

#include <glib.h>
#include <openssl/ssl.h>

typedef struct _HybridConnection HybridConnection;
typedef struct _HybridSslConnection HybridSslConnection;

typedef gboolean (*connect_callback)(gint sk, gpointer user_data);
typedef gboolean (*ssl_callback)(HybridSslConnection *ssl_conn, gpointer user_data);

#define SSL_CALLBACK(ssl)   ((ssl_callback)ssl)
#define CONN_CALLBACK(conn) ((conn_callback)conn)

#include "eventloop.h"

struct _HybridConnection {
	gint   sk;
	gchar *host;
	gint   port;
};

struct _HybridSslConnection {
	gint              sk;
	HybridConnection *conn;

	ssl_callback conn_cb;
	gpointer     conn_data;

	ssl_callback recv_cb;
	gpointer     recv_data;

	BIO     *wbio;
	BIO     *rbio;
	SSL     *ssl;
	SSL_CTX *ssl_ctx;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Make a TCP connection to the specified host and port.
 *
 * @param hostname Hostname of the destination host.
 * @param port Port of the destination host.
 * @param func Callback function to call when connection
 *        is established.
 * @param user_data User-specified data.
 *
 * @return HybridConncetion if success, which can be used to cancel
 *         the pending connection, destroy it if don't use or after use.
 *         NULL if there was an error.
 */
HybridConnection* hybrid_proxy_connect(const gchar *hostname, gint port,
                                       connect_callback func, gpointer user_data);

/**
 * Make a SSL connection to the specified host and port.
 *
 * @param hostname  Hostname of the destination host.
 * @param port      Port of the destination host.
 * @param func      Callback function to call when connection
 *                  is established.
 * @param user_data User-specified data.
 *
 * @return HybridSslConncetion if success, which can be used to cancel
 *         the pending connection, destroy it if don't use or after use.
 *         NULL if there was an error.
 */
HybridSslConnection* hybrid_ssl_connect(const gchar *hostname, gint port,
                                        ssl_callback func, gpointer user_data);

/**
 * Make a SSL hand-shake on the established tcp connection.
 *
 * @param sk        Socket file discriptor of the established tcp connection.
 * @param func      Callback function to call when connection
 *                  is established.
 * @param user_data User-specified data.
 *
 * @return HybridSslConncetion if success, which can be used to cancel
 *         the pending connection, destroy it if don't use or after use.
 *         NULL if there was an error.
 */
HybridSslConnection* hybrid_ssl_connect_with_fd(gint sk,
                                                ssl_callback func, gpointer user_data);

/**
 * Write data to a SSL connection.
 *
 * @param ssl SSL connection context.
 * @buf Data buffer to send data from.
 * @len Number of bytes to send from buffer.
 *
 * @return The number of bytes written to the connection.
 */
gint hybrid_ssl_write(HybridSslConnection *ssl, const gchar *buf, gint len);

/**
 * Read data from a SSL connection.
 *
 * @param ssl SSL connection context.
 * @param buf Data buffer to read data from.
 * @param len Number of bytes to read from buffer.
 *
 * @retutn The number of bytes read from the connection.
 */
gint hybrid_ssl_read(HybridSslConnection *ssl, gchar *buf, gint len);

/**
 * Destroy a connection, free the memory and close the socket.
 *
 * @param conn Connection to destroy.
 */
void hybrid_connection_destroy(HybridConnection *conn);

/**
 * Destroy a SSL connection, free the memory and close the socket.
 *
 * @param conn Connection to destroy.
 */
void hybrid_ssl_connection_destory(HybridSslConnection *conn);


/**
 * Parse the http response code from the response string.
 *
 * @param http_response The response string from the server.
 *
 * @return The code.
 */
gint hybrid_get_http_code(const gchar *http_response);

/**
 * Parse the body length of the http response string.
 *
 * @param http_response The response string from the server.
 *
 * @return The length.
 */
gint hybrid_get_http_length(const gchar *http_response);


#ifdef __cplusplus
}
#endif

#endif /* HYBRID_CONNECT_H */

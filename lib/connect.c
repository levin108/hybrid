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

#include <glib.h>
#include <gtk/gtk.h>

#include "util.h"
#include "network.h"
#include "eventloop.h"
#include "connect.h"

#include <unistd.h>
#include <fcntl.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

/**
 * set the socket to be nonblock
 */
static gint
nonblock(gint sk)
{
	gint flag;

	g_return_val_if_fail(sk != 0, HYBRID_ERROR);

	hybrid_debug_info("option", "set socket to be nonblock");

	if ((flag = fcntl(sk, F_GETFL, 0)) == -1) {
		hybrid_debug_error("socket", "set socket to be nonblock:%s",
				strerror(errno));
		return HYBRID_ERROR;
	}

	if ((flag = fcntl(sk, F_SETFL, flag | O_NONBLOCK)) == -1) {
		hybrid_debug_error("socket", "set socket to be nonblock:%s",
				strerror(errno));
		return HYBRID_ERROR;
	}

	return HYBRID_OK;
}

/**
 * initialize the sockaddr struct
 */
static gint
addr_init(const gchar *hostname, gint port, struct sockaddr *addr)
{
	gchar host_ip[IPADDR_LEN];
	struct sockaddr_in *addr_in = (struct sockaddr_in*)addr;

	memset(host_ip, 0, sizeof(host_ip));
	if (resolve_host(hostname, host_ip) == HYBRID_ERROR) {
		hybrid_debug_error("connect", "connect terminate due to bad hostname");
		return HYBRID_ERROR;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr_in->sin_family = AF_INET;
	addr_in->sin_addr.s_addr = inet_addr(host_ip);
	addr_in->sin_port = htons(port);

	return HYBRID_OK;
}

HybridConnection*
hybrid_proxy_connect(const gchar *hostname, gint port, connect_callback func,
		gpointer user_data)
{
	gint sk;
	struct sockaddr addr;
	HybridConnection *conn;

	g_return_val_if_fail(port != 0, NULL);
	g_return_val_if_fail(hostname != NULL, NULL);

	hybrid_debug_info("connect", "connecting to %s:%d", hostname, port);

	conn = g_new0(HybridConnection, 1);

	if ((sk = socket(AF_INET, SOCK_STREAM, 0)) == -1) {

		hybrid_debug_error("connect", "create socket: %s", strerror(errno));
		hybrid_connection_destroy(conn);

		return NULL;
	}

	if (nonblock(sk) != HYBRID_OK) {

		hybrid_connection_destroy(conn);
		return NULL;
	}

	if (addr_init(hostname, port, &addr) != HYBRID_OK) {

		hybrid_connection_destroy(conn);
		return NULL;
	}

	if (connect(sk, &addr, sizeof(addr)) != 0) {

		if (errno != EINPROGRESS) {

			hybrid_debug_error("connect", "connect to \'%s:%d\':%s", hostname,
					port, strerror(errno));
			hybrid_connection_destroy(conn);

			return NULL;
		}

		hybrid_debug_info("connect", "connect in progress");

		hybrid_event_add(sk, HYBRID_EVENT_WRITE, func, user_data);

	} else {
		/* connection establish imediately */
		func(sk, user_data);
	}

	conn->sk = sk;
	conn->host = g_strdup(hostname);
	conn->port = port;
	
	return conn;
}

/**
 * Callback function to process the SSL handshake.
 */
static gboolean
ssl_connect_cb(gint sk, gpointer user_data)
{
	gint l;
	gboolean res;
	HybridSslConnection *ssl_conn = (HybridSslConnection*)user_data;

	if (!SSL_set_fd(ssl_conn->ssl, sk)) {

		hybrid_debug_error("ssl", "add ssl to tcp socket:%s", 
				ERR_reason_error_string(ERR_get_error()));
		return FALSE;
	}

	ssl_conn->sk = sk;

	SSL_set_connect_state(ssl_conn->ssl);

	RAND_poll();

	while (RAND_status() == 0) {
		unsigned short rand_ret = rand() % 65536;
		RAND_seed(&rand_ret, sizeof(rand_ret));
	} 

	for ( ;; ) {
		l = SSL_connect(ssl_conn->ssl);

		switch (SSL_get_error(ssl_conn->ssl, l)) { 
			case SSL_ERROR_NONE:
				goto ssl_ok;
			case SSL_ERROR_WANT_WRITE:
			case SSL_ERROR_WANT_READ:
				continue;
			case SSL_ERROR_SYSCALL:
			case SSL_ERROR_WANT_X509_LOOKUP:
			case SSL_ERROR_ZERO_RETURN:
			case SSL_ERROR_SSL:
			default:
				ERR_print_errors_fp(stderr);
				goto ssl_err;
				break;
		}

	}
ssl_ok:
	res = ssl_conn->conn_cb(ssl_conn, ssl_conn->conn_data);

	return res;
ssl_err:
	return FALSE;
}

HybridSslConnection* 
hybrid_ssl_connect(const gchar *hostname, gint port, ssl_callback func,
		gpointer user_data)
{
	HybridSslConnection *conn;

	g_return_val_if_fail(hostname != NULL, NULL);
	g_return_val_if_fail(port != 0, NULL);
	g_return_val_if_fail(func != NULL, NULL);

	//SSLeay_add_all_algorithms();
	SSL_load_error_strings();
	SSL_library_init();

	conn = g_new0(HybridSslConnection, 1);

	if (!(conn->ssl_ctx = SSL_CTX_new(SSLv23_client_method()))) {

		hybrid_debug_error("ssl", "initialize SSL CTX: %s",
				ERR_reason_error_string(ERR_get_error()));
		hybrid_ssl_connection_destory(conn);

		return NULL;
	}

	if (!(conn->ssl = SSL_new(conn->ssl_ctx))) {

		hybrid_debug_error("ssl", "create SSl:%s",
				ERR_reason_error_string(ERR_get_error()));
		hybrid_ssl_connection_destory(conn);

		return NULL;
	}

	conn->conn_cb = func;
	conn->conn_data = user_data;

	conn->conn = hybrid_proxy_connect(hostname, port, ssl_connect_cb, conn);

	return conn;
}

HybridSslConnection*
hybrid_ssl_connect_with_fd(gint sk,	ssl_callback func, gpointer user_data)
{
	gint l;
	SSL *ssl;
	SSL_CTX *ssl_ctx;
	HybridSslConnection *ssl_conn;

	SSL_load_error_strings();
	SSL_library_init();

	if (!(ssl_ctx = SSL_CTX_new(SSLv3_client_method()))) {

		hybrid_debug_error("ssl", "initialize SSL CTX: %s",
				ERR_reason_error_string(ERR_get_error()));

		return NULL;
	}

	if (!(ssl = SSL_new(ssl_ctx))) {

		hybrid_debug_error("ssl", "create SSl:%s",
				ERR_reason_error_string(ERR_get_error()));

		return NULL;
	}

	if (!SSL_set_fd(ssl, sk)) {

		hybrid_debug_error("ssl", "add ssl to tcp socket:%s", 
				ERR_reason_error_string(ERR_get_error()));
		return NULL;
	}

	SSL_set_connect_state(ssl);

	RAND_poll();

	while (RAND_status() == 0) {
		unsigned short rand_ret = rand() % 65536;
		RAND_seed(&rand_ret, sizeof(rand_ret));
	} 

	for ( ;; ) {
		l = SSL_connect(ssl);

		switch (SSL_get_error(ssl, l)) { 
			case SSL_ERROR_NONE:
				goto ssl_conn_sk_ok;
			case SSL_ERROR_WANT_WRITE:
			case SSL_ERROR_WANT_READ:
				continue;
			case SSL_ERROR_SYSCALL:
			case SSL_ERROR_WANT_X509_LOOKUP:
			case SSL_ERROR_ZERO_RETURN:
			case SSL_ERROR_SSL:
			default:
				hybrid_debug_error("ssl", "ssl hand-shake error:%s",
					ERR_reason_error_string(ERR_get_error()));
				return NULL;
		}
	}

ssl_conn_sk_ok:

	ssl_conn = g_new0(HybridSslConnection, 1);

	ssl_conn->sk = sk;
	ssl_conn->ssl = ssl;
	ssl_conn->ssl_ctx = ssl_ctx;
	ssl_conn->conn_cb = func;
	ssl_conn->conn_data = user_data;

	if (func) {
		func(ssl_conn, user_data);
	}

	return ssl_conn;
}

gint
hybrid_ssl_write(HybridSslConnection *ssl, const gchar *buf, gint len)
{
	gint l;
	gint i;
	gint ret;
	fd_set write_fd;

	/*
	for (i = 0; len > 0; i ++) {
		while (1) {
			l = SSL_write(ssl->ssl, buf + i * 4096, len > 4096 ? 4096 : len);

			switch (SSL_get_error(ssl->ssl, l)) { 
				case SSL_ERROR_NONE:
					len -= 4096;
					goto ssl_write_ok;
				case SSL_ERROR_WANT_WRITE:
					g_usleep(100);
					continue;
				default:
					return -1;
			}
		}
ssl_write_ok:;
	}
	*/

	for (i = 0; len > 0; i ++) {

reselect:
		FD_ZERO(&write_fd);
		FD_SET(ssl->sk, &write_fd);

		/* use select() to determine when to write. */
		if ((ret = select (ssl->sk + 1, NULL, &write_fd, NULL, NULL)) > 0) {

			if (FD_ISSET(ssl->sk, &write_fd)) {

				l = SSL_write(ssl->ssl, buf + i * 4096, len > 4096 ? 4096 : len);

				/* hybrid_debug_error("ssl", "ssl write with ret: %d\n", ret); */

				switch (SSL_get_error(ssl->ssl, l)) { 
				case SSL_ERROR_NONE:
					len -= 4096;
					break;
				case SSL_ERROR_WANT_WRITE:
					hybrid_debug_error("ssl", 
									   "ssl write with SSL_ERROR_WANT_WRITE.");
					return -1;
				case SSL_ERROR_SYSCALL:
					hybrid_debug_error("ssl", 
									   "ssl write with SSL_ERROR_SYSCALL.");
					if (!ret) {
						return -1;
					} else {
						break;
					}
				case SSL_ERROR_WANT_X509_LOOKUP:
					hybrid_debug_error("ssl", 
									   "ssl write with SSL_ERROR_WANT_X509_LOOKUP.");
					return -1;
				case SSL_ERROR_ZERO_RETURN:
					hybrid_debug_error("ssl", 
									   "ssl write with SSL_ERROR_ZERO_RETURN.");
					return -1;
				case SSL_ERROR_SSL:
				default:
					hybrid_debug_error("ssl",
									   "ssl write with other error.");
					return -1;

				}
			} else {
				hybrid_debug_error("ssl", "ssl write no fd selected.");
				return -1;
			}			
		} else if (0 == ret) {
			goto reselect;
		} else {
			hybrid_debug_error("ssl", "ssl write error, select() return -1.");
			return -1;
		}
		
	}


	return len;
}

gint
hybrid_ssl_read(HybridSslConnection *ssl, gchar *buf, gint len)
{
	return SSL_read(ssl->ssl, buf, len);
}

void 
hybrid_connection_destroy(HybridConnection *conn)
{
	if (conn) {
		g_free(conn->host);
		g_free(conn);
	}

}

void 
hybrid_ssl_connection_destory(HybridSslConnection *conn)
{
	if (conn) {
		SSL_free(conn->ssl);
		SSL_CTX_free(conn->ssl_ctx);
		hybrid_connection_destroy(conn->conn);
		g_free(conn);
	}
}

gint 
hybrid_get_http_code(const gchar *http_response)
{
	gchar *pos;
	gchar *code_start = NULL, *code_stop = NULL;
	gchar *temp;
	gint code;

	g_return_val_if_fail(http_response != NULL, 0);

	pos = (gchar*)http_response;

	while (*pos) {
		if (*pos == ' ') {
			if (code_start == NULL) {
				code_start = pos;

			} else {
				code_stop = pos;
				break;
			}
		}
		
		pos ++;
	}

	if (!code_start || !code_stop) {
		hybrid_debug_error("http", "unknown http response");
		return 0;
	}

	temp = g_strndup(code_start, code_stop - code_start);
	code = atoi(temp);
	g_free(temp);

	return code;
}


gint 
hybrid_get_http_length(const gchar *http_response)
{
	gchar *pos, *stop;
	gchar *temp;
	gint length;
	const gchar *cur = "Content-Length: ";

	g_return_val_if_fail(http_response != NULL, 0);

	if (!(pos = g_strrstr(http_response, cur))) {
		hybrid_debug_error("http", "no Content-length in response header.");
		return 0;
	}

	pos += strlen(cur);

	for (stop = pos; *stop && *stop != '\r'; stop ++);

	temp = g_strndup(pos, stop - pos);
	length = atoi(temp);
	g_free(temp);

	return length;
}

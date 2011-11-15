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
#include "config.h"

#include <unistd.h>
#include <fcntl.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

static gint		ssl_verify_certs(SSL *ssl);

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
	addr_in->sin_family		 = AF_INET;
	addr_in->sin_addr.s_addr = inet_addr(host_ip);
	addr_in->sin_port		 = htons(port);

	return HYBRID_OK;
}

HybridConnection*
hybrid_proxy_connect(const gchar *hostname, gint port, connect_callback func,
		gpointer user_data)
{
	gint				 sk;
	struct sockaddr		 addr;
	HybridConnection	*conn;

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

	conn->sk   = sk;
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
	gint				 l;
	gboolean			 res;
	HybridSslConnection *ssl_conn = (HybridSslConnection*)user_data;

	if (!SSL_set_fd(ssl_conn->ssl, sk)) {

		hybrid_debug_error("ssl", "add ssl to tcp socket:%s", 
						   ERR_reason_error_string(ERR_get_error()));
		return FALSE;
	}

	ssl_conn->sk = sk;
	SSL_set_connect_state(ssl_conn->ssl);

 ssl_reconnect:
	l = SSL_connect(ssl_conn->ssl);

	switch (SSL_get_error(ssl_conn->ssl, l)) { 
	case SSL_ERROR_NONE:
		goto ssl_ok;
	case SSL_ERROR_WANT_WRITE:
	case SSL_ERROR_WANT_READ:
		usleep(100);
		goto ssl_reconnect;
	case SSL_ERROR_SYSCALL:
	case SSL_ERROR_WANT_X509_LOOKUP:
	case SSL_ERROR_ZERO_RETURN:
	case SSL_ERROR_SSL:
	default:
		ERR_print_errors_fp(stderr);
		goto ssl_err;
		break;
	}

ssl_ok:
	if (HYBRID_OK != ssl_verify_certs(ssl_conn->ssl)) {
		return FALSE;
	}

	res = ssl_conn->conn_cb(ssl_conn, ssl_conn->conn_data);
	return res;
	
ssl_err:
	return FALSE;
}

/**
 * Create a new SSL struct with certificates loaded.
 */
static SSL*
ssl_new_with_certs(SSL_CTX *ctx)
{
	SSL *ssl;
	
	g_return_val_if_fail(ctx != NULL, NULL);
	
	if(SSL_CTX_use_certificate_file(ctx, CCERT_DIR"/client_cert",
									SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
	}
 
	if(SSL_CTX_use_PrivateKey_file(ctx, CCERT_DIR"/client_key",
								   SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
	}

    if (!SSL_CTX_load_verify_locations(ctx, NULL, CERTS_DIR)) {
		ERR_print_errors_fp(stderr);
	}

	if (!(ssl = SSL_new(ctx))) {
		hybrid_debug_error("ssl", "create SSl:%s",
						   ERR_reason_error_string(ERR_get_error()));
		return NULL;
	}

	return ssl;

}

/**
 * Verify the peer's	certificate, if successfully verified,
 * save the peer's certificate
 */
static gint
ssl_verify_certs(SSL *ssl)
{
	FILE		*f;
	X509		*x;
	gchar		*cert_path;
	gchar		*cert_file;
	gchar		 buf[256];
	gchar		*pos;

	hybrid_debug_info("ssl", "verifying the peer's certificate");
	
	if ((x = SSL_get_peer_certificate(ssl)) != NULL) {
		if (SSL_get_verify_result(ssl)		== X509_V_OK) {

			X509_NAME_oneline(X509_get_subject_name(x), buf, 256);

			hybrid_debug_error("ssl", "client verification succeeded.");
			
			cert_path = hybrid_config_get_cert_path();
			
			if ((pos = strstr(buf, "CN="))) {
				cert_file = g_strdup_printf("%s/%s", cert_path, pos + 3);
			} else {
				cert_file = g_strdup_printf("%s/%s", cert_path, buf);
			}
			
			g_free(cert_path);
			
			f = fopen(cert_file, "w+");
			PEM_write_X509(f, x);
			fclose(f);
			g_free(cert_file);
			
		} else {
			hybrid_debug_error("ssl", "client verification failed.");
			return HYBRID_ERROR;
		}
	} else {
		hybrid_debug_error("ssl", "the peer certificate was not presented.\n");
	}

	return HYBRID_OK;
}

HybridSslConnection* 
hybrid_ssl_connect(const gchar *hostname, gint port, ssl_callback func,
		gpointer user_data)
{
	BIO					*buf_io;
	BIO					*ssl_bio;
	HybridSslConnection *conn;

	g_return_val_if_fail(hostname != NULL, NULL);
	g_return_val_if_fail(port != 0, NULL);
	g_return_val_if_fail(func != NULL, NULL);

	SSLeay_add_all_algorithms();
	SSL_load_error_strings();
	SSL_library_init();

	conn = g_new0(HybridSslConnection, 1);

	if (!(conn->ssl_ctx = SSL_CTX_new(SSLv23_client_method()))) {
		hybrid_debug_error("ssl", "initialize SSL CTX: %s",
						   ERR_reason_error_string(ERR_get_error()));
		hybrid_ssl_connection_destory(conn);
		return NULL;
	}

	if (!(conn->ssl = ssl_new_with_certs(conn->ssl_ctx))) {
		hybrid_ssl_connection_destory(conn);
		return NULL;
	}

	SSL_set_mode(conn->ssl, SSL_MODE_AUTO_RETRY);

	buf_io	= BIO_new(BIO_f_buffer());
	ssl_bio = BIO_new(BIO_f_ssl());

	BIO_set_ssl(ssl_bio, conn->ssl, BIO_NOCLOSE);
	BIO_push(buf_io, ssl_bio);
	
	conn->conn_cb	= func;
	conn->conn_data = user_data;
	conn->rbio		= buf_io;

	conn->conn = hybrid_proxy_connect(hostname, port, ssl_connect_cb, conn);

	return conn;
}

HybridSslConnection*
hybrid_ssl_connect_with_fd(gint sk,	ssl_callback func, gpointer user_data)
{
	gint				 l;
	SSL					*ssl;
	BIO					*sbio;
	BIO					*buf_io;
	BIO					*ssl_bio;
	SSL_CTX				*ssl_ctx;
	HybridSslConnection *ssl_conn;

	SSL_load_error_strings();
	SSL_library_init();

	if (!(ssl_ctx = SSL_CTX_new(TLSv1_client_method()))) {

		hybrid_debug_error("ssl", "initialize SSL CTX: %s",
						   ERR_reason_error_string(ERR_get_error()));
		return NULL;
	}

	if (!(ssl = ssl_new_with_certs(ssl_ctx))) {
		return NULL;
	}

	if (!SSL_set_fd(ssl, sk)) {
		hybrid_debug_error("ssl", "add ssl to tcp socket:%s", 
						   ERR_reason_error_string(ERR_get_error()));
		return NULL;
	}

	sbio = BIO_new(BIO_s_socket());
	BIO_set_fd(sbio, sk, BIO_NOCLOSE);
	SSL_set_bio(ssl, sbio, sbio);

	SSL_set_connect_state(ssl);

 reconnect:
	l = SSL_connect(ssl);

	switch (SSL_get_error(ssl, l)) { 
	case SSL_ERROR_NONE:
		goto ssl_conn_sk_ok;
	case SSL_ERROR_WANT_WRITE:
	case SSL_ERROR_WANT_READ:
		usleep(100);
		goto reconnect;
	case SSL_ERROR_SYSCALL:
	case SSL_ERROR_WANT_X509_LOOKUP:
	case SSL_ERROR_ZERO_RETURN:
	case SSL_ERROR_SSL:
	default:
		hybrid_debug_error("ssl", "ssl hand-shake error:%s",
						   ERR_reason_error_string(ERR_get_error()));
		return NULL;
	}
	
ssl_conn_sk_ok:

	if (HYBRID_OK != ssl_verify_certs(ssl)) {
		return NULL;
	}

	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
	
	buf_io = BIO_new(BIO_f_buffer());
	ssl_bio = BIO_new(BIO_f_ssl());

	BIO_set_ssl(ssl_bio, ssl, BIO_NOCLOSE);
	BIO_push(buf_io, ssl_bio);

	ssl_conn = g_new0(HybridSslConnection, 1);

	ssl_conn->sk		= sk;
	ssl_conn->ssl		= ssl;
	ssl_conn->ssl_ctx	= ssl_ctx;
	ssl_conn->conn_cb	= func;
	ssl_conn->conn_data = user_data;
	ssl_conn->rbio		= buf_io;
	ssl_conn->wbio		= sbio;

	if (func) {
		func(ssl_conn, user_data);
	}

	return ssl_conn;
}

gint
hybrid_ssl_write(HybridSslConnection *ssl, const gchar *buf, gint len)
{
	gint l;
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

rewrite:
	l = SSL_write(ssl->ssl, buf, len);

	switch (SSL_get_error(ssl->ssl, l)) { 
	case SSL_ERROR_NONE:
		if (l != len) {
			hybrid_debug_error("ssl", 
							   "ssl write %d bytes but only %d byte succeed.");
			return -1;
		}
		return l;
	case SSL_ERROR_WANT_WRITE:
		hybrid_debug_error("ssl", 
						   "ssl write with SSL_ERROR_WANT_WRITE.");
		usleep(100);
		goto rewrite;
	case SSL_ERROR_SYSCALL:
		hybrid_debug_error("ssl", 
						   "ssl write with io error.");
		return -1;
	case SSL_ERROR_WANT_X509_LOOKUP:
		hybrid_debug_error("ssl", 
						   "ssl write with SSL_ERROR_WANT_X509_LOOKUP.");
		return -1;
	case SSL_ERROR_ZERO_RETURN:
		hybrid_debug_error("ssl", 
						   "ssl connection permaturely closed.");
		return 0;
	case SSL_ERROR_SSL:
	default:
		ERR_print_errors_fp(stderr);
		hybrid_debug_error("ssl",
						   "ssl write with other error.");
		return -1;
	}

	return len;
}

gint
hybrid_ssl_read(HybridSslConnection *ssl, gchar *buf, gint len)
{
	gint ret;

	hybrid_debug_info("ssl", "start ssl read.");
	
	ret = BIO_read(ssl->rbio, buf, len);

	switch (SSL_get_error(ssl->ssl, ret)) { 
	case SSL_ERROR_NONE:
		hybrid_debug_info("ssl", "ssl read %d bytes, success!", ret);
		return ret;
	case SSL_ERROR_WANT_WRITE:
	case SSL_ERROR_WANT_READ:
		hybrid_debug_info("ssl", "ssl read WANT RW with ret %d.", ret);
		break;
	case SSL_ERROR_SYSCALL:
		hybrid_debug_info("ssl", "ssl read io error.");
		return -1;
	case SSL_ERROR_WANT_X509_LOOKUP:
		hybrid_debug_info("ssl", "ssl read X509.");
		return -1;
	case SSL_ERROR_ZERO_RETURN:
		hybrid_debug_info("ssl", "ssl connection permaturely closed.");
		return 0;
	case SSL_ERROR_SSL:
	default:
		hybrid_debug_error("ssl", "ssl read error:%s",
						   ERR_reason_error_string(ERR_get_error()));
		return -1;
	}

	return -1;
}

void 
hybrid_connection_destroy(HybridConnection *conn)
{
	if (conn) {
		g_free(conn->host);
		g_free(conn);
		if (conn->sk > 0) {
			close(conn->sk);
		}
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

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

	g_return_val_if_fail(sk != 0, HYBIRD_ERROR);

	hybird_debug_info("option", "set socket to be nonblock");

	if ((flag = fcntl(sk, F_GETFL, 0)) == -1) {
		hybird_debug_error("socket", "set socket to be nonblock:%s",
				strerror(errno));
		return HYBIRD_ERROR;
	}

	if ((flag = fcntl(sk, F_SETFL, flag | O_NONBLOCK)) == -1) {
		hybird_debug_error("socket", "set socket to be nonblock:%s",
				strerror(errno));
		return HYBIRD_ERROR;
	}

	return HYBIRD_OK;
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
	if (resolve_host(hostname, host_ip) == HYBIRD_ERROR) {
		hybird_debug_error("connect", "connect terminate due to bad hostname");
		return HYBIRD_ERROR;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr_in->sin_family = AF_INET;
	addr_in->sin_addr.s_addr = inet_addr(host_ip);
	addr_in->sin_port = htons(port);

	return HYBIRD_OK;
}

HybirdConnection*
hybird_proxy_connect(const gchar *hostname, gint port, connect_callback func,
		gpointer user_data)
{
	gint sk;
	struct sockaddr addr;
	HybirdConnection *conn;

	g_return_val_if_fail(port != 0, NULL);
	g_return_val_if_fail(hostname != NULL, NULL);

	hybird_debug_info("connect", "connecting to %s:%d", hostname, port);

	conn = g_new0(HybirdConnection, 1);

	if ((sk = socket(AF_INET, SOCK_STREAM, 0)) == -1) {

		hybird_debug_error("connect", "create socket: %s", strerror(errno));
		hybird_connection_destroy(conn);

		return NULL;
	}

	if (nonblock(sk) != HYBIRD_OK) {

		hybird_connection_destroy(conn);
		return NULL;
	}

	if (addr_init(hostname, port, &addr) != HYBIRD_OK) {

		hybird_connection_destroy(conn);
		return NULL;
	}

	if (connect(sk, &addr, sizeof(addr)) != 0) {

		if (errno != EINPROGRESS) {

			hybird_debug_error("connect", "connect to \'%s:%d\':%s", hostname,
					port, strerror(errno));
			hybird_connection_destroy(conn);

			return NULL;
		}

		hybird_debug_info("connect", "connect in progress");

		hybird_event_add(sk, HYBIRD_EVENT_WRITE, func, user_data);

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
	HybirdSslConnection *ssl_conn = (HybirdSslConnection*)user_data;

	if (!SSL_set_fd(ssl_conn->ssl, sk)) {

		hybird_debug_error("ssl", "add ssl to tcp socket:%s", 
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

		g_usleep(100000);

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

HybirdSslConnection* 
hybird_ssl_connect(const gchar *hostname, gint port, ssl_callback func,
		gpointer user_data)
{
	HybirdSslConnection *conn;

	g_return_val_if_fail(hostname != NULL, NULL);
	g_return_val_if_fail(port != 0, NULL);
	g_return_val_if_fail(func != NULL, NULL);

	//SSLeay_add_all_algorithms();
	SSL_load_error_strings();
	SSL_library_init();

	conn = g_new0(HybirdSslConnection, 1);

	if (!(conn->ssl_ctx = SSL_CTX_new(SSLv23_client_method()))) {

		hybird_debug_error("ssl", "initialize SSL CTX: %s",
				ERR_reason_error_string(ERR_get_error()));
		hybird_ssl_connection_destory(conn);

		return NULL;
	}

	if (!(conn->ssl = SSL_new(conn->ssl_ctx))) {

		hybird_debug_error("ssl", "create SSl:%s",
				ERR_reason_error_string(ERR_get_error()));
		hybird_ssl_connection_destory(conn);

		return NULL;
	}

	conn->conn_cb = func;
	conn->conn_data = user_data;

	conn->conn = hybird_proxy_connect(hostname, port, ssl_connect_cb, conn);

	return conn;
}

gint
hybird_ssl_write(HybirdSslConnection *ssl, const gchar *buf, gint len)
{
	return SSL_write(ssl->ssl, buf, len);
}

gint
hybird_ssl_read(HybirdSslConnection *ssl, gchar *buf, gint len)
{
	return SSL_read(ssl->ssl, buf, len);
}

void 
hybird_connection_destroy(HybirdConnection *conn)
{
	if (conn) {
		g_free(conn->host);
		g_free(conn);
	}

}

void 
hybird_ssl_connection_destory(HybirdSslConnection *conn)
{
	if (conn) {
		SSL_free(conn->ssl);
		SSL_CTX_free(conn->ssl_ctx);
		hybird_connection_destroy(conn->conn);
		g_free(conn);
	}
}

gint 
hybird_get_http_code(const gchar *http_response)
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
		hybird_debug_error("http", "unknown http response");
		return 0;
	}

	temp = g_strndup(code_start, code_stop - code_start);
	code = atoi(temp);
	g_free(temp);

	return code;
}


gint 
hybird_get_http_length(const gchar *http_response)
{
	gchar *pos, *stop;
	gchar *temp;
	gint length;
	const gchar *cur = "Content-Length: ";

	g_return_val_if_fail(http_response != NULL, 0);

	if (!(pos = g_strrstr(http_response, cur))) {
		hybird_debug_error("http", "no Content-length in response header.");
		return 0;
	}

	pos += strlen(cur);

	for (stop = pos; *stop && *stop != '\r'; stop ++);

	temp = g_strndup(pos, stop - pos);
	length = atoi(temp);
	g_free(temp);

	return length;
}

#ifndef IM_CONNECT_H
#define IM_CONNECT_H

#include <glib.h>
#include <openssl/ssl.h>

typedef struct _IMConnection IMConnection;
typedef struct _IMSslConnection IMSslConnection;


typedef gboolean (*connect_callback)(gint sk, gpointer user_data);
typedef gboolean (*ssl_callback)(IMSslConnection *ssl_conn, gpointer user_data);

#include "eventloop.h"

struct _IMConnection {
	gint sk;
	gchar *host;
	gint port;
};

struct _IMSslConnection {
	gint sk;
	IMConnection *conn;

	ssl_callback conn_cb;
	gpointer conn_data;

	ssl_callback recv_cb;
	gpointer recv_data;

	SSL* ssl;
	SSL_CTX* ssl_ctx;
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
 * @return IMConncetion if success, which can be used to cancel
 *         the pending connection, destroy it if don't use or after use.
 *         NULL if there was an error.
 */
IMConnection* im_proxy_connect(const gchar *hostname, gint port,
		connect_callback func, gpointer user_data);

/**
 * Make a SSL connection to the specified host and port.
 *
 * @param hostname Hostname of the destination host.
 * @param port Port of the destination host.
 * @param func Callback function to call when connection 
 *        is established.
 * @param user_data User-specified data.
 *
 * @return IMSslConncetion if success, which can be used to cancel
 *         the pending connection, destroy it if don't use or after use.
 *         NULL if there was an error.
 */
IMSslConnection* im_ssl_connect(const gchar *hostname, gint port,
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
gint im_ssl_write(IMSslConnection *ssl, const gchar *buf, gint len);

/**
 * Read data from a SSL connection.
 *
 * @param ssl SSL connection context.
 * @param buf Data buffer to read data from.
 * @param len Number of bytes to read from buffer.
 *
 * @retutn The number of bytes read from the connection.
 */
gint im_ssl_read(IMSslConnection *ssl, gchar *buf, gint len);

/**
 * Destroy a connection, free the memory and close the socket.
 *
 * @param conn Connection to destroy.
 */
void im_connection_destroy(IMConnection *conn);

/**
 * Destroy a SSL connection, free the memory and close the socket.
 *
 * @param conn Connection to destroy.
 */
void im_ssl_connection_destory(IMSslConnection *conn);


/**
 * Parse the http response code from the response string.
 *
 * @param http_response The response string from the server.
 * 
 * @return The code.
 */
gint im_get_http_code(const gchar *http_response);

/**
 * Parse the body length of the http response string.
 *
 * @param http_response The response string from the server.
 * 
 * @return The length.
 */
gint im_get_http_length(const gchar *http_response);


#ifdef __cplusplus
}
#endif

#endif /* IM_CONNECT_H */

#include <glib.h>
#include "util.h"
#include "network.h"

static GHashTable *host_hash = NULL;

gint 
resolve_host(const gchar *hostname, gchar *ip)
{
	g_return_val_if_fail(hostname != NULL, HYBRID_ERROR);

	struct addrinfo *result;
	struct sockaddr_in *addr;
	gchar *hash_value;

	hybrid_debug_info("dns", "resolve host \'%s\'", hostname);

	if (host_hash && (hash_value = g_hash_table_lookup(host_hash, hostname))) {
		strcpy(ip, (gchar*)hash_value);	
		hybrid_debug_info("dns", "ip of \'%s\' is \'%s\'", hostname, ip);
		return HYBRID_OK;
	}

	if (getaddrinfo(hostname, NULL, NULL, &result) != 0) {
		hybrid_debug_error("resolve_host", "resolve host \'%s\' failed",
				hostname);
		return HYBRID_ERROR;
	}

	addr = (struct sockaddr_in*)result->ai_addr;

	if (!inet_ntop(AF_INET, (void*)&addr->sin_addr, ip, 16)) {
		hybrid_debug_error("dns", "reslove host \'%s\' failed when"
				" transforming binary ip address to doted ip address",
				hostname);
		return HYBRID_ERROR;
	}

	if (!host_hash) {
		host_hash = g_hash_table_new(g_str_hash, g_str_equal);
	}

	hash_value = g_strdup(ip);

	g_hash_table_insert(host_hash, (gchar*)hostname, hash_value);

	hybrid_debug_info("dns", "ip of \'%s\' is \'%s\'", hostname, ip);

	return HYBRID_OK;
}

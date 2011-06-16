#include "util.h"
#include "network.h"

gint 
resolve_host(const gchar *hostname, gchar *ip)
{
	g_return_val_if_fail(hostname != NULL, Hybird_ERROR);

	struct addrinfo *result;
	struct sockaddr_in *addr;

	hybird_debug_info("dns", "resolve host \'%s\'", hostname);

	if (getaddrinfo(hostname, NULL, NULL, &result) != 0) {
		hybird_debug_error("resolve_host", "resolve host \'%s\' failed", hostname);
		return Hybird_ERROR;
	}

	addr = (struct sockaddr_in*)result->ai_addr;

	if (!inet_ntop(AF_INET, (void*)&addr->sin_addr, ip, 16)) {
		hybird_debug_error("dns", "reslove host \'%s\' failed when"
				" transforming binary ip address to doted ip address", hostname);
		return Hybird_ERROR;
	}

	hybird_debug_info("dns", "ip of \'%s\' is \'%s\'", hostname, ip);

	return Hybird_OK;
}

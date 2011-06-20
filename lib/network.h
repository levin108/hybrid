#ifndef HYBRID_NETWORK_H
#define HYBRID_NETWORK_H

#define IPADDR_LEN 32

/**
 * resolve DNS hostname into dotted ip address
 * 
 * @param hostname The name to be resolved
 * @param ip It will be filled with the result ip address.
 *
 * @return HYBRID_OK if success, orelse HYBRID_ERROR
 */
gint resolve_host(const gchar *hostname, gchar *ip);

#endif

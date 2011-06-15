#ifndef IM_NETWORK_H
#define IM_NETWORK_H

#define IPADDR_LEN 32

/**
 * resolve DNS hostname into dotted ip address
 * 
 * @param hostname The name to be resolved
 * @param ip It will be filled with the result ip address.
 *
 * @return IM_OK if success, orelse IM_ERROR
 */
gint resolve_host(const gchar *hostname, gchar *ip);

#endif

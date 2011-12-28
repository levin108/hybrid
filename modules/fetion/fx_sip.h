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

#ifndef HYBRID_FX_SIP_H
#define HYBRID_FX_SIP_H

enum {
	FETION_SIP_OK = 200,
};

/**
 * Sip message type.
 */
enum {
	SIP_REGISTER = 1,
	SIP_SERVICE,
 	SIP_SUBSCRIPTION,
	SIP_NOTIFICATION,
	SIP_INVITATION,
	SIP_INFO,
	SIP_OPTION,
	SIP_MESSAGE,
	SIP_SIPC_4_0,
	SIP_ACKNOWLEDGE,
	SIP_UNKNOWN
};

/**
 * Notification type.
 */
enum {
	NOTIFICATION_TYPE_PRESENCE,
	NOTIFICATION_TYPE_CONTACT,
	NOTIFICATION_TYPE_CONVERSATION,
	NOTIFICATION_TYPE_REGISTRATION,
	NOTIFICATION_TYPE_SYNCUSERINFO,
	NOTIFICATION_TYPE_PGGROUP,
	NOTIFICATION_TYPE_UNKNOWN
};

/**
 * Notification event type.
 */
enum {
	NOTIFICATION_EVENT_PRESENCECHANGED,
	NOTIFICATION_EVENT_ADDBUDDYAPPLICATION,
	NOTIFICATION_EVENT_USERENTER,
	NOTIFICATION_EVENT_USERLEFT,
	NOTIFICATION_EVENT_DEREGISTRATION,
	NOTIFICATION_EVENT_SYNCUSERINFO,
	NOTIFICATION_EVENT_PGGETGROUPINFO,
	NOTIFICATION_EVENT_UNKNOWN
};

/**
 * Sip event type.
 */
enum {
	SIP_EVENT_PRESENCE = 0,
	SIP_EVENT_SETPRESENCE,
	SIP_EVENT_CONTACT,
	SIP_EVENT_CONVERSATION,
	SIP_EVENT_CATMESSAGE,
	SIP_EVENT_SENDCATMESSAGE,
	SIP_EVENT_STARTCHAT,
	SIP_EVENT_INVITEBUDDY,
	SIP_EVENT_GETCONTACTINFO,
	SIP_EVENT_CREATEBUDDYLIST,
	SIP_EVENT_DELETEBUDDYLIST,
	SIP_EVENT_SETCONTACTINFO,
	SIP_EVENT_SETUSERINFO,
	SIP_EVENT_SETBUDDYLISTINFO,
	SIP_EVENT_DELETEBUDDY,
	SIP_EVENT_ADDBUDDY,
	SIP_EVENT_KEEPALIVE,
	SIP_EVENT_DIRECTSMS,
	SIP_EVENT_SENDDIRECTCATSMS,
	SIP_EVENT_HANDLECONTACTREQUEST,
	SIP_EVENT_PGGETGROUPLIST,
	SIP_EVENT_PGGETGROUPINFO,
	SIP_EVENT_PGGETGROUPMEMBERS,
	SIP_EVENT_PGSENDCATSMS,
	SIP_EVENT_PGPRESENCE
};

typedef struct _fetion_sip fetion_sip;
typedef struct _sip_header sip_header;

#include "fx_account.h"

/**
 * sip header.
 */
struct _sip_header {
	gchar      *name;           /**< sip header namne */
	gchar      *value;          /**< sip header value */
	sip_header *next;           /**< next sip header */
};

/**
 * Sip context to handle sip message.
 */
struct _fetion_sip {
	gint        type;			/**< sip message type */
	gchar      *from;			/**< sender's fetion number,in sip header it's "F: "  */
	gint        callid;
	gint        sequence;		/**< sequence number , in sip it`s "Q: " */
	gint        threadCount;	/**< listening threads count using this sip */
	gchar      *sipuri;			/**< outer sipuri used when listening */
	sip_header *header;         /**< some othre header list	*/
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a fetion sip struct bound to a fetion account.
 *
 * @param ac The fetion account to bound to.
 *
 * @return The fetion sip created.
 */
fetion_sip *fetion_sip_create(fetion_account *ac);

/**
 * Set the type of the sip context.
 *
 * @param sip The sip context.
 * @param sip_type The sip type.
 */
void fetion_sip_set_type(fetion_sip *sip, gint sip_type);

/**
 * Get the type of the sip message by the head of the message.
 *
 * @param sipmsg The sip message.
 *
 * @return The enum value of the type.
 */
gint fetion_sip_get_msg_type(const gchar *sipmsg);

/**
 * Set the from attribute of the sip context.
 *
 * @param sip The sip context.
 * @param from The from attribute value.
 */
void fetion_sip_set_from(fetion_sip *sip, const gchar *from);

/**
 * Get the sip response code.
 *
 * @param sipmsg The sip message.
 *
 * @return The response code.
 */
gint fetion_sip_get_code(const gchar *sipmsg);
/**
 * Get the length of the sip body.
 *
 * @param sipmsg The sip message.
 *
 * @return The length of the sip body.
 */
gint fetion_sip_get_length(const gchar *sipmsg);

/**
 * Get the sid from the sipuri.
 * ie. sipuri: sip:916000000@fetion.com.cn;p=12207
 *     sid:    916000000
 *
 * @param sipuri The sipuri.
 *
 * @return The sid, needs to be freed after use.
 */
gchar *get_sid_from_sipuri(const gchar *sipuri);

/**
 * Add a header to the sip context's header chain. Then we can
 * use this chain to generate header string.
 *
 * @param sip The fetion sip struct.
 * @param header The sip header to add.
 */
void fetion_sip_add_header(fetion_sip *sip, sip_header *header);

/**
 * Generate sip message string using the sip context information
 * and the specified body string.
 *
 * @param sip The sip context.
 * @param body The sip body string.
 *
 * @return The sip message string, needs to be freed after use.
 */
gchar* fetion_sip_to_string(fetion_sip *sip, const gchar *body);

/**
 * Destroy a fetion sip struct.
 *
 * @param sip The fetion sip to destroy.
 */
void fetion_sip_destroy(fetion_sip *sip);

/**
 * Create a sip header.
 *
 * @param name The header name.
 * @param value The header value.
 *
 * @return The sip header created.
 */
sip_header *sip_header_create(const gchar *name, const gchar *value);

/**
 * Create a sip authentication header. like:
 *
 * Digest response="5E30DAE46EB253A7F0DF214BFA49E00A580DEA9FF79657643
 * E4237043659295428E5FB3791D38B987FFDF2E51",algorithm="SHA1-sess-v4"
 *
 * @param response The authencation header.
 */
sip_header *sip_authentication_header_create(const gchar *response);

/**
 * Create a sip acknowledgement header. This header is used when the
 * authentication needs to input confirm code.
 *
 * @param code The confirm code.
 * @param algorithm The algorithm string returned by the server.
 * @param type The acknowledgement type.
 * @param guid GUID also returned by the server.
 *
 * @return The acknowledgement header.
 */
sip_header *sip_ack_header_create(const gchar *code, const gchar *algorithm,
		const gchar *type, const gchar *guid);

/**
 * Create a credential sip header. This header is used when we start
 * a new chat session with an online friend. We need to start a new
 * socket channel to send data to.
 *
 * @param credential. The credential returned by the server.
 *
 * @param The credential header.
 */
sip_header *sip_credential_header_create(const gchar *credential);

/**
 * Create an event sip header. This header is used in most sip message.like:
 *
 * N: PresenceV4
 *
 * @param event_type The type of event.
 *
 * @return The event header created.
 *
 */
sip_header *sip_event_header_create(gint event_type);

/**
 * Get the header attribute value to the given name.
 *
 * @param header_string The headers string.
 * @param name The header attribute name.
 *
 * @return The header attribute value.
 */
gchar *sip_header_get_attr(const gchar *header_string, const gchar *name);

/**
 * Parse the authentication header to strip the ip address and port of the
 * new server, and the credential string.
 *
 * @param header_string The authentication attribute.
 * @param ip            The ip address of the server returned.
 * @param port          The port of the server returned.
 * @param credential    The credential string returned.
 *
 * HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint sip_header_get_auth(const gchar *header_string, gchar **ip, gint *port,
		gchar **credential);

/**
 * Destroy a sip header.
 *
 * @param header The sip header to destroy.
 */
void sip_header_destroy(sip_header *header);

/**
 * Parse the notify sip message, and get the notification type and event type.
 *
 * @param notify_type The notification type to fill.
 * @param event_type The event type to fill.
 */
void sip_parse_notify(const gchar *sipmsg, gint *notify_type, gint *event_type);

/**
 * Parse the synchronization sip message,
 * Note that we only concern with the UPDATE message.
 *
 * @param account The fetion account.
 * @param sipmsg  The synchronization message.
 *
 * @return The list of the buddies whose info should be synchronized.
 */
GSList *sip_parse_sync(fetion_account *account, const gchar *sipmsg);

/**
 * Parse the presence message.
 *
 * @param ac The fetion account context.
 * @param sipmsg The presence sip message.
 *
 * @return The list of presence-changedd buddies, with the lastest presence
 *         information filled.
 */
GSList *sip_parse_presence(fetion_account *ac, const gchar *sipmsg);

/**
 * Parse the add-buddy request message.
 *
 * @param sipmsg The add-buddy request message.
 * @param userid If not NULL, it will be filled with the userid of the buddy who
 *               sent the request.
 * @param sipuri If not NULL, it will be filled with the sipuri of the buddy who
 *               sent the request.
 * @param desc If not NULL, it will be filled with the description of the buddy who
 *               sent the request.
 *
 * @return HYBRID_OK or HYBRID_ERROR if @sipmsg is in bad format.
 */
gint sip_parse_appbuddy(const gchar *sipmsg, gchar **userid,
		gchar **sipuri, gchar **desc);

#ifdef __cplusplus
}
#endif

#endif

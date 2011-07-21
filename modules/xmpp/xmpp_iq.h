#ifndef HYBRID_XMPP_IQ_H
#define HYBRID_XMPP_IQ_H
#include <glib.h>
#include "xmpp_stream.h"

typedef struct _IqRequest IqRequest;
typedef struct _IqTransaction IqTransaction;

typedef gboolean (*trans_callback)(XmppStream *stream, xmlnode *node,
									gpointer user_data);


struct _IqRequest {
	gint type;
	XmppStream *stream;
	xmlnode *node; /**< root node of the xml context. */
	gint id;
	IqTransaction *trans;
};

struct _IqTransaction {
	gint iq_id;

	trans_callback callback;
	gpointer user_data;
};


enum {
	IQ_TYPE_GET,
	IQ_TYPE_SET,
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create an IQ request.
 *
 * @param stream The xmpp stream. 
 * @param type   Type of the request.
 *
 * @return The IQ request created.
 */
IqRequest *iq_request_create(XmppStream *stream, gint type);

/**
 * Set callback function called when the request was responded.
 *
 * @param iq        The IQ request.
 * @param callback  The callback function.
 * @param user_data User-specified data for the callback function.
 */
void iq_request_set_callback(IqRequest *iq, trans_callback callback,
		gpointer user_data);

/**
 * Send the xml request on the current stream.
 *
 * @param iq The iq request.
 *
 * HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint iq_request_send(IqRequest *iq);

/**
 * Destroy an iq request context.
 *
 * @param iq The iq request to destroy.
 */
void iq_request_destroy(IqRequest *iq);

/**
 * Create an iq transaction.
 *
 * @param iq_id Id of current transaction.
 *
 * @return The transaction created.
 */
IqTransaction *iq_transaction_create(gint iq_id);

/**
 * Set callback function for a transaction.
 *
 * @param trans     The iq transaction.
 * @param callback  The callback function.
 * @param user_data User-specified data for the callback function.
 */
void iq_transaction_set_callback(IqTransaction *trans, trans_callback callback,
						gpointer user_data);

/**
 * Add a transaction to the stream's pending list.
 *
 * @param stream The xmpp stream.
 * @param trans  The transaction pending to be processed.
 */
void iq_transaction_add(XmppStream *stream, IqTransaction *trans);

/**
 * Remove a transaction from the stream's pending list.
 *
 * @param stream The xmpp stream.
 * @param trans  The transaction processed.
 */
void iq_transaction_remove(XmppStream *stream, IqTransaction *trans);

/**
 * Destroy an transaction.
 *
 * @param trans The transaction to destroy.
 */
void iq_transaction_destroy(IqTransaction *trans);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
}
#endif

#endif

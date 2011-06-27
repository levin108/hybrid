#include "util.h"
#include "conv.h"
#include "xmlnode.h"

#include "fx_msg.h"
#include "fx_buddy.h"
#include "fx_trans.h"

gint
fetion_message_parse_sysmsg(const gchar *sipmsg, gchar **content, gchar **url)
{
	gchar *pos;
	xmlnode *root;
	xmlnode *node;

	if (!(pos = strstr(sipmsg, "\r\n\r\n"))) {
		goto sysmsg_error;
	}

	if (!(root = xmlnode_root(pos, strlen(pos)))) {
		goto sysmsg_error;
	}

	if (!(node = xmlnode_find(root, "content"))) {
		xmlnode_free(root);
		goto sysmsg_error;
	}

	*content = xmlnode_content(node);

	if ((node = xmlnode_find(root, "url"))) {
		*url = xmlnode_content(node);

	} else {
		*url = NULL;
	}

	xmlnode_free(root);

	return HYBRID_OK;

sysmsg_error:
	hybrid_debug_error("fetion", "invalid system message");
	return HYBRID_ERROR;
}


static gint
sms_response_cb(fetion_account *account, const gchar *sipmsg, 
						fetion_transaction *trans)
{
	gint code;

	g_source_remove(trans->timer);

	code = fetion_sip_get_code(sipmsg);

	if (code != 200 && code != 280) {
		return HYBRID_ERROR;
	}

	/* TODO add error message to textview */


	return HYBRID_OK;
}

static void
sms_timeout_cb(fetion_transaction *trans)
{
	fetion_account *account;

	account = trans->data;

	/* TODO add error message to textview */
	printf("send message time out\n");

	g_source_remove(trans->timer);
	transaction_remove(account, trans);
}

gint
fetion_message_send(fetion_account *account, const gchar *userid,
						const gchar *text)
{
	fetion_sip *sip;
	sip_header *toheader;
	sip_header *cheader;
	sip_header *kheader;
	sip_header *nheader;
	gchar *sip_text;
	fetion_buddy *buddy;

	g_return_val_if_fail(account != NULL, HYBRID_ERROR);
	g_return_val_if_fail(userid != NULL && *userid != '\0', HYBRID_ERROR);
	g_return_val_if_fail(text != NULL, HYBRID_ERROR);

	sip = account->sip;

	if (!(buddy = fetion_buddy_find_by_userid(account, userid))) {
		hybrid_debug_error("fetion", "FATAL, can't find specified buddy");

		return HYBRID_ERROR;
	}

	fetion_transaction *trans = transaction_create();

	transaction_set_userid(trans, userid);
	//transaction_set_msg(trans, text);

	fetion_sip_set_type(sip, SIP_MESSAGE);

	nheader  = sip_event_header_create(SIP_EVENT_CATMESSAGE);
	toheader = sip_header_create("T", buddy->sipuri);
	cheader  = sip_header_create("C", "text/plain");
	kheader  = sip_header_create("K", "SaveHistory");
	fetion_sip_add_header(sip, toheader);
	fetion_sip_add_header(sip, cheader);
	fetion_sip_add_header(sip, kheader);
	fetion_sip_add_header(sip, nheader);

	transaction_set_callid(trans, sip->callid);
	transaction_set_callback(trans, sms_response_cb);

	transaction_set_data(trans, account);
	transaction_set_timeout(trans, (GSourceFunc)sms_timeout_cb, trans);
	transaction_add(account, trans);

	sip_text = fetion_sip_to_string(sip, text);

	if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {
		g_free(sip_text);

		return HYBRID_ERROR;
	}

	g_free(sip_text);

	return HYBRID_OK;
}

gint
fetion_process_message(fetion_account *account, const gchar *sipmsg)
{
	gchar *from;
	gchar *sid;
	gchar *callid;
	gchar *sequence;
	gchar *sendtime;
	gchar *text;
	gchar *sip_text;
	fetion_buddy *buddy;

	g_return_val_if_fail(account != NULL, HYBRID_ERROR);
	g_return_val_if_fail(sipmsg != NULL, HYBRID_ERROR);

	if (!(text = strstr(sipmsg, "\r\n\r\n"))) {
		hybrid_debug_error("fetion", "invalid message received\n");

		return HYBRID_ERROR;
	}

	text += 4;

	from     = sip_header_get_attr(sipmsg, "F");
	callid   = sip_header_get_attr(sipmsg, "I");
	sendtime = sip_header_get_attr(sipmsg, "D");
	sequence = sip_header_get_attr(sipmsg, "Q");

	sip_text = g_strdup_printf(
					"SIP-C/4.0 200 OK\r\n"
					"I: %s\r\n"
					"Q: %s\r\n"
					"F: %s\r\n\r\n", callid, sequence, from);
	g_free(callid);
	g_free(sendtime);
	g_free(sequence);

	if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {
		g_free(sip_text);
		g_free(from);

		return HYBRID_ERROR;
	}

	g_free(sip_text);

	sid = get_sid_from_sipuri(from);
	g_free(from);

	if (!(buddy = fetion_buddy_find_by_sid(account, sid))) {

		hybrid_debug_error("fetion", "invalid message received\n");
		g_free(sid);

		return HYBRID_ERROR;
	}

	hybrid_conv_got_message(account->account, buddy->userid, text);

	g_free(sid);


	return HYBRID_OK;
}

/**
 * Callback function to handle the new_chat response message, if success
 * we would get the following message:
 *
 * SIP-C/4.0 200 OK
 * I: 4
 * Q: 2 S
 * A: CS address="221.176.31.128:8080;221.176.31.128:443",credential="439333922.916967705"
 *
 * Now we should start a new socket connect to 221.176.31.128:8080 with port 443 as
 * a back port if 8080 failed to connect.
 */
gint
new_chat_cb(fetion_account *account, const gchar *sipmsg,
				fetion_transaction *trans)
{
	printf("%s\n", sipmsg);

	return HYBRID_OK;
}

gint
fetion_message_new_chat(fetion_account *account, const gchar *userid,
								const gchar *text)
{
	fetion_sip *sip;
	sip_header *eheader;
	fetion_transaction *trans;
	gchar *sip_text;

	g_return_val_if_fail(account != NULL, HYBRID_ERROR);
	g_return_val_if_fail(userid != NULL, HYBRID_ERROR);

	sip = account->sip;

	/*start chat*/
	fetion_sip_set_type(sip, SIP_SERVICE);
	eheader = sip_event_header_create(SIP_EVENT_STARTCHAT);
	fetion_sip_add_header(sip, eheader);

	trans = transaction_create();
	transaction_set_callid(trans, sip->callid);
	transaction_set_userid(trans, userid);
	transaction_set_msg(trans, text);
	transaction_set_callback(trans, new_chat_cb);
	transaction_add(account, trans);

	sip_text = fetion_sip_to_string(sip, NULL);

	hybrid_debug_info("fetion", "new chat,send:\n%s", sip_text);

	if (send(account->sk, sip_text, strlen(sip_text), 0) == -1) {

		hybrid_debug_error("fetion", "new chat failed");

		return HYBRID_ERROR;
	}
	
	g_free(sip_text); 

	return HYBRID_OK;
}

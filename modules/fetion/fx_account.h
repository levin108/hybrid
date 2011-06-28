#ifndef HYBRID_FX_ACCOUNT_H
#define HYBRID_FX_ACCOUNT_H
#include <glib.h>
#include "account.h"

typedef struct _fetion_account fetion_account;
typedef struct _Verification Verification;

#include "fx_sip.h"

struct _Verification {
	gchar *algorithm;
	gchar *type;
	gchar *text;
	gchar *tips;
	gchar *code;
	gchar *guid;
};


struct _fetion_account {
	HybridAccount *account;
	gint sk; /**< The socket descriptor. */
	guint source; /**< The ID of the event source. */

	gchar *buffer; /**< buffer for received data */

	gchar *sid;      /**< Fetion number. */
	gchar *userid;   /**< user id */
	gchar *mobileno; /**< Mobile phone number */
	gchar *password; /**< Raw password not hashed */
	gchar *sipuri;   /**< sipuri like 'sip:100@fetion.com.cn' */
	gchar *nickname;
	gchar *mood_phrase;	
	gchar *portrait_crc;
	gchar *country;
	gchar *province;
	gchar *city;
	gint   gender;

	GSList *groups; /**< The group list */
	GSList *buddies; /**< The buddy list */

	gchar *sms_online_status;
	gchar *public_ip;					
	gchar *last_login_ip;				
	gchar *last_login_time;				

	gchar *portrait_host_name; /**< host name of the portrait server */
	gchar *portrait_host_path; /**< path on the portrait server */
	gchar *sipc_proxy_ip; /**< ip address of the sipc host */
	gint  sipc_proxy_port; /**< port of the sipc host */

	/* config versions */
	gchar *cfg_server_version;
	gchar *cfg_param_version;
	gchar *cfg_hint_version;
	gchar *cfg_client_version;

	gint sms_day_limit;
	gint sms_day_count;
	gint sms_month_limit;
	gint sms_month_count;

	gint pgGroupCallId;					
	gint groupInfoCallId;			

	gint state;					
	gint login_status; 	

	gchar *carrier;
	gint carrier_status;
	gint bound_to_mobile;

	/* versions */
	gchar *personal_version;
	gchar *contact_list_version;
	gchar *custom_config_version;

	/* cookie string read from reply message after ssi login */
	gchar *ssic;

	/* custom config string used to set personal information */
	gchar *custom_config; 

	/* a struct used to generate picture code */
	Verification *verification;		 

	fetion_sip *sip;

	GSList *trans_list;

	GSList *trans_wait_list;
	/*
	 * only used in the conversation with an online buddy, if channel
	 * is not ready, transaction should be stored in the trans_wait_list
	 * until the channel is ready, so it's always TRUE for the main channel.
	 */
	gboolean channel_ready;
	
	/* only used in the conversation with an online buddy, the buddy's userid. */
	gchar *who;

};


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a fetion account structure, using the specified number and password.
 *
 * @param account The HybridAccount struct.
 * @param no Mobile number or fetion number.
 * @param password The password of this account.
 *
 * @return NULL if there was an error. fetion_account if success.
 */
fetion_account *fetion_account_create(HybridAccount *account, const gchar *no,
		const gchar *password);

/**
 * Clone an account with the basic attribute, when we start a new socket channel,
 * we will use the new account context to make sip request, it should be freed
 * with fetion_account_destroy() when no longer needed.
 *
 * @param account The fetion account.
 *
 * @return The fetion account cloned.
 */
fetion_account *fetion_account_clone(fetion_account *account);

/**
 * Set the attribute value of the fetion account.
 */
#define fetion_account_set_sipuri(ac,v)	    do { g_free((ac)->sipuri);   (ac)->sipuri = g_strdup(v); }   while(0)
#define fetion_account_set_userid(ac,v)	    do { g_free((ac)->userid);   (ac)->userid = g_strdup(v); }   while(0)
#define fetion_account_set_mobileno(ac,v)	do { g_free((ac)->mobileno); (ac)->mobileno = g_strdup(v); } while(0)
#define fetion_account_set_ssic(ac,v)	    do { g_free((ac)->ssic);     (ac)->ssic = g_strdup(v); }     while(0)
#define fetion_account_set_who(ac,v)	    do { g_free((ac)->who);      (ac)->who = g_strdup(v); }      while(0)
//#define fetion_account_set_sipuri(ac,v)	((ac)->sipuri = g_strdup(v))

/**
 * Destroy the fetion account.
 *
 * @param ac The fetion account to destroy.
 */
void fetion_account_destroy(fetion_account *ac);

/**
 * Update the presence state of the account, the message is:
 *
 * S fetion.com.cn SIP-C/4.0
 * F: 547264589
 * I: 7
 * Q: 2 S
 * N: SetPresenceV4
 * L: 56
 *
 * <args><presence><basic value="400"/></presence></args>
 */
gint fetion_account_update_state(fetion_account *ac, gint state);

/**
 * Set a keep alive message to the server.
 *
 * @param ac The fetion account.
 *
 * @return HYBRID_OK if success, HYBRID_ERROR if there was an error.
 */
gint fetion_account_keep_alive(fetion_account *ac);

/**
 * Fetch the portrait of the account from the server.
 *
 * @param ac The account.
 *
 * @return HYBRID_OK if success, HYBRID_ERROR if there was an error.
 */
gint fetion_account_update_portrait(fetion_account *ac);


#ifdef __cplusplus
}
#endif

#endif /* HYBRID_FX_ACCOUNT_H */

#ifndef HYBIRD_FX_ACCOUNT_H
#define HYBIRD_FX_ACCOUNT_H
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
	HybirdAccount *account;

	/* buffer for received data */
	gchar *buffer;

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

	/* ip address for sipc host */
	gchar *sipc_proxy_ip;

	/* port for sipc host */
	gint  sipc_proxy_port;

	/* host name for portrait server */
	gchar *portrait_host_name;		

	/* path on the portrait server */
	gchar *portrait_host_path;	

	gchar *cfg_server_version;
	gchar *cfg_param_version;
	gchar *cfg_hint_version;

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
};


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a fetion account structure, using the specified number and password.
 *
 * @param account The HybirdAccount struct.
 * @param no Mobile number or fetion number.
 * @param password The password of this account.
 *
 * @return NULL if there was an error. fetion_account if success.
 */
fetion_account *fetion_account_create(HybirdAccount *account, const gchar *no,
		const gchar *password);

/**
 * Set the attribute value of the fetion account.
 */
#define fetion_account_set_sipuri(ac,v)	    ((ac)->sipuri = g_strdup(v))
#define fetion_account_set_userid(ac,v)	    ((ac)->userid = g_strdup(v))
#define fetion_account_set_mobileno(ac,v)	((ac)->mobileno = g_strdup(v))
#define fetion_account_set_ssic(ac,v)	    ((ac)->ssic = g_strdup(v))
//#define fetion_account_set_sipuri(ac,v)	((ac)->sipuri = g_strdup(v))

/**
 * Destroy the fetion account.
 *
 * @param ac The fetion account to destroy.
 */
void fetion_account_destroy(fetion_account *ac);


#ifdef __cplusplus
}
#endif

#endif /* HYBIRD_FX_ACCOUNT_H */

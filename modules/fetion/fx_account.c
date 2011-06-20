#include <glib.h>
#include "util.h"
#include "fx_account.h"

fetion_account*
fetion_account_create(HybridAccount *account, const gchar *no, const gchar *password)
{
	fetion_account *ac;

	g_return_val_if_fail(no != NULL, NULL);
	g_return_val_if_fail(password != NULL, NULL);

	ac = g_new0(fetion_account, 1);

	ac->account = account;

	if (strlen(no) == 11) { /* mobile no */
		ac->mobileno = g_strdup(no);

	} else {
		ac->sid = g_strdup(no);
	}

	ac->sip = fetion_sip_create(ac);
	ac->password = g_strdup(password);

	return ac;
}

void 
fetion_account_destroy(fetion_account *ac)
{
	if (ac) {
		g_free(ac->sid);
		g_free(ac->mobileno);
		g_free(ac->password);
		g_free(ac->sipuri);
		g_free(ac->userid);
		g_free(ac->ssic);
		g_free(ac->nickname);
		g_free(ac->mood_phrase);
		g_free(ac->portrait_crc);
		g_free(ac->country);
		g_free(ac->province);
		g_free(ac->city);
		g_free(ac->sms_online_status);
		g_free(ac->last_login_ip);
		g_free(ac->last_login_time);
		g_free(ac->public_ip);
		g_free(ac->sipc_proxy_ip);
		g_free(ac->portrait_host_path);
		g_free(ac->portrait_host_name);
		g_free(ac->cfg_server_version);
		g_free(ac->cfg_param_version);
		g_free(ac->cfg_hint_version);
		g_free(ac->personal_version);
		g_free(ac->contact_list_version);
		g_free(ac->custom_config_version);
		g_free(ac->custom_config);
		g_free(ac->ssic);

		fetion_sip_destroy(ac->sip);
		g_free(ac);
	}
}

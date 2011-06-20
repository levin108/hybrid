#ifndef HYBRID_FETION_H
#define HYBRID_FETION_H
#include <glib.h>

#define PROTO_VERSION "4.0.2510"

#define NAV_SERVER "nav.fetion.com.cn"
#define SSI_SERVER "uid.fetion.com.cn"

#include "fx_account.h"


enum {
	P_ONLINE = 		 400, 
	P_RIGHTBACK = 	 300,
	P_AWAY = 		 100,
	P_BUSY = 		 600,
	P_OUTFORLUNCH =  500,
	P_ONTHEPHONE = 	 150,
	P_MEETING = 	 850,
	P_DONOTDISTURB = 800,
	P_INVISIBLE =	 0,
	P_OFFLINE =      -1
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Process the pushed sip message.
 *
 * @param ac The fetion account context.
 * @param sipmsg The sip message to process.
 */
void process_pushed(fetion_account *ac, const gchar *sipmsg);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_FETION_H */

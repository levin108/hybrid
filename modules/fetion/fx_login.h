#ifndef HYBRID_FX_LOGIN_H
#define HYBRID_FX_LOGIN_H

#include "connect.h"

#ifdef __cplusplus
extern "C" {
#endif

gboolean ssi_auth_action(HybridSslConnection *isc, gpointer user_data);

gboolean hybrid_push_cb(gint sk, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_FX_LOGIN_H */

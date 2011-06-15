#ifndef IM_FX_LOGIN_H
#define IM_FX_LOGIN_H

#include "connect.h"

#ifdef __cplusplus
extern "C" {
#endif

gboolean ssi_auth_action(IMSslConnection *isc, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif /* IM_FX_LOGIN_H */

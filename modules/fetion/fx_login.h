#ifndef HYBIRD_FX_LOGIN_H
#define HYBIRD_FX_LOGIN_H

#include "connect.h"

#ifdef __cplusplus
extern "C" {
#endif

gboolean ssi_auth_action(HybirdSslConnection *isc, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif /* HYBIRD_FX_LOGIN_H */

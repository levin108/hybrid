#ifndef Hybird_FX_LOGIN_H
#define Hybird_FX_LOGIN_H

#include "connect.h"

#ifdef __cplusplus
extern "C" {
#endif

gboolean ssi_auth_action(HybirdSslConnection *isc, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif /* Hybird_FX_LOGIN_H */

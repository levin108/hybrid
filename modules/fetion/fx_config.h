#ifndef HYBRID_FX_CONFIG_H
#define HYBRID_FX_CONFIG_H

#include "fx_account.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the local directory where to store configuration files
 * of the given account.
 *
 * @param account The fetion account.
 *
 * @return The absolute path of fetion config directory, should be
 *         freed with g_free() when no longer needed.
 */
gchar *fetion_get_config_dir(fetion_account *account);

/**
 * Save the contact-list node in the sipc response message.
 *
 * @param account The fetiona account.
 * @param node    The contact-list xml node.
 */
void fetion_config_save_buddies(fetion_account *account, xmlnode *node);

#ifdef __cplusplus
}
#endif

#endif /* HYBRId_FX_CONFIG_H */

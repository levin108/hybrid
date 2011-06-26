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
 * Save some version information of the fetion account, such as
 * personal version,contact-list version,custom-config version, etc.
 *
 * @param account The fetion account.
 */
void fetion_config_save_account(fetion_account *account);

/**
 * Load the version information of the fetion account from the disk.
 *
 * @param account The fetion account.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint fetion_config_load_account(fetion_account *account);

/**
 * Save the personal node in the sipc response message.
 *
 * @param account The fetion account.
 * @param node    The personal xml node.
 */
void fetion_config_save_personal(fetion_account *account, xmlnode *node);

/**
 * Load the personal node from the disk.
 *
 * @param account The fetion account.
 *
 * @return The root node of personal.xml, should be freed with 
 *         xmlnode_free() when no longer needed.
 */
xmlnode *fetion_config_load_personal(fetion_account *account);

/**
 * Save the contact-list node in the sipc response message.
 *
 * @param account The fetion account.
 * @param node    The contact-list xml node.
 */
void fetion_config_save_buddies(fetion_account *account, xmlnode *node);

/**
 * Load the contact-list node from the disk.
 *
 * @param account The fetion account.
 *
 * @return The root node of buddies.xml, should be freed with 
 *         xmlnode_free() when no longer needed.
 */
xmlnode *fetion_config_load_buddies(fetion_account *account);
#ifdef __cplusplus
}
#endif

#endif /* HYBRId_FX_CONFIG_H */

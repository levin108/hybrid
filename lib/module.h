/***************************************************************************
 *   Copyright (C) 2011 by levin                                           *
 *   levin108@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 ***************************************************************************/

#ifndef HYBRID_MODULE_H
#define HYBRID_MODULE_H

#include <glib.h>
#include <gmodule.h>

typedef struct _HybridModule	 HybridModule;
typedef struct _HybridModuleInfo HybridModuleInfo;
typedef struct _HybridIMOps		 HybridIMOps;
typedef struct _HybridEmailOps	 HybridEmailOps;
typedef enum _HybridInputState	 HybridInputState;
typedef enum _HybridModuleType	 HybridModuleType;

enum _HybridInputState {
	INPUT_STATE_TYPING,
	INPUT_STATE_PAUSED,
	INPUT_STATE_ACTIVE,
};

enum _HybridModuleType {
	MODULE_TYPE_IM,
	MODULE_TYPE_EMAIL,
};


#include "account.h"
#include "tooltip.h"
#include "blist.h"


struct _HybridIMOps {

	gboolean (*login)(HybridAccount *account);
	/*
	 * Get the detail information of a buddy.
	 */
	void     (*get_info)(HybridAccount *account, HybridBuddy *buddy);
	/*
	 * Modify nickname of this account.
	 */
	gboolean (*modify_name)(HybridAccount *account, const gchar *new_name);
	/*
	 * Modify status text of this account. 
	 */
	gboolean (*modify_status)(HybridAccount *account, const gchar *new_status);
	/*
	 * Modify photo of this account.
	 */
	gboolean (*modify_photo)(HybridAccount *account, const gchar*);
	/*
	 * Change state of this account.
	 */
	gboolean (*change_state)(HybridAccount *account, gint new_state);
	/*
	 * Callback function to process keep alive event.
	 */
	gboolean (*keep_alive)(HybridAccount *account);

	/*
	 * Show account tooltip when mouse over the account panel.
	 */
	gboolean (*account_tooltip)(HybridAccount	  *account,
								HybridTooltipData *data);

	/*
	 * Show buddy tooltip when mouse over a certain buddy.
	 */
	gboolean (*buddy_tooltip)(HybridAccount		*account,
							  HybridBuddy		*buddy,
							  HybridTooltipData	*data);

	/*
	 * Move a buddy to a new group.
	 */
	gboolean (*buddy_move)(HybridAccount *account,
						   HybridBuddy	 *buddy,
						   HybridGroup	 *group);
  
	/*
	 * Remove a buddy from the buddy list.
	 */
	gboolean (*buddy_remove)(HybridAccount *account,
							 HybridBuddy   *buddy);

	/*
	 * Modify the local-name of a buddy.
	 */
	gboolean (*buddy_rename)(HybridAccount *account,
							 HybridBuddy   *buddy,
							 const gchar   *new_name);

	/*
	 * Add a new buddy.
	 */
	gboolean (*buddy_add)(HybridAccount	*account,
						  HybridGroup	*group,
						  const gchar	*name,
						  const gchar	*alias,
						  const gchar	*tips);
	
	gboolean (*buddy_req)(HybridAccount	 *account,
						  HybridGroup	 *group,
						  const gchar	 *id,
						  const gchar	 *alias,
						  gboolean		  accept,
						  const gpointer  user_data);
	
	/*
	 * Modify the name of the group.
	 */
	gboolean (*group_rename)(HybridAccount *account,
							 HybridGroup *	group,
							 const gchar   *new_name);
	/*
	 * Remove a group.
	 */
	gboolean (*group_remove)(HybridAccount *account,
							 HybridGroup   *group);

    /*
	 * Group-add hook function, note that it won't add the new group to the
	 * buddy list automaticly, you should add it manually in the hook function.
	 */
	void     (*group_add)(HybridAccount	*account,
						  const gchar	*group_name);

    /*
	 * To get the word limit in a chat window, return zero for no limit.
	 */
	gint     (*chat_word_limit)(HybridAccount *account);

	/*
	 * Start chat with a buddy.
	 */
	gboolean (*chat_start)(HybridAccount *account,
						   HybridBuddy	 *buddy);

	/*
	 * Send input state to a certain buddy.
	 */
	void     (*chat_send_typing)(HybridAccount	  *account,
								 HybridBuddy	  *buddy,
								 HybridInputState  state);
	
	/*
	 * Send a message to a certain buddy.
	 */
	void     (*chat_send)(HybridAccount	*account,
						  HybridBuddy	*buddy,
						  const gchar	*text);
	
	/*
	 * Close this account.
	 */
	void     (*close)(HybridAccount *account);
	
};

struct _HybridEmailOps {
	gboolean (*login)(HybridAccount *account);
    gboolean (*close)(HybridAccount *account);
};

struct _HybridModuleInfo {
	gchar *name;
	gchar *author;
	gchar *summary;
	gchar *description;
	gchar *homepage;
	gchar *major_version;
	gchar *minor_version;
	gchar *icon; /**< The name of the protocol icon, ie,
				   "msn" for "msn.png", the protocol icon must
				   be in "png" format, and stored in 
				   HYBRID_INSTALL_DIR/share/hybrid/protocols
					   */
  
	HybridModuleType  module_type;
	HybridIMOps		 *im_ops;
	HybridEmailOps	 *email_ops;
  
	
	/*
	 * To tell the plugin which menus this account has.
	 */
	GSList* (*actions)(HybridAccount *account);
	
	/*
	 * To tell the plugin which login options this account has;
	 */
	GSList* (*options)();
};

struct _HybridModule {
	/* full path of the library file */
	gchar            *path;
	gboolean          loaded;
	GSList           *option_list;
	/* plugin info */
	HybridModuleInfo *info;
  
};

#ifdef __cplusplus
extern "C" {
#endif

#define HYBRID_MODULE_INIT(func, moduleinfo) \
	G_MODULE_EXPORT gboolean proto_module_init(HybridModule *module); \
	G_MODULE_EXPORT gboolean proto_module_init(HybridModule *module) { \
		module->info = (moduleinfo); \
		func(module); \
		hybrid_module_register(module); \
		return TRUE; \
	}


/**
 * Initialize the module function, load all the 
 * protocol modules in MODULE_DIR directory.
 *
 * @return HYBRID_OK if success, orelse HYBRID_ERROR.
 */
gint hybrid_module_init();

/**
 * Create a new protocol plugin.
 *
 * @param path Full path to the module library file *.so,*.dll.
 *
 * @return Hybrid Module created, need to be destroyed after use.
 */
HybridModule *hybrid_module_create(const gchar *path);

/**
 * Destroy a module, free the memory allocated.
 * 
 * @param module Module to destroy.
 */
void hybrid_module_destroy(HybridModule *module);

/**
 * Load the module from the module file, run the exported sympol 
 * function hybrid_plugin_init() inside the module.
 *
 * @param module Module to load.
 *
 * @return HYBRID_OK if success, orelse HYBRID_ERROR.
 */
gint hybrid_module_load(HybridModule *module);

/**
 * Register the plugin to the plugin chain.
 *
 * @param module Module to register.
 */
void hybrid_module_register(HybridModule *module);

/**
 * Deregister the plugin from the plugin chain.
 *
 * @param module Module to deregister.
 */
void hybrid_module_deregister(HybridModule *module);

/**
 * Find a protocol module by name.
 *
 * @param name Name of the module.
 *
 * @return Hybrid Module if found, orelse NULL.
 */
HybridModule *hybrid_module_find(const gchar *name);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_MODULE_H */

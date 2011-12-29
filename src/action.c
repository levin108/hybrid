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

#include "action.h"

HybridAction*
hybrid_action_create(HybridAccount *account, const gchar *text,
                     ActionCallback callback)
{
    HybridAction *action;

    g_return_val_if_fail(account != NULL, NULL);
    g_return_val_if_fail(text != NULL, NULL);

    action = g_new0(HybridAction, 1);
    action->text     = g_strdup(text);
    action->account  = account;
    action->callback = callback;

    return action;
}

void
hybrid_action_destroy(HybridAction *action)
{
    if (action) {
        g_free(action->text);
        g_free(action);
    }
}

HybridAccount*
hybrid_action_get_account(HybridAction *action)
{
    g_return_val_if_fail(action != NULL, NULL);

    return action->account;
}

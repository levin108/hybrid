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

#ifndef HYBRID_FETION_H
#define HYBRID_FETION_H
#include <glib.h>

#define PROTO_VERSION "4.0.2510"

#define NAV_SERVER "nav.fetion.com.cn"
#define SSI_SERVER "uid.fetion.com.cn"

#include "fx_account.h"


enum {
	P_ONLINE       = 400,
	P_RIGHTBACK    = 300,
	P_AWAY         = 100,
	P_BUSY         = 600,
	P_OUTFORLUNCH  = 500,
	P_ONTHEPHONE   = 150,
	P_MEETING      = 850,
	P_DONOTDISTURB = 800,
	P_INVISIBLE    = 0,
	P_OFFLINE      = -1
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

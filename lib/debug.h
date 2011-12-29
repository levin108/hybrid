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

#include <glib.h>

#ifndef HYBRID_DEBUG_H
#define HYBRID_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/* format cannot be a variable now. (this should always be the case.) */

#define hybrid_debug_print(f, pre, post, relm, format, args...)   \
    fprintf(f, pre" <%s> "post" "format"\n", relm, ##args)

#define hybrid_debug_info(relm, format, args...)                        \
    hybrid_debug_print(stdout, "\e[35m\e[1mINFO", "\e[0m", relm, format, ##args)
#define hybrid_debug_error(relm, format, args...)                       \
    hybrid_debug_print(stderr, "\e[31m\e[1mERROR ***", "***\e[0m",      \
                       relm, format, ##args)
#define hybrid_debug_warning(relm, format, args...)                \
    hybrid_debug_print(stdout, "\e[35m\e[1mWARNING", "\e[0m", relm, format, ##args)

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_DEBUG_H */

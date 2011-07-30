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

#include "debug.h"
#include "util.h"

void 
hybrid_debug_info(const gchar *relm, const gchar *format, ...)
{
	gchar fmt[4096];
	va_list ap;

	snprintf(fmt, sizeof(fmt) - 1, "INFO <%s> %s\n", relm, format);
	va_start(ap, format);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
}

void 
hybrid_debug_error(const gchar *relm, const gchar *format, ...)
{
	gchar fmt[4096];
	va_list ap;

	snprintf(fmt, sizeof(fmt) - 1, "ERROR *** <%s> *** %s\n", relm, format);
	va_start(ap, format);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
}

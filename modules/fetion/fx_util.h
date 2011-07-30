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

#ifndef HYBRID_FX_UTIL_H
#define HYBRID_FX_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the full province name by the province code.
 *
 * @param province The province code.
 *
 * @return The full province name, should be freed with g_free()
 *         when no longer needed.
 */
gchar *get_province_name(const gchar *province);

/**
 * Get the full city name by the city code.
 *
 * @param province The code of the province to which the city belongs.
 * @param city     The city code.
 *
 * @return The full city name, should be freed with g_free()
 *         when no longer needed.
 */
gchar *get_city_name(const gchar *province, const gchar *city);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_FX_UTIL_H */

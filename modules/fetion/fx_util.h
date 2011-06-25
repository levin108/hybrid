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

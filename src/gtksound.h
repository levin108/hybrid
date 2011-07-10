#ifndef HYBRID_GTKSOUND_H
#define HYBRID_GTKSOUND_H
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Initialize the sound context.
 */
void hybrid_sound_init(gint argc, gchar **argv);

/**
 * Play an wav file.
 *
 * @param filename The absolute path of the wav file.
 */
void hybrid_sound_play_file(const gchar *filename);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_GTKSOUND_H */

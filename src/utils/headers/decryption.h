#ifndef DECRYPTION_H
#define DECRYPTION_H

#include <file_manager.h>

#define MOD 257

/**
 * Recupera la imagen secreta a partir de k portadoras en un esquema (k, n).
 *
 * @param shades_count  Valor de k (mínimo de sombras necesarias).
 * @param output_file   Ruta donde escribir la imagen secreta recuperada.
 * @param images_count  Número de portadoras proporcionadas.
 * @param cover_files   Array con las rutas de las portadoras.
 * @return 0 en éxito, -1 en error.
 */
int shamir_recover(int shades_count, const char* output_file, int images_count, const char** cover_files);

#endif
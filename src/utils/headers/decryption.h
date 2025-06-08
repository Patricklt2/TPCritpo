#ifndef DECRYPTION_H
#define DECRYPTION_H

#include <file_manager.h>


#define MOD 257

void unflatten_matrix(Mod257Pixel* flat, int height, int width, Mod257Pixel** matrix);
void recover_polynomial(int* x_coords, Mod257Pixel* shares, int k, Mod257Pixel* coefficients);
void recover_from_files_v2(int k, int n, const char** cover_files, char* output_file);
void unprocess_image_partial_v2(BMP257Image *image, Mod257Pixel **processed_pixels, int k, int n, const int *shadow_indices, int num_shadows, uint16_t seed);   

// ----- Deprecated war zone ---------

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
void process_image(BMP257Image *image, Mod257Pixel **processed_pixels, int k, int n, uint16_t seed); 
void unprocess_image(BMP257Image *image, Mod257Pixel **processed_pixels, int k, int n, uint16_t seed);
void unprocess_image_partial(BMP257Image *image, Mod257Pixel **processed_pixels, int k, int n, const int *indices, uint16_t seed); 
#endif
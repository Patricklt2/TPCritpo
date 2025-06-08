#ifndef DECRYPTION_H
#define DECRYPTION_H

#include <file_manager.h>


#define MOD 257

void unflatten_matrix(Mod257Pixel* flat, int height, int width, Mod257Pixel** matrix);
void recover_polynomial(int* x_coords, Mod257Pixel* shares, int k, Mod257Pixel* coefficients);
void recover_from_files_v2(int k, int n, const char** cover_files, char* output_file);
void unprocess_image_partial_v2(BMP257Image *image, Mod257Pixel **processed_pixels, int k, int n, const int *shadow_indices, int num_shadows, uint16_t seed);   

#endif
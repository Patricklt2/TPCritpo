#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <file_manager.h>
#include <time.h>
#include <Permutation_Table.h>

#define MAX_K 10
/**
 * Distributes a secret image into multiple shares using Shamir's Secret Sharing
 * @param shades_count Number of shares to create (k)
 * @param file_name Name of the secret image file to distribute
 * @param images_count Total number of images to create (n)
 * @param cover_files Array of file names for the output shares
 * @return 0 on success, -1 on error
 */

int shamir_distribute( int shades_count, const char* file_name,  int images_count, const char** cover_files);
void scramble_flattened_image(Mod257Pixel* image, int size, int64_t seed);
void unscramble_flattened_image(Mod257Pixel* image, int size, int64_t seed); 
void get_shares(Mod257Pixel* pixel_values, int k, int n, Mod257Pixel* result);
int pow_mod(int base, int exp, int mod);
uint16_t evaluate_shamir(Mod257Pixel* pixel_values, int k, int x);
void process_image(BMP257Image * image, Mod257Pixel** pixels, int k, int n);
void unflatten_matrix(Mod257Pixel* flat, int height, int width, Mod257Pixel** matrix); 
void flatten_matrix(Mod257Pixel** matrix, int height, int width, Mod257Pixel* flat); 
void recover_polynomial(int* x_coords, Mod257Pixel* shares, int k, Mod257Pixel* coefficients); 
void unprocess_image(BMP257Image *image, Mod257Pixel **processed_pixels, int k, int n);
void scramble_flattened_image_xor(Mod257Pixel* image, int size, int64_t seed);
void unprocess_image_partial(
    BMP257Image *image,
    Mod257Pixel **processed_pixels,
    int k,
    int n,
    const int *indices  // Length k; values in range 0 to n-1
);
void cover_in_files(BMP257Image* secret_image, const char** cover_files, int k, int n);
#endif
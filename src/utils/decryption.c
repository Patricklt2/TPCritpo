#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <decryption.h>
#include <encryption.h>

int check_seed(char** cover_files, int k){
    BMP257Image* first_cover = read_bmp_257(cover_files[0]);
    if (!first_cover) {
        fprintf(stderr, "Error reading first cover file\n");
        return 1; // Return error code
    }
    uint16_t seed = first_cover->file_header.reserved2;
    for (int i = 1; cover_files[i] != NULL && i < k; i++) {
        BMP257Image* cover = read_bmp_257(cover_files[i]);
        if (!cover) {
            fprintf(stderr, "Error reading cover file '%s'\n", cover_files[i]);
            free_bmp257_image(first_cover);
            return 1; // Return error code
        }
        if (cover->file_header.reserved2 != seed) {
            fprintf(stderr, "Seed mismatch in cover files\n");
            free_bmp257_image(cover);
            free_bmp257_image(first_cover);
            return 1; // Return error code
        }
        free_bmp257_image(cover);
    }
    free_bmp257_image(first_cover);
    return 0; // Return 0 if all seeds match
}

int shamir_recover(int k, char* output_file, int n, char** cover_files) {

    int images = n;

    if (images == 0) {
        images = count_files(cover_files);
    }

    if (images < k) {
        fprintf(stderr, "Error: n must be greater than or equal to k\n");
        return 1; // Return error code
    }

    if (check_seed(cover_files, k)) {
        fprintf(stderr, "Error: Invalid seed in cover files\n");
        return 1; // Return error code
    }
    char* aux = getcwd(NULL, 0); // Get current working directory

    char* complete_file_name = malloc(strlen(output_file)+ strlen(aux) + 2);
    complete_file_name = strcpy(complete_file_name, aux);
    complete_file_name = strcat(complete_file_name, "/");
    complete_file_name = strcat(complete_file_name, output_file);
    printf("Complete file name: %s\n", complete_file_name); // Debugging line

    recover_from_files_v2(k, images, cover_files, complete_file_name);

    return 0; // Return 0 on success

}

void unflatten_matrix(Mod257Pixel* flat, int height, int width, Mod257Pixel** matrix) {
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            matrix[row][col] = flat[row * width + col];
        }
    }
}

int modmul(int a, int b, int mod) { 
    return (a * b) % mod;
}

int modinv(int a, int mod) {
    a %= mod;
    int res = 1;
    for (int exp = mod-2; exp > 0; exp >>= 1) {
        if (exp & 1) res = modmul(res, a, mod);
        a = modmul(a, a, mod);
    }
    return res;
}

/**
 * Recovers the original polynomial coefficients (pixel values) from a set of shares
 * using Lagrange interpolation over a finite field (mod 257).
 *
 * @param x_coords An array of integers representing the x-coordinates of the shares.
 *                 These should be distinct and typically start from 1 to n.
 * @param shares An array of Mod257Pixel structures representing the y-values
 *               (shares) at the corresponding x-coordinates.
 * @param k The number of shares used to reconstruct the original polynomial.
 *          This is also the number of coefficients to recover.
 * @param coefficients An output array of Mod257Pixel structures where the reconstructed
 *                     polynomial coefficients (i.e., original pixel values) will be stored.
 */
void recover_polynomial(int* x_coords, Mod257Pixel* shares, int k, Mod257Pixel* coefficients) {
    int field_coeffs[MAX_K] = {0};

    for (int i = 0; i < k; i++) {
        int y_i = shares[i].is_257 ? 0 : shares[i].value;
        
        int numerator[MAX_K] = {0};
        numerator[0] = 1;
        int denom = 1;
        int degree = 0;

        for (int j = 0; j < k; j++) {
            if (j == i) continue;
            
            // Multiply numerator by (x - x_j)
            int temp[MAX_K] = {0};
            for (int m = 0; m <= degree; m++) {
                temp[m] = (temp[m] + modmul(numerator[m], PRIME - x_coords[j], PRIME)) % PRIME;
                temp[m+1] = (temp[m+1] + numerator[m]) % PRIME;
            }
            degree++;
            memcpy(numerator, temp, sizeof(temp));
            
            // Update denominator
            denom = modmul(denom, (x_coords[i] - x_coords[j] + PRIME) % PRIME, PRIME);
        }

        int inv_denom = modinv(denom, PRIME);
        for (int m = 0; m < k; m++) {
            int term = modmul(modmul(y_i, numerator[m], PRIME), inv_denom, PRIME);
            field_coeffs[m] = (field_coeffs[m] + term) % PRIME;
        }
    }

    for (int i = 0; i < k; i++) {
        coefficients[i].value = (field_coeffs[i] == 256) ? 0 : (uint8_t)field_coeffs[i];
        coefficients[i].is_257 = (field_coeffs[i] == 256) ? 1 : 0;
    }
}

//Auxiliar function
int count_strings(char **arr) {
    int count = 0;
    while (arr[count] != NULL) {
        count++;
    }
    return count;
}

// I need to create the array from k shadows
// process pixels and unflatten array


void unflatten_mod257_matrix(Mod257Pixel** matrix, int width, int height, const Mod257Pixel* flat) {
    if (!matrix || !flat) return;

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            // Reconstruct matrix by reversing the flattening order
            matrix[height - 1 - row][col] = flat[row * width + col];
        }
    }
}

void  flatten_mod257_matrix(Mod257Pixel** pixels, int width, int height, Mod257Pixel* flat) {
    if (!pixels || !flat || width <= 0 || height <= 0) {
        fprintf(stderr, "Invalid input dimensions\n");
        return ;
    }

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            // Reverse row order (last row becomes first in flattened array)
            flat[row*width + col] = pixels[row][col]; // Ensure mod 257
        }
    }

    return;
}
void recover_from_files_v2(int k, int n, char** cover_files, char* output_file) {
    int shadow_indices[n];
    int shadow_count = count_strings(cover_files);

    BMP257Image *first_cover = read_bmp_257(cover_files[0]);
    if (!first_cover) {
        fprintf(stderr, "Error reading first cover file\n");
        return;
    }

    int height = first_cover->info_header.height;
    int width = first_cover->info_header.width;
    int total_pixels = height * width;
    int padded_pixels = ((total_pixels + k - 1) / k) * k;

    Mod257Pixel **processed_pixels = malloc(sizeof(Mod257Pixel*) * (padded_pixels / k));
    if (!processed_pixels) {
        fprintf(stderr, "Memory allocation failed for processed_pixels\n");
        free_bmp257_image(first_cover);
        return;
    }

    for (int i = 0; i < padded_pixels / k; i++) {
        processed_pixels[i] = calloc(n, sizeof(Mod257Pixel));
        if (!processed_pixels[i]) {
            fprintf(stderr, "Memory allocation failed for processed_pixels[%d]\n", i);
            for (int j = 0; j < i; j++) free(processed_pixels[j]);
            free(processed_pixels);
            free_bmp257_image(first_cover);
            return;
        }
    }

    uint16_t seed = first_cover->file_header.reserved2;

    for (int i = 0; i < n; i++) {
        if (!cover_files[i]) break;

        BMP257Image* cover = (i == 0) ? first_cover : read_bmp_257(cover_files[i]);
        if (!cover) {
            fprintf(stderr, "Error reading cover file '%s'\n", cover_files[i]);
            continue;
        }

        shadow_indices[i] = cover->file_header.reserved1;

        Mod257Pixel *flat_pixels = malloc(sizeof(Mod257Pixel) * total_pixels);
        if (!flat_pixels) {
            fprintf(stderr, "Memory allocation failed for flat_pixels\n");
            if (i != 0) free_bmp257_image(cover);
            continue;
        }

        flatten_mod257_matrix(cover->pixels, height, width, flat_pixels);

    char dumpFilename[300];
    snprintf(dumpFilename, sizeof(dumpFilename), "cover{%d}.txt", i);

    FILE* dumpFile = fopen(dumpFilename, "wb");

    if (dumpFile) {

        for (int y = 0; y < total_pixels; y++) {
                fprintf(dumpFile, "%02X ", flat_pixels[y].value);
                if((y+1) % 16 == 0)
                    fprintf(dumpFile, "\n");
        }

        fclose(dumpFile);
    } else {
        fprintf(stderr, "Could not open pixels_dump.txt for writing\n");
    }

        for (int j = 0; j < padded_pixels / k; j++) {
            uint8_t byte = 0;
            if (k >= 8) { // [ 1 1 1 1 | 1 1 1 1 | 1 1 1 1 | 1 1 1 1 ] => los primeros 8 bits
                for (int b = 0; b < 8; b++) {
                    int pixel_idx = j * 8 + b;
                    byte <<= 1;
                    if (pixel_idx < total_pixels)
                        byte |= (flat_pixels[pixel_idx].value & 1);
                }
            }else if ( k >= 4 && k < 8 ) { // [ 1 1 | 1 1 | 1 1 | 1 1 ] => los 2 primeros 6, 4 , 2 ,0
                for (int b = 0; b < 4; b++) {
                    int pixel_idx = j * k + b;
                    byte <<= 2;
                    if (pixel_idx < total_pixels)
                        byte |= (flat_pixels[pixel_idx].value & 0x3); // 0000 0011
                }
            }else if (k == 3) {
                for (int b = 0; b < 3; b++) {
                    int pixel_idx = j * k + b;
                    if(b != 2){
                        byte <<= 3;
                        if (pixel_idx < total_pixels)
                            byte |= (flat_pixels[pixel_idx].value & 0x7); // 0000 0111
                    }else{
                        byte <<= 2;
                        if (pixel_idx < total_pixels){
                            char val = flat_pixels[pixel_idx].value >> 1;
                            byte |= (val & 0x3);
                        }
                    }
                }
            } else {
                for (int b = 0; b < 2; b++) {
                    int pixel_idx = j * k + b;
                    byte <<= 4;
                    if (pixel_idx < total_pixels)
                        byte |= (flat_pixels[pixel_idx].value & 0xF);
                }
            }

            processed_pixels[j][shadow_indices[i] - 1].value = byte;
            processed_pixels[j][shadow_indices[i] - 1].is_257 = 0;
        }

        free(flat_pixels);
        if (i != 0) free_bmp257_image(cover);
    }

    FILE *fp = fopen("shadows_output.txt", "w");
    if (!fp) {
        fprintf(stderr, "Error opening shadows output file\n");
        // You can decide whether to continue or return here
    }

    fprintf(fp, "Recovered shadows:\n");
    for (int shadow = 0; shadow < n; shadow++) {
        fprintf(fp, "Shadow %d (shadow_indices[%d] = %d):\n", shadow + 1, shadow, shadow_indices[shadow]);
        for (int j = 0; j < padded_pixels / k; j++) {
            fprintf(fp, "%02X ", processed_pixels[j][shadow_indices[shadow] - 1].value);
            if ((j + 1) % 16 == 0) fprintf(fp, "\n");  // 16 bytes per line
        }
        fprintf(fp, "\n\n");
    }

    fclose(fp);
    
    BMP257Image *outImage = create_bmp_257(NULL, width, height);
    if (!outImage) {
        fprintf(stderr, "Error creating output image\n");
        for (int i = 0; i < padded_pixels / k; i++) free(processed_pixels[i]);
        free(processed_pixels);
        free_bmp257_image(first_cover);
        return;
    }

    unprocess_image_partial_v2(outImage, processed_pixels, k, n, shadow_indices, shadow_count, seed);  // new trim-aware version

    write_bmp_257(outImage, output_file);

    for (int i = 0; i < padded_pixels / k; i++) free(processed_pixels[i]);
    free(processed_pixels);
    free_bmp257_image(outImage);
    free_bmp257_image(first_cover);
}

void unprocess_image_partial_v2(BMP257Image *image, Mod257Pixel **processed_pixels, int k, int n, const int *shadow_indices, int num_shadows, uint16_t seed) {
    if (num_shadows < k) {
        fprintf(stderr, "Not enough shares to reconstruct the secret\n");
        return;
    }

    int height = image->info_header.height;
    int width = image->info_header.width;
    int total_pixels = height * width;
    int num_blocks = (total_pixels + k - 1) / k;

    Mod257Pixel *flattened_pixels = malloc(sizeof(Mod257Pixel) * total_pixels);
    if (!flattened_pixels) {
        fprintf(stderr, "Memory allocation failed for flattened_pixels\n");
        return;
    }

    for (int block_idx = 0, pixel_idx = 0; block_idx < num_blocks; block_idx++, pixel_idx += k) {
        Mod257Pixel selected_shares[k];
        int x_coords[k];

        for (int j = 0; j < k; j++) {
            int shadow_index = shadow_indices[j];
            x_coords[j] = shadow_index;
            selected_shares[j] = processed_pixels[block_idx][shadow_index - 1]; // 1-based to 0-based
        }

        Mod257Pixel coefficients[k];
        recover_polynomial(x_coords, selected_shares, k, coefficients);

        int copy_len = (total_pixels - pixel_idx < k) ? (total_pixels - pixel_idx) : k;
        for (int w = 0; w < copy_len; w++) {
            flattened_pixels[pixel_idx + w] = coefficients[w];
        }
    }

    scramble_flattened_image_xor(flattened_pixels, total_pixels, seed);
    unflatten_matrix(flattened_pixels, height, width, image->pixels);

    free(flattened_pixels);
}


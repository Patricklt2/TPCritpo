#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <decryption.h>
#include <lagrange.h>
#include <encryption.h>




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
int count_strings(const char **arr) {
    int count = 0;
    while (arr[count] != NULL) {
        count++;
    }
    return count;
}

// I need to create the array from k shadows
// process pixels and unflatten array
void recover_from_files_v2(int k, int n, const char** cover_files, char* output_file) {
    // Read first cover file to get dimensions and seed

    int shadow_count = count_strings(cover_files);
    int shadow_indices[k];

    BMP257Image *first_cover = read_bmp_257(cover_files[0]);
    if (!first_cover) {
        fprintf(stderr, "Error reading first cover file\n");
        return;
    }

    int height = first_cover->info_header.height;
    int width = first_cover->info_header.width;
    int total_pixels = height * width;
    uint16_t seed = first_cover->file_header.reserved2;

    // Allocate processed pixels array
    Mod257Pixel **processed_pixels = malloc(sizeof(Mod257Pixel*) * (total_pixels / k));
    if (!processed_pixels) {
        fprintf(stderr, "Memory allocation failed for processed_pixels\n");
        free_bmp257_image(first_cover);
        return;
    }

    // Initialize processed pixels
    for (int i = 0; i < total_pixels / k; i++) {
        processed_pixels[i] = malloc(sizeof(Mod257Pixel) * n);
        if (!processed_pixels[i]) {
            fprintf(stderr, "Memory allocation failed for processed_pixels[%d]\n", i);
            // Clean up previously allocated memory
            for (int j = 0; j < i; j++) free(processed_pixels[j]);
            free(processed_pixels);
            free_bmp257_image(first_cover);
            return;
        }
    }

    // Process all cover files to extract shares
    for (int i = 0; i < n; i++) {
        if(cover_files[i] == NULL){
            break;
        }
        BMP257Image* cover = (i == 0) ? first_cover : read_bmp_257(cover_files[i]);
        shadow_indices[i] = cover->file_header.reserved1;

        if (!cover) {
            fprintf(stderr, "Error reading cover file '%s'\n", cover_files[i]);
            continue;
        }

        Mod257Pixel *flat_pixels = malloc(sizeof(Mod257Pixel) * total_pixels);
        if (!flat_pixels) {
            fprintf(stderr, "Memory allocation failed for flat_pixels\n");
            if (i != 0) free_bmp257_image(cover);
            continue;
        }

        flatten_matrix(cover->pixels, height, width, flat_pixels);

        // Extract shares from LSBs
        for (int j = 0; j < total_pixels / k; j++) {
            uint8_t extracted_byte = 0;
            for (int b = 0; b < 8; b++) {
                int pixel_idx = j * 8 + b;
                if (pixel_idx >= total_pixels) break;  // Prevent overflow
                
                extracted_byte <<= 1;
                extracted_byte |= (flat_pixels[pixel_idx].value & 1);
            }
            processed_pixels[j][shadow_indices[i]-1].value = extracted_byte;
            processed_pixels[j][shadow_indices[i]-1].is_257 = 0;
        }

        free(flat_pixels);
        if (i != 0) free_bmp257_image(cover);
    }


    // Create output image and reconstruct
    BMP257Image *outImage = create_bmp_257(NULL, height, width);
    if (!outImage) {
        fprintf(stderr, "Error creating output image\n");
        // Clean up
        for (int i = 0; i < total_pixels / k; i++) free(processed_pixels[i]);
        free(processed_pixels);
        free_bmp257_image(first_cover);
        return;
    }

    unprocess_image_partial_v2(outImage, processed_pixels, k, n, shadow_indices, shadow_count, seed);

    // Write output file
    write_bmp_257(outImage, output_file);

    // Clean up
    for (int i = 0; i < total_pixels / k; i++) free(processed_pixels[i]);
    free(processed_pixels);
    free_bmp257_image(outImage);
    free_bmp257_image(first_cover);
}

void unprocess_image_partial_v2(BMP257Image *image, Mod257Pixel **processed_pixels, int k, int n, const int *shadow_indices, int num_shadows, uint16_t seed) {
    if (num_shadows < k) return;  // Not enough shares to reconstruct

    int height = image->info_header.height;
    int width = image->info_header.width;
    int total_pixels = height * width;
    int num_blocks = (total_pixels + k - 1) / k;

    Mod257Pixel *flattened_pixels = malloc(sizeof(Mod257Pixel) * total_pixels);
    if (!flattened_pixels) return;

    // For each block of k pixels
    for (int i = 0, block_idx = 0; i < total_pixels && block_idx < num_blocks; i += k, block_idx++) {
        Mod257Pixel selected_shares[n];
        int x_coords[n];

        for (int j = 0; j < k; j++) {
            int shadow_index = shadow_indices[j]; // Get the j-th available shadow
            x_coords[j] = shadow_index;       // Convert 0-based index to 1-based x
            selected_shares[j] = processed_pixels[block_idx][shadow_index-1]; // Share from that shadow
        }

        Mod257Pixel coefficients[n];
        recover_polynomial(x_coords, selected_shares, k, coefficients);

        int copy_len = (total_pixels - i) < k ? (total_pixels - i) : k;
        for (int w = 0; w < copy_len; w++) {
            flattened_pixels[i + w] = coefficients[w];
        }
    }

    scramble_flattened_image_xor(flattened_pixels, total_pixels, seed);
    unflatten_matrix(flattened_pixels, height, width, image->pixels);
    free(flattened_pixels);
}


//-------------------------- Deprecated War Zone -------------------------------------------------------------------

void read_from_shares( Mod257Pixel* plane_pixels, Mod257Pixel pixel_values[], int share_index, int pixel_index) {
    char read_value = 0;
    for(int i = 0; i < 8; i++) {
        // Read the LSB of the pixel and set it to the i-th bit of read_value
        read_value |= ((plane_pixels[pixel_index + i].value & 0x01) << (7-i));
    }
    pixel_values[share_index].value = read_value;
    
}

void recover_from_files(int k, int n, const char** cover_files, char* output_file) {
    // Placeholder for the actual implementation of recovering shares from files
    // This function should read the shares from cover_files and reconstruct the original image
    printf("Recovering secret image from %d files with a threshold of %d\n", n, k);

    uint32_t max_bytes = 300*(300/k);
    uint16_t seed = 0;

    Mod257Pixel* recovered_pixels[300*300/k];
    for ( int i=0; i< 300*300/k; i++ ){
        recovered_pixels[i] = malloc(sizeof(Mod257Pixel) * n);
    }
    printf("hola\n");
    
    for( int i=0; i < n; i++ ){
        printf("i:%d\n",i);
        BMP257Image* cover_image = read_bmp_257(cover_files[i]);
        if (!cover_image) {
            fprintf(stderr, "Error reading cover file '%s'\n", cover_files[i]);
            continue;
        }
        
        Mod257Pixel* plane_pixels = malloc(sizeof(Mod257Pixel) * cover_image->info_header.width * cover_image->info_header.height);
        flatten_matrix(cover_image->pixels, cover_image->info_header.height, cover_image->info_header.width, plane_pixels);
        for( int j=0,a=0; j < max_bytes-1; j+=k,a++ ){
            printf("j:%d\n", j);
            read_from_shares(plane_pixels, recovered_pixels[a], i, j); // Reads the transport images with the assigned shares
        }
        seed = cover_image->file_header.reserved2; // Get the seed from the cover image

        free(plane_pixels);
        free_bmp257_image(cover_image);
    }

    BMP257Image* secret_image = create_bmp_257(NULL, 300, 300);
    secret_image->file_header.reserved2 = seed; // seed
    unprocess_image(secret_image,recovered_pixels, k, n, seed);

    write_bmp_257(secret_image, output_file);
    // Free allocated memory
    for (int i = 0; i < max_bytes; i++) {
        free(recovered_pixels[i]);
    }
    free_bmp257_image(secret_image);

}   


/**
 * Reconstructs the original image from processed pixels using
 * (k, n) secret sharing recovery. The shares are used to recover
 * the original pixels, which are then unscrambled and reshaped into
 * the original 2D image format.
 *
 * @param image The BMP257Image structure to store the reconstructed image.
 *              The `pixels` field will be overwritten with recovered data.
 * @param processed_pixels An array of Mod257Pixel pointers. Each pointer
 *                         holds n shares corresponding to a block of k pixels.
 * @param k The number of pixels originally shared together (threshold).
 * @param n The number of shares available per block (total number of shares).
 * @param indices Subset of the shadows to be used
 */
void unprocess_image_partial(BMP257Image *image, Mod257Pixel **processed_pixels, int k, int n, const int *indices, uint16_t seed) {
    int height = image->info_header.height;
    int width = image->info_header.width;
    int total_pixels = height * width;
    int num_blocks = total_pixels / k;

    Mod257Pixel *flattened_pixels = malloc(sizeof(Mod257Pixel) * total_pixels);
    if (!flattened_pixels) return;

    // Build x_coords from provided indices (typically x=1..n)
    int x_coords[k];
    for (int i = 0; i < k; i++) {
        x_coords[i] = indices[i] + 1;
    }

    for (int i = 0, block_idx = 0; i < total_pixels && block_idx < num_blocks; i += k, block_idx++) {
        // Pick k shares using the indices
        Mod257Pixel selected_shares[k];
        for (int j = 0; j < k; j++) {
            selected_shares[j] = processed_pixels[block_idx][indices[j]];
        }

        // Reconstruct coefficients (original pixels) using only the selected shares
        Mod257Pixel coefficients[k];
        recover_polynomial(x_coords, selected_shares, k, coefficients);

        int copy_len = (total_pixels - i) < k ? (total_pixels - i) : k;
        for (int w = 0; w < copy_len; w++) {
            flattened_pixels[i + w] = coefficients[w];
        }
    }

    scramble_flattened_image_xor(flattened_pixels, total_pixels, seed);
    unflatten_matrix(flattened_pixels, height, width, image->pixels);
    free(flattened_pixels);
}


/**
 * Reconstructs the original image from processed pixels using
 * (k, n) secret sharing recovery. The shares are used to recover
 * the original pixels, which are then unscrambled and reshaped into
 * the original 2D image format.
 *
 * @param image The BMP257Image structure to store the reconstructed image.
 *              The `pixels` field will be overwritten with recovered data.
 * @param processed_pixels An array of Mod257Pixel pointers. Each pointer
 *                         holds n shares corresponding to a block of k pixels.
 * @param k The number of pixels originally shared together (threshold).
 * @param n The number of shares available per block (total number of shares).
 */
void unprocess_image(BMP257Image *image, Mod257Pixel **processed_pixels, int k, int n, uint16_t seed) {
    int height = image->info_header.height;
    int width = image->info_header.width;
    int total_pixels = height * width;
    int num_blocks = (total_pixels + k - 1) / k;  // Ceiling division

    // Allocate flattened array for reconstructed pixels
    Mod257Pixel *flattened_pixels = malloc(sizeof(Mod257Pixel) * total_pixels);
    if (!flattened_pixels) return;

    // Standard x-coordinates for shares (1..n)
    int x_coords[n];
    for (int i = 0; i < n; i++) {
        x_coords[i] = i + 1;  // Shares are typically at x=1, x=2, ..., x=n
    }

    // For each block of k pixels
    for (int i = 0, block_idx = 0; i < total_pixels && block_idx < num_blocks; i += k, block_idx++) {
        // Get the shares for this block
        Mod257Pixel *shares = processed_pixels[block_idx];
        
        // Reconstruct the polynomial coefficients (which are our original pixel values)
        Mod257Pixel coefficients[k];
        recover_polynomial(x_coords, shares, k, coefficients);
        
        // The first k coefficients are our original pixel values
        int copy_len = (total_pixels - i) < k ? (total_pixels - i) : k;
        for (int w = 0; w < copy_len; w++) {
            flattened_pixels[i + w] = coefficients[w];
        }
    }

    // Undo the scrambling
    scramble_flattened_image_xor(flattened_pixels, total_pixels, seed);

    // Unflatten back into the image matrix
    unflatten_matrix(flattened_pixels, height, width, image->pixels);

    free(flattened_pixels);
}
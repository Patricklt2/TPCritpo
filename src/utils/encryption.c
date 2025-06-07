
#include <encryption.h>

#define PRIME 257
#define MAX_BYTES 300*(300/8)

int shamir_distribute( int k, const char* file_name, int n, const char** cover_files) {
    // Placeholder for the actual implementation of Shamir's Secret Sharing distribution
    // This function should distribute the secret image into n shares using k as the threshold
    printf("Distributing secret image '%s' into %d shares with a threshold of %d\n", file_name, n, k);
    
    if ( n < k ) {
        fprintf(stderr, "Error: n must be greater than or equal to k\n");
        return 1; // Return error code
    }

    BMP257Image* secret_image = read_bmp_257(file_name);

    char shades[k];

    // < 8
    if( k < 8 ){
//        cover_in_files_less(secret_image, cover_files, k, n);
    }else if( k == 8 ){
        cover_in_files(secret_image, cover_files, k, n);
    }else{
  //      cover_in_files_more(secret_image, cover_files, k, n);
    }

    // Free the secret image resources
    free_bmp257_image(secret_image);
    
    return 0; // Return 0 on success
}

void write_with_shares( Mod257Pixel* plane_pixels, Mod257Pixel pixel_values[], int share_index, int pixel_index) {
    
    for(int i = 0; i < 8; i++) {
        // Clear the LSB of the pixel and set it to the i-th bit of pixel_values[share_index].value
        plane_pixels[pixel_index + i].value = (plane_pixels[pixel_index + i].value & 0xFE) | ((pixel_values[share_index].value >> 8-i-1) & 1);
    }
    
}

void cover_in_files(BMP257Image* secret_image, const char** cover_files, int k, int n) {
    printf("Distributing into 8 cover files\n");
    long max_bytes = secret_image->info_header.with * secret_image->info_header.height;
    
    Mod257Pixel** processed_pixels = malloc(max_bytes * sizeof(Mod257Pixel*)); // n array of pixel values
    process_image(secret_image, processed_pixels, k, n);
     
    for ( int j = 0; j < max_bytes; j+=k ) {
        for( int i = 0; i < n; i++ ) {
            uint16_t seed = 1000; // Unique seed
            BMP257Image* cover_image = read_bmp_257(cover_files[i]);
            if (!cover_image) {
                fprintf(stderr, "Error reading cover file '%s'\n", cover_files[i]);
                break;
            }
            sercret_image->file_header.reserved1 = i + 1; // Assigning the share index to reserved1
            secret_image->file_header.reserved2 = seed; // Assigning the seed to reserved2

            Mod257Pixel* plane_pixels = malloc(sizeof(Mod257Pixel) * cover_image->info_header.width * cover_image->info_header.height);
            if (!plane_pixels) {
                fprintf(stderr, "Memory allocation failed for plane pixels\n");
                free_bmp257_image(cover_image);
                break;
            }
            flatten_matrix(cover_image->pixels, cover_image->info_header.height, cover_image->info_header.width, plane_pixels);

            write_with_shares(plane_pixels, processed_pixels[j], i, j); // Writes the transport images with the asigned shares

            unflatten_matrix(plane_pixels, cover_image->info_header.height, cover_image->info_header.width, cover_image->pixels);
            // Save the modified cover image
            char output_filename[256];
            snprintf(output_filename, sizeof(output_filename), "encodings/share%d.bmp", i + 1);
            write_bmp_257(cover_files[i], output_filename);
            free(plane_pixels);
            free_bmp257_image(cover_image);
        }
    }

    // Free the processed pixels
    for (int i = 0; i < max_bytes; i++) {
        free(processed_pixels[i]);
    }
    free(processed_pixels);

}


void scramble_flattened_image_xor(Mod257Pixel* image, int size, int64_t seed) {
    setSeed(seed);
    for (int i = 0; i < size; i++) {
        uint8_t randByte = nextChar();
        if (image[i].value == 256) {
            continue;
        }
        image[i].value  = image[i].value  ^ randByte;
    }
}


//Fisher Yates Style scramble
void scramble_flattened_image(Mod257Pixel* image, int size, int64_t seed) {
    setSeed(seed);
    for (int i = size - 1; i > 0; i--) {
        int j = nextChar() % (i + 1);

        Mod257Pixel temp = image[i];
        image[i] = image[j];
        image[j] = temp;
    }
}

//Fisher Yates Style unscramble
void unscramble_flattened_image(Mod257Pixel* image, int size, int64_t seed) {
    setSeed(seed);
    // Store original permutation
    int* indices = malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        indices[i] = i;
    }

    // Apply same shuffle to indices array
    for (int i = size - 1; i > 0; i--) {
        int j = nextChar() % (i + 1);
        int tmp = indices[i];
        indices[i] = indices[j];
        indices[j] = tmp;
    }

    // Reverse permutation
    Mod257Pixel* copy = malloc(size * sizeof(Mod257Pixel));
    for (int i = 0; i < size; i++) {
        copy[indices[i]] = image[i];
    }

    for (int i = 0; i < size; i++) {
        image[i] = copy[i];
    }

    free(copy);
    free(indices);
}



int pow_mod(int base, int exp, int mod) {
    int result = 1;
    for(int i = 0; i < exp; i++) {
        result = (result * base) % mod;
    }
    return result;
}

// pixel values --> array aplanado de pixeles
// genera las n shares
/**
 * Generates n shares from k original Mod257Pixel values using Shamir's Secret Sharing
 * over a finite field (mod 257). The function ensures that none of the generated shares
 * are equal to 256 (the special reserved value used as a 257th symbol).
 *
 * @param pixel_values An array of k Mod257Pixel values representing the original data (e.g., pixels).
 *                     These values form the coefficients of a secret polynomial.
 * @param k The threshold number of shares required to reconstruct the original data.
 *          Also the number of coefficients in the secret polynomial.
 * @param n The total number of shares to generate.
 * @param result An output array of n Mod257Pixel structures to store the resulting shares.
 *               Each share corresponds to a point on the polynomial (x = 1 to n).
 */
void get_shares(Mod257Pixel* pixel_values, int k, int n, Mod257Pixel* result) {
    for (int i = 1; i <= n; i++) {
        while (1) {
            uint16_t eval = evaluate_shamir(pixel_values, k, i);
            if (eval != 256) {
                result[i - 1].value = (uint8_t)eval;
                result[i - 1].is_257 = 0;
                break;
            }

            // Step 5: Find lowest non-zero pixel and decrement it
            int min_index = -1;
            uint16_t min_value = 257;  // higher than any valid value

            for (int j = 0; j < k; j++) {
                if (pixel_values[j].value != 0 && pixel_values[j].value < min_value) {
                    min_value = pixel_values[j].value;
                    min_index = j;
                }
            }

            if (min_index != -1) {
                pixel_values[min_index].value = (pixel_values[min_index].value + PRIME - 1) % PRIME;
                i = 1;
            } else {
                // Shouldn't happen: all coefficients are 0
                result[i - 1].value = 0;
                result[i - 1].is_257 = 0;
                i=1;
                break;
            }
        }
    }
}
/*
    @param pixel_values: Array of Mod257Pixel pointers containing k pixel values
    @param k: Size of share array
    @param n: share operating
    @return value of evaluation
*/
uint16_t evaluate_shamir(Mod257Pixel* pixel_values, int k, int x) {
    uint16_t aux = 0;

    for (int i = 0; i < k; i++) {
        uint16_t term = (pixel_values[i].value * pow_mod(x, i, PRIME)) % PRIME;
        aux = (aux + term) % PRIME;
    }

    return aux;
}

//this will scramble and process the shares
// [b0, b1, b2, b3, b4, .. bn]
// B --> b0 b1 b2 b3 b4 b5 b6 b7
// [0][B0 B1 B2 B3 B4 B5 .. Bn]
//  ...
// [11250][B0 B1 B2 B3 B4 .. Bn]
// ------------------------------------------ 
/**
 * Processes the pixels of a BMP257Image using a (k, n) secret sharing scheme.
 * The original pixels are flattened, scrambled, split into blocks of size k,
 * and shared into n shares using polynomial-based secret sharing.
 *
 * @param image The BMP257Image to process. Contains the original image data.
 * @param processed_pixels A pre-allocated array of Mod257Pixel pointers.
 *                         Each pointer will point to an array of n Mod257Pixels
 *                         representing the shares of a block of k pixels.
 * @param k The number of original pixels in each block. Also, the threshold
 *          number of shares required to reconstruct the block.
 * @param n The total number of shares to generate per block.
 */
void process_image(BMP257Image *image, Mod257Pixel **processed_pixels, int k, int n) {
    int height = image->info_header.height;
    int width = image->info_header.width;
    int total_pixels = height * width;

    Mod257Pixel *flattened_pixels = malloc(sizeof(Mod257Pixel) * total_pixels);
    if (!flattened_pixels) return;

    flatten_matrix(image->pixels, height, width, flattened_pixels);
    scramble_flattened_image_xor(flattened_pixels, total_pixels, 1000);

    //Agarro de a tandas de k pixeles
    for (int i = 0, j = 0; i < total_pixels; i += k, j++) {
        Mod257Pixel *aux = malloc(sizeof(Mod257Pixel) * k);
        if (!aux) 
        break;

        for (int w = 0; w < k; w++) {
            aux[w] = flattened_pixels[i + w];
        }

        Mod257Pixel *results = malloc(sizeof(Mod257Pixel) * n);
        if (!results) {
            free(aux);
            break;
        }

        get_shares(aux, k, n, results);
        processed_pixels[j] = results;

        free(aux);
    }

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
void unprocess_image(BMP257Image *image, Mod257Pixel **processed_pixels, int k, int n) {
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
    scramble_flattened_image_xor(flattened_pixels, total_pixels, 1000);

    // Unflatten back into the image matrix
    unflatten_matrix(flattened_pixels, height, width, image->pixels);

    free(flattened_pixels);
}

void flatten_matrix(Mod257Pixel** matrix, int height, int width, Mod257Pixel* flat) {
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            flat[row * width + col] = matrix[row][col];
        }
    }
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
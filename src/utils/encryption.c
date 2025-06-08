
#include <encryption.h>

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

    uint16_t seed = 1002;

    // < 8
    if( k < 8 ){
//        cover_in_files_less(secret_image, cover_files, k, n);
    }else if( k == 8 ){
        cover_in_files_v2(secret_image, cover_files, k, n, seed);
    }else{
  //      cover_in_files_more(secret_image, cover_files, k, n);
    }

    // Free the secret image resources
    free_bmp257_image(secret_image);
    
    return 0; // Return 0 on success
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
// [11250] --> [90000] () LSB byte del carrier
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
void process_image(BMP257Image *image, Mod257Pixel **processed_pixels, int k, int n, uint16_t seed) {
    int height = image->info_header.height;
    int width = image->info_header.width;
    int total_pixels = height * width;

    Mod257Pixel *flattened_pixels = malloc(sizeof(Mod257Pixel) * total_pixels);
    if (!flattened_pixels) return;

    flatten_matrix(image->pixels, height, width, flattened_pixels);
    scramble_flattened_image_xor(flattened_pixels, total_pixels, seed);

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

void flatten_matrix(Mod257Pixel** matrix, int height, int width, Mod257Pixel* flat) {
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            flat[row * width + col] = matrix[row][col];
        }
    }
}


void cover_in_files_v2(BMP257Image* secret_image, const char** cover_files, int k, int n, uint16_t seed) {
    int height = secret_image->info_header.height;
    int width = secret_image->info_header.width;
    int total_pixels = height * width;

    // Allocate processed pixels array
    Mod257Pixel **processed_pixels = malloc(sizeof(Mod257Pixel*) * (total_pixels / k));
    if (!processed_pixels) {
        fprintf(stderr, "Memory allocation failed for processed_pixels\n");
        return;
    }

    process_image(secret_image, processed_pixels, k, n, seed);

    for (int i = 0; i < n; i++) {
        BMP257Image* carrier = read_bmp_257(cover_files[i]);
        if (!carrier) {
            fprintf(stderr, "Error reading cover file '%s'\n", cover_files[i]);
            continue;
        }

        // Verify carrier dimensions match secret image
        if (carrier->info_header.height != height || carrier->info_header.width != width) {
            fprintf(stderr, "Cover image dimensions don't match secret image\n");
            free_bmp257_image(carrier);
            continue;
        }

        Mod257Pixel *flat_pixels = malloc(sizeof(Mod257Pixel) * total_pixels);
        if (!flat_pixels) {
            fprintf(stderr, "Memory allocation failed for flat_pixels\n");
            free_bmp257_image(carrier);
            continue;
        }

        flatten_matrix(carrier->pixels, height, width, flat_pixels);

        // Embed shares in LSBs
        for (int j = 0; j < total_pixels / k; j++) {
            uint8_t secret_val = processed_pixels[j][i].value;
            for (int bit_idx = 0; bit_idx < 8; bit_idx++) {
                int pixel_idx = j * 8 + bit_idx;
                if (pixel_idx >= total_pixels) break;  // Prevent overflow
                
                uint8_t bit_val = (secret_val >> (7 - bit_idx)) & 1;
                flat_pixels[pixel_idx].value = (flat_pixels[pixel_idx].value & 0xFE) | bit_val;
            }
        }

        unflatten_matrix(flat_pixels, height, width, carrier->pixels);
        carrier->file_header.reserved1 = i + 1;  // Share index
        carrier->file_header.reserved2 = seed;   // Common seed

        char output_filename[256];
        snprintf(output_filename, sizeof(output_filename), "encodings/share%d.bmp", i + 1);
        write_bmp_257(carrier, output_filename);

        free(flat_pixels);
        free_bmp257_image(carrier);
    }

    // Free processed pixels
    for (int i = 0; i < total_pixels / k; i++) {
        free(processed_pixels[i]);
    }
    free(processed_pixels);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <file_manager.h>
#include <new_encryption.h>

// Helper function to generate a permutation sequence
static void generate_permutation(int* perm, int length, unsigned int seed) {
    for (int i = 0; i < length; i++) {
        perm[i] = i;
    }
    
    if (seed == 0) seed = time(NULL);
    srand(seed);
    
    for (int i = length - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = perm[i];
        perm[i] = perm[j];
        perm[j] = temp;
    }
}

// Shares a secret image into n shadow images (r required for reconstruction)
BMP257Image** share_secret_image(const BMP257Image* secret, int r, int n, unsigned int seed) {
    if (!secret || r < 2 || n < r) return NULL;
    
    int width = secret->info_header.width;
    int height = abs(secret->info_header.height);
    int total_pixels = width * height;
    
    // Allocate memory for shadow images
    BMP257Image** shadows = malloc(n * sizeof(BMP257Image*));
    if (!shadows) return NULL;
    
    // Initialize shadow images with 1/r size
    int shadow_width = width;
    int shadow_height = (height + r - 1) / r; // Round up
    
    for (int i = 0; i < n; i++) {
        shadows[i] = create_bmp_257(NULL, shadow_width, shadow_height);
        if (!shadows[i]) {
            // Cleanup on error
            for (int j = 0; j < i; j++) free_bmp257_image(shadows[j]);
            free(shadows);
            return NULL;
        }
        // Embed metadata in shadow images
        embed_seed(shadows[i], seed);
        embed_shadow_index(shadows[i], i+1); // Using 1-based index
    }
    
    // Generate permutation sequence
    int* perm = malloc(total_pixels * sizeof(int));
    if (!perm) {
        for (int i = 0; i < n; i++) free_bmp257_image(shadows[i]);
        free(shadows);
        return NULL;
    }
    generate_permutation(perm, total_pixels, seed);
    
    // Process each section of r pixels
    int sections = (total_pixels + r - 1) / r; // Round up
    int shadow_pixels = shadow_width * shadow_height;
    
    for (int s = 0; s < sections; s++) {
        // Get r pixels (pad with 0 if needed)
        uint16_t coefficients[r];
        int pixels_in_section = 0;
        
        for (int i = 0; i < r; i++) {
            int pixel_idx = s * r + i;
            if (pixel_idx < total_pixels) {
                // Get permuted pixel
                int orig_row = perm[pixel_idx] / width;
                int orig_col = perm[pixel_idx] % width;
                coefficients[i] = get_mod257_value(secret->pixels[orig_row][orig_col]);
                pixels_in_section++;
            } else {
                // Padding with 0 if needed
                coefficients[i] = 0;
            }
        }
        
        // Generate n shares for this section
        for (int share_num = 1; share_num <= n; share_num++) {
            // Evaluate polynomial q(x) = coefficients[0] + coefficients[1]*x + ... + coefficients[r-1]*x^(r-1)
            uint16_t share_value = 0;
            uint16_t x_power = 1; // x^0
            
            for (int i = 0; i < r; i++) {
                share_value = mod257_add(share_value, mod257_mul(coefficients[i], x_power));
                x_power = mod257_mul(x_power, share_num); // x^(i+1)
            }
            
            // Store share in appropriate shadow image
            int share_pixel_idx = s;
            if (share_pixel_idx < shadow_pixels) {
                int row = share_pixel_idx / shadow_width;
                int col = share_pixel_idx % shadow_width;
                shadows[share_num-1]->pixels[row][col] = value_to_mod257_pixel(share_value);
            }
        }
    }
    
    free(perm);
    return shadows;
}

// Reconstructs the secret image from r shadow images
BMP257Image* reconstruct_secret_image(BMP257Image** shadows, int r) {
    if (!shadows || r < 2) return NULL;
    
    // Verify all shadows have same dimensions and metadata
    int width = shadows[0]->info_header.width;
    int height = shadows[0]->info_header.height;
    unsigned int seed = shadows[0]->file_header.reserved1;
    
    for (int i = 1; i < r; i++) {
        if (shadows[i]->info_header.width != width || 
            shadows[i]->info_header.height != height ||
            shadows[i]->file_header.reserved1 != seed) {
            return NULL;
        }
    }
    
    int shadow_pixels = width * height;
    int secret_width = width;
    int secret_height = height * r; // Original size
    
    // Create secret image
    BMP257Image* secret = create_bmp_257(NULL, secret_width, secret_height);
    if (!secret) return NULL;
    
    // Generate permutation sequence (using seed from first shadow)
    int total_pixels = secret_width * secret_height;
    int* perm = malloc(total_pixels * sizeof(int));
    if (!perm) {
        free_bmp257_image(secret);
        return NULL;
    }
    generate_permutation(perm, total_pixels, seed);
    
    // Create inverse permutation
    int* inv_perm = malloc(total_pixels * sizeof(int));
    if (!inv_perm) {
        free(perm);
        free_bmp257_image(secret);
        return NULL;
    }
    
    for (int i = 0; i < total_pixels; i++) {
        inv_perm[perm[i]] = i;
    }
    
    // Process each section
    for (int s = 0; s < shadow_pixels; s++) {
        // Get r shares for this section
        uint16_t x_values[r];
        uint16_t y_values[r];
        
        for (int i = 0; i < r; i++) {
            int row = s / width;
            int col = s % width;
            x_values[i] = shadows[i]->file_header.reserved2; // Shadow index (1-based)
            y_values[i] = get_mod257_value(shadows[i]->pixels[row][col]);
        }
        
        // Reconstruct polynomial coefficients using Lagrange interpolation
        uint16_t coefficients[r];
        
        for (int k = 0; k < r; k++) {
            coefficients[k] = 0;
            
            for (int i = 0; i < r; i++) {
                uint16_t term = y_values[i];
                
                for (int j = 0; j < r; j++) {
                    if (i != j) {
                        uint16_t x_diff = mod257_sub(x_values[i], x_values[j]);
                        uint16_t x_kj = mod257_sub(k+1, x_values[j]); // k+1 because we need to evaluate at x=1,2,...,r
                        term = mod257_mul(term, x_kj);
                        term = mod257_mul(term, mod257_inv(x_diff));
                    }
                }
                
                coefficients[k] = mod257_add(coefficients[k], term);
            }
        }
        
        // Store reconstructed pixels (only up to original image size)
        for (int i = 0; i < r; i++) {
            int pixel_idx = s * r + i;
            if (pixel_idx < total_pixels) {
                int orig_pos = inv_perm[pixel_idx];
                int row = orig_pos / secret_width;
                int col = orig_pos % secret_width;
                secret->pixels[row][col] = value_to_mod257_pixel(coefficients[i]);
            }
        }
    }
    
    free(perm);
    free(inv_perm);
    return secret;
}

int test(){
 // Example usage:
BMP257Image* secret = read_bmp_257("assets/Alfredssd.bmp");

// Share the secret into 5 shadows, requiring 3 to reconstruct
BMP257Image** shadows = share_secret_image(secret, 3, 5, 12345);

// Write shadow images
for (int i = 0; i < 5; i++) {
    char filename[30];
    sprintf(filename, "encodings/shadow%d.bmp", i+1);
    write_bmp_257(shadows[i], filename);
}

// Reconstruct from any 3 shadows
BMP257Image* subset[3] = {shadows[0], shadows[2], shadows[4]};
BMP257Image* reconstructed = reconstruct_secret_image(subset, 3);
write_bmp_257(reconstructed,"encodings/recovered.bmp");

// Cleanup
free_bmp257_image(secret);
for (int i = 0; i < 5; i++) free_bmp257_image(shadows[i]);
free(shadows);

free_bmp257_image(reconstructed);

return 0;
}
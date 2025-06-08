#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <decryption.h>
#include <lagrange.h>
#include <encryption.h>


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
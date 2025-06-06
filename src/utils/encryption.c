
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
        cover_in_files_less(secret_image, cover_files, k, n);
    }else if( k == 8 ){
        cover_in_files(secret_image, cover_files, k, n);
    }else{
        cover_in_files_more(secret_image, cover_files, k, n);
    }

    // Free the secret image resources
    free_bmp257_image(secret_image);
    
    return 0; // Return 0 on success
}

void separate_pixels(Mod257Pixel** pixels, int k, Mod257Pixel** pixel_values) {
    printf("Separating pixels into %d shares\n", k);

    for ( int i=0, int n=0; i < k; i++ ) {
        
        memcpy(dest, src + start, num_elements * sizeof());
    }
}

void cover_in_files(BMP257Image* secret_image, const char** cover_files, int k, int n) {
    printf("Distributing into 8 cover files\n");
    Mod257Pixel** pixel_values = malloc(MAX_BYTES * sizeof(Mod257Pixel*)); // n array of pixel values
    separate_pixels(secret_image->pixels, k, n, pixel_values);
    Mod257Pixel* shares = malloc(n * sizeof(Mod257Pixel*));
    get_shares(pixel_values, k, n, shares); // This should be implemented to get the shares
    for (int i = 0; i < MAX_BYTES; i++) {
        BMP257Image* cover_image = read_bmp_257(cover_files[i]);
        if (!cover_image) {
            fprintf(stderr, "Error reading cover file '%s'\n", cover_files[i]);
            break;
        }

        write_with_shares(cover_image, shares[i], i); // Writes the transport images with the asigned shares

        free_bmp257_image(cover_image);
    }
    free(pixel_values);
    free(shares);
}

void separate_pixels(){
     = malloc(k * sizeof(Mod257Pixel*));
}

void get_shares(Mod257Pixel** pixel_values, int k, int n, Mod257Pixel* result){
        
}


// [] * n  --> 8 [] [] [] [] [] [] [] [] 8 bytes cada uno 
// 300x300/8 --> 37.5 x 300
// 300 x 300 bytes


int pow(int k, int i){
    int result = 1;
    for(int j = 0; j < i; j++){
        result *= k;
    }
    return result;
}

/*
    @param pixel_values: Array of Mod257Pixel pointers containing k pixel values
    @param k: Size of share array
    @param n: share operating
    @param result: Mod257Pixel structure to store the result of the evaluation
*/

void evaluate_shamir(Mod257Pixel* pixel_values, int k, int n, Mod257Pixel result){
    // f(k) = (pv[0] + pv[1] * n + pv[2] * n^2 + ... + pv[k-1] *n^(k-1)) mod 257
    // result = f(k);

    uint8_t aux = 0;
    
    for(int i = 0; i < k; i++){
        uint8_t term = (pixel_values[i]->value * pow(n,i))%PRIME;
        aux += (aux + term)%PRIME;
    } 

    result->value = aux;
    result->is_257 = false;
}

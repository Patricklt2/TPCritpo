
#include <encryption.h>

#define PRIME 257
#define MAX_BYTES 300*(300/8)
#define MAX_K 10

// ------------------ Private Declarations ------------------
void scramble_flattened_image(Mod257Pixel* image, int size);
void unscramble_flattened_image(Mod257Pixel* image, int size); 
void get_shares(Mod257Pixel* pixel_values, int k, int n, Mod257Pixel* result);
int pow_mod(int base, int exp, int mod);
void evaluate_shamir(Mod257Pixel* pixel_values, int k, int x, Mod257Pixel* result);
void process_image(BMP257Image * image, Mod257Pixel** pixels, int k, int n);
void unflatten_matrix(Mod257Pixel* flat, int height, int width, Mod257Pixel** matrix); 
void flatten_matrix(Mod257Pixel** matrix, int height, int width, Mod257Pixel* flat); 
// ---------------------------------------------------------

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

void separate_pixels(Mod257Pixel** pixels, int k, Mod257Pixel** pixel_values) {
    printf("Separating pixels into %d shares\n", k);

    for ( int i=0, n=0; i < k; i++ ) {
        
        //memcpy(dest, src + start, num_elements * sizeof());
    }
}

void cover_in_files(BMP257Image* secret_image, const char** cover_files, int k, int n) {
    printf("Distributing into 8 cover files\n");
    //TODO change to Height*width /k
    Mod257Pixel** processed_pixels = malloc(MAX_BYTES * sizeof(Mod257Pixel*)); // n array of pixel values
    process_image(secret_image, processed_pixels, k, n);

    for (int i = 0; i < MAX_BYTES; i++) {
        BMP257Image* cover_image = read_bmp_257(cover_files[i]);
        if (!cover_image) {
            fprintf(stderr, "Error reading cover file '%s'\n", cover_files[i]);
            break;
        }

//        write_with_shares(cover_image, shares[i], i); // Writes the transport images with the asigned shares

        free_bmp257_image(cover_image);
    }
}


//image to scramble
void scramble_flattened_image(Mod257Pixel* image, int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = nextChar() % (i + 1);

        Mod257Pixel temp = image[i];
        image[i] = image[j];
        image[j] = temp;
    }
}

// Unscramble image
void unscramble_flattened_image(Mod257Pixel* image, int size) {
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

// pixel values --> array aplanado de pixeles
// genera las n shares
void get_shares(Mod257Pixel* pixel_values, int k, int n, Mod257Pixel* result){
    for(int i = 1; i <= n; i++){
        evaluate_shamir(pixel_values, k, i, &result[i-1]);
    }
}



int pow_mod(int base, int exp, int mod) {
    int result = 1;
    for(int i = 0; i < exp; i++) {
        result = (result * base) % mod;
    }
    return result;
}

/*
    @param pixel_values: Array of Mod257Pixel pointers containing k pixel values
    @param k: Size of share array
    @param n: share operating
    @param result: Mod257Pixel structure to store the result of the evaluation
*/
void evaluate_shamir(Mod257Pixel* pixel_values, int k, int x, Mod257Pixel* result){
    // f(k) = (pv[0] + pv[1] * n + pv[2] * n^2 + ... + pv[k-1] *n^(k-1)) mod 257
    // result = f(k);

    uint16_t aux = 0;
    
    for(int i = 0; i < k; i++){
        uint16_t term = (pixel_values[i].value * pow_mod(x,i,PRIME))%PRIME;
        aux = (aux + term)%PRIME;
    } 

    result->value = (aux == 256) ? 0 : (uint8_t)aux;
    result->is_257 = (aux == 256) ? 1 : 0;
}

//this will scramble and process the shares
// [b0, b1, b2, b3, b4, .. bn]
// B --> b0 b1 b2 b3 b4 b5 b6 b7
// [0][B0 B1 B2 B3 B4 B5 .. Bn]
//  ...
// [11250][B0 B1 B2 B3 B4 .. Bn]
// ------------------------------------------ 
void process_image(BMP257Image *image, Mod257Pixel **processed_pixels, int k, int n) {
    int height = image->info_header.height;
    int width = image->info_header.width;
    int total_pixels = height * width;

    Mod257Pixel *flattened_pixels = malloc(sizeof(Mod257Pixel) * total_pixels);
    if (!flattened_pixels) return;

    flatten_matrix(image->pixels, height, width, flattened_pixels);
    scramble_flattened_image(flattened_pixels, total_pixels);

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
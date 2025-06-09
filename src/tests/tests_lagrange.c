#include <tests_lagrange.h>


void test_flatten_unflatten() {
    int height = 3, width = 4;

    // Allocate original matrix
    Mod257Pixel** original = malloc(height * sizeof(Mod257Pixel*));
    for (int i = 0; i < height; i++)
        original[i] = malloc(width * sizeof(Mod257Pixel));

    // Fill with test values
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            original[i][j].value = i * width + j;
            original[i][j].is_257 = (original[i][j].value == 0) ? 1 : 0;
        }

    // Flatten
    Mod257Pixel* flat = malloc(height * width * sizeof(Mod257Pixel));
    flatten_matrix(original, height, width, flat);

    // Allocate matrix to unflatten into
    Mod257Pixel** result = malloc(height * sizeof(Mod257Pixel*));
    for (int i = 0; i < height; i++)
        result[i] = malloc(width * sizeof(Mod257Pixel));

    // Unflatten
    unflatten_matrix(flat, height, width, result);

    // Verify
    int success = 1;
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            if (original[i][j].value != result[i][j].value || original[i][j].is_257 != result[i][j].is_257) {
                success = 0;
                printf("Mismatch at (%d, %d): original=(%d, %d), result=(%d, %d)\n",
                       i, j,
                       original[i][j].value, original[i][j].is_257,
                       result[i][j].value, result[i][j].is_257);
            }

    if (success)
        printf("✅ Flatten/unflatten test passed!\n");
    else
        printf("❌ Flatten/unflatten test failed.\n");

    // Cleanup
    for (int i = 0; i < height; i++) {
        free(original[i]);
        free(result[i]);
    }
    free(original);
    free(result);
    free(flat);
}

// === Test utility ===
void print_coeffs(const char* label, Mod257Pixel* coeffs, int k) {
    printf("%s: ", label);
    for (int i = 0; i < k; i++) {
        if (coeffs[i].is_257)
            printf("[256] ");
        else
            printf("[%d] ", coeffs[i].value);
    }
    printf("\n");
}

int compare_polys(Mod257Pixel* a, Mod257Pixel* b, int k) {
    for (int i = 0; i < k; i++) {
        if (a[i].value != b[i].value || a[i].is_257 != b[i].is_257)
            return 0;
    }
    return 1;
}

void test_cover_and_recover() {
    const int k = 9;
    const int n = 9;
    const uint16_t seed = 1000;
    const char* cover_files[] = {
        "assets/Alfredssd.bmp", "assets/Albertssd.bmp", "assets/Audreyssd.bmp",
        "assets/Evassd.bmp", "assets/Facundo.bmp", "assets/Gustavossd.bmp",
        "assets/Jamesssd.bmp", "assets/Marilynssd.bmp", 
        "assets/Jamesssd.bmp", "assets/Marilynssd.bmp"  // extras
    };

    BMP257Image* original_secret = read_bmp_257("assets/Alfredssd.bmp");

    // Cover the secret into shares
    cover_in_files_v2(original_secret, cover_files, k, n, seed);

    // Recover using a subset of k shares
    const char* subset[10] = {
        "encodings/share1.bmp", "encodings/share2.bmp", "encodings/share3.bmp",
        "encodings/share4.bmp", "encodings/share5.bmp", "encodings/share6.bmp",
        "encodings/share7.bmp", "encodings/share8.bmp", "encodings/share9.bmp",
        NULL
    };
    const char* recovered_file = "encodings/hola2.bmp";
    recover_from_files_v2(k, n, subset, recovered_file);

    // Load recovered image
    BMP257Image* recovered_image = read_bmp_257(recovered_file);
    if (!recovered_image) {
        fprintf(stderr, "Failed to load recovered image.\n");
        free_bmp257_image(original_secret);
        return;
    }

    // Compare pixel-by-pixel
    int differing_pixels = 0;
    int height = original_secret->info_header.height;
    int width = original_secret->info_header.width;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            Mod257Pixel p1 = original_secret->pixels[i][j];
            Mod257Pixel p2 = recovered_image->pixels[i][j];

            if(p1.value != p2.value)
                differing_pixels++;
        }
    }

    printf("Differing pixels: %d (out of %d)\n", differing_pixels, width * height);

    // Cleanup
    free_bmp257_image(original_secret);
    free_bmp257_image(recovered_image);
}

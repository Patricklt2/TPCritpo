#include <tests_lagrange.h>

// Test cases for lagrange_interpolate function
void lagrangeTests() {
    int x1[] = {1, 2};          // Shadow numbers
    int y1[] = {75, 198};       // Shadow values
    int secret1 = lagrange_interpolate(x1, y1, 2);
    printf("Test 1: Secret = %d (Expected: 209) => ", secret1);
    assert(secret1 == 209);

    int x2[] = {1, 2, 3};        // Shadow numbers
    int y2[] = {75, 198, 321};   // Shadow values
    int secret2 = lagrange_interpolate(x2, y2, 3);
    printf("OK!\nTest 2: Secret = %d (Expected: 209) => ", secret2);
    assert(secret2 == 209); // Should be the same secret as in Test 1

    int x3[] = {1, 2};
    int y3[] = {8, 11};
    int secret3 = lagrange_interpolate(x3, y3, 2);
    printf("OK!\nTest 3: Secret = %d (Expected: 5) => ", secret3);
    assert(secret3 == 5);

    int x4[] = {1, 2, 3};
    int y4[] = {6, 11, 18};
    int secret4 = lagrange_interpolate(x4, y4, 3);
    printf("OK!\nTest 4: Secret = %d (Expected: 3) => ", secret4);
    assert(secret4 == 3);

    int x5[] = {5, 10, 15};
    int y5[] = {42, 42, 42};
    int secret5 = lagrange_interpolate(x5, y5, 3);
    printf("OK!\nTest 5: Secret = %d (Expected: 42) => ", secret5);
    assert(secret5 == 42);

    int x6[] = {255, 256};
    int y6[] = {(255 * 5 + 7) % MOD, (256 * 5 + 7) % MOD};
    int secret6 = lagrange_interpolate(x6, y6, 2);
    printf("OK!\nTest 6: Secret = %d (Expected: 7) => ", secret6);
    assert(secret6 == 7);

    int x8[] = {0, 1};
    int y8[] = {256, 1}; 
    int secret8 = lagrange_interpolate(x8, y8, 2);
    printf("OK!\nTest 7: Secret = %d (Expected: 256) => ", secret8);
    assert(secret8 == 256);

    int x9[] = {1, 2, 3};
    int y9[] = {0, 0, 0};
    int secret9 = lagrange_interpolate(x9, y9, 3);
    printf("OK!\nTest 8: Secret = %d (Expected: 0) => ", secret9);
    assert(secret9 == 0);

    int x10[] = {0, 256};
    int y10[] = {100, 200};
    int secret10 = lagrange_interpolate(x10, y10, 2);
    printf("OK!\nTest 9: Secret = %d (Expected: (100*256 + 200*0)*mod_inverse(256) %% 257) => ", secret10);
    int expected10 = (100 * 256 % MOD * mod_inverse((256 - 0 + MOD) % MOD)) % MOD;
    assert(secret10 == expected10);

    printf("OK\nAll lagrange tests passed!\n");
}

// Test cases for mod_inverse function
void inverseModTests() {

    printf("Test 1: ");
    assert(mod_inverse(1) == 1);

    printf("OK!\nTest 2: ");
    assert(mod_inverse(2) == 129);

    printf("OK!\nTest 3: ");
    assert(mod_inverse(3) == 86);

    printf("OK!\nTest 4: ");
    assert(mod_inverse(0) == -1);

    printf("OK!\nTest 5: ");
    assert(mod_inverse(256) == 256);

    printf("OK!\nAll inverse modulo tests passed!\n");
}

void test_scramble_unscramble() {
    const int size = 100;
    Mod257Pixel* original = malloc(size * sizeof(Mod257Pixel));
    Mod257Pixel* backup = malloc(size * sizeof(Mod257Pixel));

    // Fill original image with values 0 to 99
    for (int i = 0; i < size; i++) {
        original[i].value = i % 256;
        original[i].is_257 = 0;
        backup[i] = original[i];  // copy
    }

    int64_t test_seed = 987654321;

    // Scramble
    setSeed(test_seed);
    scramble_flattened_image(original, size);

    // Unscramble
    setSeed(test_seed);  // reset seed!
    unscramble_flattened_image(original, size);

    // Compare
    int match = 1;
    for (int i = 0; i < size; i++) {
        if (original[i].value != backup[i].value || original[i].is_257 != backup[i].is_257) {
            printf("Mismatch at index %d: got %d, expected %d\n",
                   i, original[i].value, backup[i].value);
            match = 0;
            break;
        }
    }

    if (match)
        printf("✅ Scramble/unscramble test passed!\n");
    else
        printf("❌ Scramble/unscramble test failed.\n");

    free(original);
    free(backup);
}

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

// === Main Test ===
void test_lagrange_recovery() {
    srand(time(NULL));
    for (int test = 0; test < 5; test++) {
        int k = 5 + rand() % 4;  // Degree + 1

        // Random coefficients
        Mod257Pixel coeffs[MAX_K];
        for (int i = 0; i < k; i++) {
            int val = rand() % 257;
            coeffs[i].value = (val == 256) ? 0 : val;
            coeffs[i].is_257 = (val == 256) ? 1 : 0;
        }

        // Generate shares
        int x_coords[MAX_K];
        Mod257Pixel shares[MAX_K];
        for (int i = 0; i < k; i++) {
            x_coords[i] = i + 1; // x = 1, 2, 3, ...
            evaluate_shamir(coeffs, k, x_coords[i], &shares[i]);
        }

        // Recover
        Mod257Pixel recovered[MAX_K];
        recover_polynomial(x_coords, shares, k, recovered);

        // Output
        printf("Test %d\n", test + 1);
        print_coeffs("Original", coeffs, k);
        print_coeffs("Recovered", recovered, k);
        printf(compare_polys(coeffs, recovered, k) ? "✅ Success\n\n" : "❌ Failure\n\n");
    }
}
#include <tests_lagrange.h>

// Test cases for lagrange_interpolate function
int lagrangeTests() {
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
    return 0;
}

// Test cases for mod_inverse function
int inverseModTests() {

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
    return 0;
}
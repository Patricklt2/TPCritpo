#include "../utils/lagrange.h"
#include <assert.h>

int main() {
    // Test Case 1: Basic (k=2)
    int x1[] = {1, 2};          // Shadow numbers
    int y1[] = {75, 198};       // Shadow values (from f(x) = 209 + 123x)
    int secret1 = lagrange_interpolate(x1, y1, 2);
    printf("Test 1: Secret = %d (Expected: 209)\n", secret1);
    assert(secret1 == 209);

    // Test Case 2: Invalid inverse
    int x2[] = {1, 2};
    int y2[] = {10, 20};
    printf("Testing mod_inverse(0)...\n");
    assert(mod_inverse(0) == -1);  // 0 has no inverse

    printf("All tests passed!\n");
    return 0;
}
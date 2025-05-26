#include <tests_file_manager.h>
#include <tests_lagrange.h>

int main() {
    printf("Running Lagrange Tests...\n");
    lagrangeTests();

    printf("Running Inverse Modulo Tests...\n");
    inverseModTests();

    printf("Running File Manager Tests...\n");
    fileManagerTests();

    printf("All tests completed successfully!\n");
    return 0;
}
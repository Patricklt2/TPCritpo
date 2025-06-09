#include <tests_file_manager.h>
#include <tests_enc_dec.h>

int main() {
    printf("Running File Manager Tests...\n");
    fileManagerTests();

    test_flatten_unflatten();
    test_cover_and_recover();
    printf("All tests completed successfully!\n");
    return 0;
}
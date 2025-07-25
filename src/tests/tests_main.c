#include <tests_file_manager.h>
#include <tests_enc_dec.h>

int main() {
    printf("Running File Manager Tests...\n");
    fileManagerTests();

    test_flatten_unflatten();



    test_cover_and_recover_k(2, 2, "assets/Alfredssd.bmp", "encodings/test1.bmp");
    test_cover_and_recover_k(3, 3, "assets/Alfredssd.bmp", "encodings/test2.bmp");
    test_cover_and_recover_k(4, 4, "assets/Alfredssd.bmp", "encodings/test3.bmp");
    test_cover_and_recover_k(5, 5, "assets/Alfredssd.bmp", "encodings/test4.bmp");
    test_cover_and_recover_k(6, 6, "assets/Alfredssd.bmp", "encodings/test5.bmp");
    test_cover_and_recover_k(7, 7, "assets/Alfredssd.bmp", "encodings/test6.bmp");
    test_cover_and_recover_k(8, 8, "assets/Alfredssd.bmp", "encodings/test7.bmp");
    test_cover_and_recover_k(9, 9, "assets/Alfredssd.bmp", "encodings/test8.bmp");
    test_cover_and_recover_k(10,10,"assets/Alfredssd.bmp", "encodings/test9.bmp");


    printf("All tests completed successfully!\n");
    return 0;
}
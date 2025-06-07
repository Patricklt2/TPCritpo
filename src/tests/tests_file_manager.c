#include <tests_file_manager.h>
#include <string.h>

long get_file_size(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    return size;
}

//---------------------------------Begin Tests---------------------------------


void fileManagerTests() {
    test_create_valid_2x2();
    test_create_with_257_pixel();
    //test_write_valid_bmp();
    test_write_null_image();
    test_write_null_filename();
    //test_bmp_signature();
    //test_read_valid_bmp();
    test_read_nonexistent_file();
    test_read_invalid_signature();
    test_read_non_8bit_bmp();
    test_read_palette_not_grayscale();
    test_read_non_8bit_bmp();

    printf("All file manager tests passed!\n");
    return 0;
}

void test_create_valid_2x2() {

    int width = 2, height = 2;
    Mod257Pixel** pixels = malloc(height * sizeof(Mod257Pixel*));
    for (int i = 0; i < height; i++) {
        pixels[i] = malloc(width * sizeof(Mod257Pixel));
        for (int j = 0; j < width; j++) {
            pixels[i][j].value = i * width + j;
            pixels[i][j].is_257 = 0;
        }
    }

    BMP257Image* img = create_bmp_257(pixels, width, height);
    assert(img != NULL);
    assert(img->info_header.width == width);
    assert(img->info_header.height == height);
    assert(img->info_header.bits_per_pixel == 8);
    assert(img->palette != NULL);
    for (int i = 0; i < 256; i++) {
        assert(img->palette[i].red == i);
        assert(img->palette[i].green == i);
        assert(img->palette[i].blue == i);
    }

    free_bmp257_image(img);
    printf("Test 1: OK!\n");
}

void test_create_invalid_dimensions() {
    Mod257Pixel** dummy = NULL;

    BMP257Image* img = create_bmp_257(dummy, 0, 10);
    assert(img == NULL);

    img = create_bmp_257(dummy, 10, -1);
    assert(img == NULL);

    printf("Test 2: OK!\n");
}

void test_create_with_257_pixel() {
    int width = 1, height = 1;
    Mod257Pixel** pixels = malloc(sizeof(Mod257Pixel*));
    pixels[0] = malloc(sizeof(Mod257Pixel));
    pixels[0][0].value = 0;      // Could be interpreted as special if read back
    pixels[0][0].is_257 = 1;

    BMP257Image* img = create_bmp_257(pixels, width, height);
    assert(img != NULL);
    assert(img->info_header.width == 1);
    assert(img->palette[0].red == 0);
    assert(img->palette[0].green == 0);
    assert(img->palette[0].blue == 0);

    free_bmp257_image(img);
    printf("Test 3: OK!\n");
}

void test_write_valid_bmp() {
    const char* filename = "test_2x2.bmp";

    int width = 2, height = 2;
    Mod257Pixel** pixels = malloc(height * sizeof(Mod257Pixel*));
    for (int i = 0; i < height; i++) {
        pixels[i] = malloc(width * sizeof(Mod257Pixel));
        for (int j = 0; j < width; j++) {
            pixels[i][j].value = i * width + j;
            pixels[i][j].is_257 = 0;
        }
    }

    BMP257Image* image = create_bmp_257(pixels, width, height);
    assert(image != NULL);

    int result = write_bmp_257(image, filename);
    assert(result == 0);

    long file_size = get_file_size(filename);
    assert(file_size > 0);

    // Expected size: headers + palette + padded pixel data
    int row_padded = ((width + 3) / 4) * 4;
    long expected = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader)
                  + 256 * sizeof(RGBQuad) + row_padded * height;
    assert(file_size == expected);

    printf("Test 4: OK!\n");

    free_bmp257_image(image);
    remove(filename);
}

void test_write_null_image() {
    int result = write_bmp_257(NULL, "dummy.bmp");
    assert(result == -1);
    printf("Test 5: OK!\n");
}

void test_write_null_filename() {
    int width = 1, height = 1;
    Mod257Pixel** pixels = malloc(sizeof(Mod257Pixel*));
    pixels[0] = malloc(sizeof(Mod257Pixel));
    pixels[0][0].value = 0;
    pixels[0][0].is_257 = 0;

    BMP257Image* image = create_bmp_257(pixels, width, height);
    assert(image != NULL);

    int result = write_bmp_257(image, NULL);
    assert(result == -1);
    printf("Test 6: OK!\n");

    free_bmp257_image(image);
}

void test_bmp_signature() {
    const char* filename = "test_signature.bmp";

    int width = 1, height = 1;
    Mod257Pixel** pixels = malloc(sizeof(Mod257Pixel*));
    pixels[0] = malloc(sizeof(Mod257Pixel));
    pixels[0][0].value = 42;
    pixels[0][0].is_257 = 0;

    BMP257Image* image = create_bmp_257(pixels, width, height);
    assert(image != NULL);

    int result = write_bmp_257(image, filename);
    assert(result == 0);

    FILE* f = fopen(filename, "rb");
    assert(f != NULL);

    uint8_t sig[2];
    fread(sig, 1, 2, f);
    fclose(f);

    assert(sig[0] == 'B' && sig[1] == 'M');
    printf("Test 7: 0K!\n");

    free_bmp257_image(image);
    remove(filename);
}

void test_read_valid_bmp() {
    const char* filename = "src/assets/Alfred.bmp";

    BMP257Image* img = read_bmp_257(filename);
    assert(img != NULL);
    assert(img->info_header.width == 300);
    assert(abs(img->info_header.height) == 300);

    // Check palette is grayscale
    for (int i = 0; i < 256; i++) {
        assert(img->palette[i].red == i);
        assert(img->palette[i].green == i);
        assert(img->palette[i].blue == i);
    }

    free_bmp257_image(img);
    remove(filename);
    printf("Test 8: 0K!\n");
}

void test_read_nonexistent_file() {
    BMP257Image* img = read_bmp_257("nonexistent.bmp");
    assert(img == NULL);

    printf("Test 9: 0K!\n");
}

void test_read_invalid_signature() {
    // Create a file with bad signature, e.g. write 2 bytes != 0x4D42 at start
    FILE* f = fopen("bad_signature.bmp", "wb");
    uint16_t bad_sig = 0x1234;
    fwrite(&bad_sig, sizeof(bad_sig), 1, f);
    fclose(f);

    BMP257Image* img = read_bmp_257("bad_signature.bmp");
    assert(img == NULL);

    remove("bad_signature.bmp");

    printf("Test 10: 0K!\n");
}

void test_read_non_8bit_bmp() {
    FILE* f = fopen("non8bit.bmp", "wb");
    BMPFileHeader fh = {0x4D42, 0, 0, 0, 54 + 256*4 + 4}; // dummy sizes
    BMPInfoHeader ih = {40, 2, 2, 1, 24, 0, 0, 2835, 2835, 256, 256}; // 24bpp here
    fwrite(&fh, sizeof(fh), 1, f);
    fwrite(&ih, sizeof(ih), 1, f);

    // Write dummy palette (still 256 colors, even if unused for 24bpp)
    RGBQuad pal[256] = {0};
    for (int i=0; i<256; i++) {
        pal[i].red = pal[i].green = pal[i].blue = (uint8_t)i;
    }
    fwrite(pal, sizeof(RGBQuad), 256, f);

    // Write dummy pixel data
    uint8_t data[4] = {0,0,0,0};
    fwrite(data, sizeof(data), 1, f);
    fclose(f);

    BMP257Image* img = read_bmp_257("non8bit.bmp");
    assert(img == NULL);

    remove("non8bit.bmp");

    printf("Test 11: 0K!\n");
}

void test_read_palette_not_grayscale() {
    FILE* f = fopen("bad_palette.bmp", "wb");
    BMPFileHeader fh = {0x4D42, 0, 0, 0, 54 + 256*4 + 4};
    BMPInfoHeader ih = {40, 2, 2, 1, 8, 0, 4, 2835, 2835, 256, 256};
    fwrite(&fh, sizeof(fh), 1, f);
    fwrite(&ih, sizeof(ih), 1, f);

    RGBQuad pal[256];
    for (int i=0; i<256; i++) {
        pal[i].red = pal[i].green = pal[i].blue = (uint8_t)i;
        pal[i].reserved = 0;
    }
    pal[100].green = 0;  // Break grayscale condition for palette entry 100
    fwrite(pal, sizeof(RGBQuad), 256, f);

    // Write dummy pixel data
    uint8_t data[8] = {0};
    fwrite(data, 1, 8, f);
    fclose(f);

    BMP257Image* img = read_bmp_257("bad_palette.bmp");
    assert(img == NULL);

    remove("bad_palette.bmp");
    printf("Test 12: 0K!\n");
}
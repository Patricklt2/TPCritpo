#include "headers/file_manager.h"


BMP257Image* read_bmp_257(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    // Allocate memory for image structure
    BMP257Image* image = malloc(sizeof(BMP257Image));
    if (!image) {
        fclose(file);
        perror("Memory allocation failed");
        return NULL;
    }

    // Read BMP headers
    if (fread(&image->file_header, sizeof(BMPFileHeader), 1, file) != 1 ||
        fread(&image->info_header, sizeof(BMPInfoHeader), 1, file) != 1) {
        fclose(file);
        free(image);
        perror("Error reading headers");
        return NULL;
    }

    // Verify BMP format
    if (image->file_header.signature != 0x4D42 || 
        image->info_header.bits_per_pixel != 8) {
        fclose(file);
        free(image);
        fprintf(stderr, "Only 8-bit grayscale BMP files are supported\n");
        return NULL;
    }

    // Read color palette (256 entries)
    image->palette = malloc(256 * sizeof(RGBQuad));
    if (!image->palette) {
        fclose(file);
        free(image);
        perror("Palette allocation failed");
        return NULL;
    }

    if (fread(image->palette, sizeof(RGBQuad), 256, file) != 256) {
        fclose(file);
        free(image->palette);
        free(image);
        perror("Error reading palette");
        return NULL;
    }

    // Verify grayscale palette (R=G=B)
    for (int i = 0; i < 256; i++) {
        if (image->palette[i].red != image->palette[i].green ||
            image->palette[i].red != image->palette[i].blue) {
            fclose(file);
            free(image->palette);
            free(image);
            fprintf(stderr, "Palette is not grayscale\n");
            return NULL;
        }
    }

    // Calculate image dimensions
    int width = image->info_header.width;
    int height = abs(image->info_header.height);
    int row_padded = ((width + 3) / 4) * 4; // Row size with padding

    // Allocate pixel matrix
    image->pixels = malloc(height * sizeof(Mod257Pixel*));
    if (!image->pixels) {
        fclose(file);
        free(image->palette);
        free(image);
        perror("Pixel matrix allocation failed");
        return NULL;
    }

    for (int i = 0; i < height; i++) {
        image->pixels[i] = malloc(width * sizeof(Mod257Pixel));
        if (!image->pixels[i]) {
            // Cleanup already allocated rows
            for (int j = 0; j < i; j++) free(image->pixels[j]);
            free(image->pixels);
            fclose(file);
            free(image->palette);
            free(image);
            perror("Row allocation failed");
            return NULL;
        }
    }

    // Read pixel data
    uint8_t* row_buffer = malloc(row_padded);
    if (!row_buffer) {
        for (int i = 0; i < height; i++) free(image->pixels[i]);
        free(image->pixels);
        fclose(file);
        free(image->palette);
        free(image);
        perror("Row buffer allocation failed");
        return NULL;
    }

    // Move to pixel data start
    fseek(file, image->file_header.pixel_offset, SEEK_SET);

    // Read rows (BMP stores bottom row first)
    for (int y = height - 1; y >= 0; y--) {
        if (fread(row_buffer, 1, row_padded, file) != row_padded) {
            free(row_buffer);
            for (int i = 0; i < height; i++) free(image->pixels[i]);
            free(image->pixels);
            fclose(file);
            free(image->palette);
            free(image);
            perror("Error reading pixel data");
            return NULL;
        }

        // Process each pixel in row
        for (int x = 0; x < width; x++) {
            // Check for special 256 value (marked by palette)
            if (row_buffer[x] == 0 && 
                image->palette[0].red == 0x00 &&
                image->palette[0].green == 0x00 &&
                image->palette[0].blue == 0x01) {
                image->pixels[y][x].value = 0;
                image->pixels[y][x].is_257 = 1;
            } else {
                image->pixels[y][x].value = row_buffer[x];
                image->pixels[y][x].is_257 = 0;
            }
        }
    }

    free(row_buffer);
    fclose(file);
    return image;
}

uint16_t get_mod257_value(Mod257Pixel pixel) {
    return pixel.is_257 ? 256 : pixel.value;
}

Mod257Pixel value_to_mod257_pixel(uint16_t value) {
    Mod257Pixel pixel;
    value %= 257;
    if (value == 256) {
        pixel.value = 0;
        pixel.is_257 = 1;
    } else {
        pixel.value = (uint8_t)value;
        pixel.is_257 = 0;
    }
    return pixel;
}

void free_bmp257_image(BMP257Image* image) {
    if (image) {
        if (image->pixels) {
            int height = abs(image->info_header.height);
            for (int i = 0; i < height; i++) {
                free(image->pixels[i]);
            }
            free(image->pixels);
        }
        free(image->palette);
        free(image);
    }
}


uint16_t mod257_add(uint16_t a, uint16_t b) {
    uint16_t sum = a + b;
    return sum >= 257 ? sum - 257 : sum;
}

uint16_t mod257_sub(uint16_t a, uint16_t b) {
    return a >= b ? a - b : 257 - (b - a);
}

uint16_t mod257_mul(uint16_t a, uint16_t b) {
    uint16_t res = 0;
    a %= 257;
    b %= 257;
    
    while (b > 0) {
        if (b & 1) 
            res = mod257_add(res, a);
        a = mod257_add(a, a);
        b >>= 1;
    }
    return res;
}

uint16_t mod257_inv(uint16_t a) {
    if (a == 0) return 0;  // No inverse for 0
    if (a == 1) return 1;  // Inverse of 1 is 1
    
    int16_t old_t = 0, new_t = 1;
    int16_t old_r = 257, new_r = a;
    
    while (new_r != 0) {
        int16_t quotient = old_r / new_r;
        int16_t temp = old_t - quotient * new_t;
        old_t = new_t;
        new_t = temp;
        temp = old_r - quotient * new_r;
        old_r = new_r;
        new_r = temp;
    }
    
    if (old_r != 1) return 0; // No inverse exists
    return old_t < 0 ? old_t + 257 : old_t;
}

// TODO: check this function, coz the construction of the header

BMP257Image* create_bmp_257(Mod257Pixel** pixels, int width, int height) {
    // Validate dimensions
    if (width <= 0 || height <= 0) {
        fprintf(stderr, "Invalid image dimensions\n");
        return NULL;
    }

    // Allocate image structure
    BMP257Image* image = malloc(sizeof(BMP257Image));
    if (!image) {
        perror("Memory allocation failed");
        return NULL;
    }

    // Calculate padding and sizes
    int row_padded = ((width + 3) / 4) * 4;
    int image_size = row_padded * height;
    int palette_size = 256 * sizeof(RGBQuad);
    
    // Initialize file header
    image->file_header.signature = 0x4D42; // 'BM'
    image->file_header.file_size = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + palette_size + image_size;
    image->file_header.reserved1 = 0;
    image->file_header.reserved2 = 0;
    image->file_header.pixel_offset = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + palette_size;

    // Initialize info header
    image->info_header.header_size = sizeof(BMPInfoHeader);
    image->info_header.width = width;
    image->info_header.height = height;
    image->info_header.planes = 1;
    image->info_header.bits_per_pixel = 8;
    image->info_header.compression = 0;
    image->info_header.image_size = image_size;
    image->info_header.x_resolution = 2835; // 72 DPI × 39.3701 inches/meter
    image->info_header.y_resolution = 2835;
    image->info_header.colors_used = 256;
    image->info_header.colors_important = 256;

    // Create grayscale palette
    image->palette = malloc(palette_size);
    if (!image->palette) {
        free(image);
        perror("Palette allocation failed");
        return NULL;
    }

    for (int i = 0; i < 256; i++) {
        image->palette[i].red = i;
        image->palette[i].green = i;
        image->palette[i].blue = i;
        image->palette[i].reserved = 0;
    }

    image->pixels = pixels; // Copy pixel data

    return image;
}
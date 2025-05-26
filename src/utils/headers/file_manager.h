#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Disable struct padding for exact binary matching
#pragma pack(push, 1)

// BMP File Header (14 bytes)
typedef struct {
    uint16_t signature;      // 'BM' identifier (0x4D42)
    uint32_t file_size;      // Total file size in bytes
    uint16_t reserved1;      // Reserved space (unused)
    uint16_t reserved2;      // Reserved space (unused)
    uint32_t pixel_offset;   // Offset to pixel data
} BMPFileHeader;

// BMP Info Header (40 bytes)
typedef struct {
    uint32_t header_size;    // Size of this struct (40 bytes)
    int32_t width;           // Image width in pixels
    int32_t height;          // Image height in pixels
    uint16_t planes;         // Color planes (must be 1)
    uint16_t bits_per_pixel; // Bits per pixel (8 for grayscale)
    uint32_t compression;    // Compression type (0 = uncompressed)
    uint32_t image_size;     // Size of raw pixel data
    int32_t x_resolution;    // Horizontal resolution (pixels/meter)
    int32_t y_resolution;    // Vertical resolution (pixels/meter)
    uint32_t colors_used;    // Number of palette colors used
    uint32_t colors_important; // Number of important colors
} BMPInfoHeader;

// Color palette entry (4 bytes per color)
typedef struct {
    uint8_t blue;           // Blue component
    uint8_t green;          // Green component
    uint8_t red;            // Red component
    uint8_t reserved;       // Reserved (must be 0)
} RGBQuad;

#pragma pack(pop)  // Restore default struct packing

// Special pixel structure for modulo 257 values
typedef struct {
    uint8_t value;          // Actual pixel value (0-255)
    uint8_t is_257;         // Flag for 256 value (1 = true, 0 = false)
} Mod257Pixel;

// Complete image structure with modulo 257 support
typedef struct {
    BMPFileHeader file_header;
    BMPInfoHeader info_header;
    RGBQuad* palette;       // Color palette (256 entries)
    Mod257Pixel** pixels;   // 2D pixel array [height][width]
} BMP257Image;


// Function prototypes

/**
 * Reads an 8-bit grayscale BMP file with modulo 257 support
 * @param filename Path to the BMP file
 * @return Pointer to BMP257Image structure, NULL on error
 */
BMP257Image* read_bmp_257(const char* filename);

/**
 * Gets the actual modulo 257 value from a pixel
 * @param pixel The Mod257Pixel structure
 * @return Value in 0-256 range
 */
uint16_t get_mod257_value(Mod257Pixel pixel);

/**
 * Converts a modulo 257 value to pixel structure
 * @param value Value in 0-256 range
 * @return Mod257Pixel structure
 */
Mod257Pixel value_to_mod257_pixel(uint16_t value);

/**
 * Frees all resources allocated for a BMP257Image
 * @param image Pointer to the image structure
 */
void free_bmp257_image(BMP257Image* image);

/**
 * Performs addition modulo 257
 * @param a First operand (0-256)
 * @param b Second operand (0-256)
 * @return Result of (a + b) mod 257
 */
uint16_t mod257_add(uint16_t a, uint16_t b);

/**
 * Performs subtraction modulo 257
 * @param a First operand (0-256)
 * @param b Second operand (0-256)
 * @return Result of (a - b) mod 257
 */
uint16_t mod257_sub(uint16_t a, uint16_t b);

/**
 * Performs multiplication modulo 257 using Russian Peasant algorithm
 * @param a First operand (0-256)
 * @param b Second operand (0-256)
 * @return Result of (a * b) mod 257
 */
uint16_t mod257_mul(uint16_t a, uint16_t b);

/**
 * Computes modular inverse using Extended Euclidean Algorithm
 * @param a Number to invert (0-256)
 * @return Inverse of a mod 257, or 0 if non-invertible
 */
uint16_t mod257_inv(uint16_t a);

/**
 * Creates a new 8-bit grayscale BMP image with modulo 257 support
 * @param pixels 2D array of Mod257Pixel pointers representing pixel data
 * @param width Image width in pixels
 * @param height Image height in pixels
 * @return Pointer to new BMP257Image, or NULL on error
 */
BMP257Image* create_bmp_257(Mod257Pixel** pixels, int width, int height);

/**
 * Converts a BMP257Image to a file
 * @param image Pointer to the BMP257Image structure
 * @param filename Path to save the BMP file
 * @return 0 on success, -1 on error
 */
int write_bmp_257(const BMP257Image* image, const char* filename);

#endif
#ifndef NEW_ENCRYPTION_H
#define NEW_ENCRYPTION_H

#include <file_manager.h>

// Helper function to generate a permutation sequence
static void generate_permutation(int* perm, int length, unsigned int seed);

// Shares a secret image into n shadow images (r required for reconstruction)
BMP257Image** share_secret_image(const BMP257Image* secret, int r, int n, unsigned int seed);
// Reconstructs the secret image from r shadow images
BMP257Image* reconstruct_secret_image(BMP257Image** shadows, int r);

int test();

#endif
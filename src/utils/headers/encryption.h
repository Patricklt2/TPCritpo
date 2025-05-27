#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include "file_manager.h"
#include <time.h>

/**
 * Distributes a secret image into multiple shares using Shamir's Secret Sharing
 * @param shades_count Number of shares to create (k)
 * @param file_name Name of the secret image file to distribute
 * @param images_count Total number of images to create (n)
 * @param cover_files Array of file names for the output shares
 * @return 0 on success, -1 on error
 */

int shamir_distribute( int shades_count, const char* file_name,  int images_count, const char** cover_files);

#endif
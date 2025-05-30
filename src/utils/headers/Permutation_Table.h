
#ifndef PERMUTATION_TABLE_H
#define PERMUTATION_TABLE_H
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>

void setSeed(int64_t seed);
uint8_t nextChar(void); /* devuelve un unsigned char */

/**
 * Generates a permutation table of size `size` using the provided seed.
 * @param R Pointer to an array where the permutation table will be stored.
 * @param size The size of the permutation table to generate.
 * @param seed The seed value used to generate the permutation.
 */
void generate_permutation_table(uint16_t* R, int size, uint16_t seed);
#endif




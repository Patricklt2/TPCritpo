#ifndef TESTS_ENC_DEC_H
#define TESTS_ENC_DEC_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <encryption.h>
#include <decryption.h>

void test_flatten_unflatten(); 

void print_coeffs(const char* label, Mod257Pixel* coeffs, int k);
int compare_polys(Mod257Pixel* a, Mod257Pixel* b, int k);
void test_cover_and_recover_k(const int k, const int n, const char * file_path, const char * out_path); 
#endif
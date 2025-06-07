#ifndef TESTS_LAGRANGE_H
#define TESTS_LAGRANGE_H

#include <stdio.h>
#include <stdlib.h>
#include <lagrange.h>
#include <assert.h>
#include <encryption.h>

void lagrangeTests();
void inverseModTests();
void test_scramble_unscramble();
void test_flatten_unflatten(); 

void print_coeffs(const char* label, Mod257Pixel* coeffs, int k);
int compare_polys(Mod257Pixel* a, Mod257Pixel* b, int k);
void test_lagrange_recovery();
void test_process_unprocess();

#endif
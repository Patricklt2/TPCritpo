#ifndef TESTS_FILE_MANAGER_H
#define TESTS_FILE_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <file_manager.h>
#include <assert.h>

int test_create_valid_2x2();
int fileManagerTests();
void test_create_invalid_dimensions() ;
void test_create_with_257_pixel();
void test_write_valid_bmp();
void test_write_null_image();
void test_write_null_filename();
void test_bmp_signature();
void test_read_valid_bmp();
void test_read_nonexistent_file();
void test_read_invalid_signature();
void test_read_non_8bit_bmp();
void test_read_palette_not_grayscale();
#endif
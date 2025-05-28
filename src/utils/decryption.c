#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decryption.h"
#include "lagrange.h"



/**
 * Extrae un share de una imagen portadora usando el bit menos significativo (LSB).
 * La imagen debe ser BMP de 8 bits por píxel.
 *
 * @param cover_file  Ruta al BMP portador.
 * @param share       Puntero a BMP257Image donde se almacenará el share.
 * @param share_number Puntero para almacenar el número de sombra (de bytes 8-9).
 * @param seed        Puntero para almacenar la semilla (de bytes 6-7).
 * @return 0 en éxito, -1 en error.
 */
static int extract_share_from_cover(const char* cover_file, BMP257Image* share, uint16_t* share_number, uint16_t* seed) {
    BMP257Image* cover = read_bmp_257(cover_file);
    if (!cover) {
        fprintf(stderr, "No puedo leer portadora: %s\n", cover_file);
        return -1;
    }

    // Verificar que es BMP de 8 bits por píxel
    if (cover->info_header.bits_per_pixel != 8) {
        fprintf(stderr, "La portadora %s no es de 8 bits por píxel\n", cover_file);
        free_bmp257_image(cover);
        return -1;
    }

    // Extraer semilla (bytes 6-7) y número de sombra (bytes 8-9)
    *seed = cover->file_header.reserved1;
    *share_number = cover->file_header.reserved2;

    // Copiar dimensiones y encabezado
    share->info_header = cover->info_header;
    share->file_header = cover->file_header;

    int w = cover->info_header.width;
    int h = cover->info_header.height;

    // Extraer LSB de cada píxel para reconstruir el share
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint8_t cover_val = get_mod257_value(cover->pixels[y][x]);
            uint8_t share_msb = cover_val & 0x1; // Extraer LSB
            share->pixels[y][x] = value_to_mod257_pixel(share_msb << 7); // Alinear como MSB
        }
    }

    free_bmp257_image(cover);
    return 0;
}

/**
 * Recupera la imagen secreta a partir de k portadoras en un esquema (k, n).
 *
 * @param shades_count  Valor de k (mínimo de sombras necesarias).
 * @param output_file   Ruta donde escribir la imagen secreta recuperada.
 * @param images_count  Número de portadoras proporcionadas.
 * @param cover_files   Array con las rutas de las portadoras.
 * @return 0 en éxito, -1 en error.
 */
int shamir_recover(int shades_count, const char* output_file, int images_count, const char** cover_files) {
    // Validar parámetros
    if (shades_count < 2 || shades_count > 10 || images_count < shades_count) {
        fprintf(stderr, "Parámetros inválidos: k=%d, n=%d (k debe ser 2<=k<=10, n>=k)\n", shades_count, images_count);
        return -1;
    }

    // 1) Leer las portadoras y extraer shares
    BMP257Image** shares = malloc(shades_count * sizeof(BMP257Image*));
    int* x = malloc(shades_count * sizeof(int));
    uint16_t* seeds = malloc(shades_count * sizeof(uint16_t));
    int w = -1, h = -1;

    for (int i = 0; i < shades_count; ++i) {
        shares[i] = create_bmp_257(NULL, 0, 0); // Dimensiones se ajustan en extract
        if (!shares[i]) {
            fprintf(stderr, "Error creando share %d\n", i + 1);
            goto cleanup;
        }

        uint16_t share_number, seed;
        if (extract_share_from_cover(cover_files[i], shares[i], &share_number, &seed) != 0) {
            fprintf(stderr, "Error extrayendo share de %s\n", cover_files[i]);
            goto cleanup;
        }

        // Validar dimensiones consistentes
        if (i == 0) {
            w = shares[i]->info_header.width;
            h = shares[i]->info_header.height;
        } else if (shares[i]->info_header.width != w || shares[i]->info_header.height != h) {
            fprintf(stderr, "Dimensiones no coinciden en %s (%dx%d vs %dx%d)\n",
                    cover_files[i], shares[i]->info_header.width, shares[i]->info_header.height, w, h);
            goto cleanup;
        }

        // Almacenar número de sombra (x[i])
        x[i] = share_number;
        seeds[i] = seed;

        // Validar que los números de sombra sean distintos
        for (int j = 0; j < i; ++j) {
            if (x[j] == x[i]) {
                fprintf(stderr, "Número de sombra duplicado (%d) en %s\n", x[i], cover_files[i]);
                goto cleanup;
            }
        }
    }

    // Validar que todas las semillas sean iguales (para la tabla de permutación)
    for (int i = 1; i < shades_count; ++i) {
        if (seeds[i] != seeds[0]) {
            fprintf(stderr, "Semillas inconsistentes entre portadoras\n");
            goto cleanup;
        }
    }

    // 2) Crear imagen secreta
    BMP257Image* secret = create_bmp_257(NULL, w, h);
    if (!secret) {
        fprintf(stderr, "Error creando imagen secreta\n");
        goto cleanup;
    }
    secret->file_header = shares[0]->file_header; // Copiar encabezado de la primera portadora
    secret->info_header = shares[0]->info_header;
    secret->file_header.reserved1 = 0; // Limpiar bytes reservados
    secret->file_header.reserved2 = 0;

    // 3) Reconstruir píxeles usando interpolación de Lagrange
    int* y = malloc(shades_count * sizeof(int));
    for (int py = 0; py < h; ++py) {
        for (int px = 0; px < w; ++px) {
            // Extraer valores y[i] para el píxel actual
            for (int i = 0; i < shades_count; ++i) {
                y[i] = get_mod257_value(shares[i]->pixels[py][px]) >> 7; // Usar MSB
            }

            // Reconstruir píxel con Lagrange
            int pixel_value = lagrange_interpolate(x, y, shades_count);
            secret->pixels[py][px] = value_to_mod257_pixel(pixel_value);
        }
    }

    // 4) Escribir imagen secreta
    int res = write_bmp_257(secret, output_file);
    if (res != 0) {
        fprintf(stderr, "Error escribiendo imagen secreta: %s\n", output_file);
    }

    // 5) Liberar memoria
    free(y);
    free_bmp257_image(secret);

cleanup:
    for (int i = 0; i < shades_count; ++i) {
        if (shares[i]) free_bmp257_image(shares[i]);
    }
    free(shares);
    free(x);
    free(seeds);
    return res;
}

#include <encryption.h>

/**
 * Oculta un share dentro de una imagen portadora usando el bit menos significativo (LSB).
 * Ambas imágenes deben ser 8-bit grayscale con las mismas dimensiones.
 *
 * @param share       Puntero a BMP257Image con el share (valor 0–256).
 * @param cover_file  Ruta al BMP portador.
 * @param output_file Ruta donde escribir el BMP esteganografiado.
 * @return 0 en éxito, -1 en error.
 */
static int hide_share_bit_in_cover(const BMP257Image* share, const char* cover_file, const char* output_file, int bit_index){
    BMP257Image* cover = read_bmp_257(cover_file);
    if (!cover) {
        fprintf(stderr, "No puedo leer portada: %s\n", cover_file);
        return -1;
    }

    int w = cover->info_header.width;
    int h = cover->info_header.height;
    int sw = share->info_header.width;
    int sh = share->info_header.height;

    if (w != sw || h != sh) {
        fprintf(stderr, "La portadora y el share deben tener el mismo tamaño.\n");
        free_bmp257_image(cover);
        return -1;
    }

    for (int y = 0; y < sh; ++y) {
        for (int x = 0; x < sw; ++x) {
            uint8_t share_val = get_mod257_value(share->pixels[y][x]) & 0xFF;
            uint8_t share_bit = (share_val >> bit_index) & 0x1;
            uint8_t cover_val = cover->pixels[y][x].value;
            cover_val = (cover_val & 0xFE) | share_bit;
            cover->pixels[y][x] = value_to_mod257_pixel(cover_val);
        }
    }

    cover->file_header.reserved1 = share->file_header.reserved1;
    cover->file_header.reserved2 = share->file_header.reserved2;

    int res = write_bmp_257(cover, output_file);
    free_bmp257_image(cover);
    return res;
}


// --------------------------------------------------------------------------------------- //


int shamir_distribute(int shades_count, const char* file_name, int images_count, const char** cover_files){
    if (shades_count <= 0 || images_count <= 0 || shades_count > images_count) return -1;

    uint16_t seed = (unsigned)time(NULL) % UINT16_MAX; 

    // 1) Leemos imagen secreta
    BMP257Image* original = read_bmp_257(file_name);
    if (!original) {
        fprintf(stderr, "Error reading the secret image: %s\n", file_name);
        return -1;
    }

    int w = original->info_header.width;
    int h = original->info_header.height;

    // 2) Creamos n imágenes vacías para shares
    BMP257Image** shares = malloc(images_count * sizeof(BMP257Image*)); 
    for (int i = 0; i < images_count; ++i) {
        shares[i] = create_bmp_257(NULL, w, h);
        shares[i]->file_header.reserved1 = seed;  // guardo seed
        shares[i]->file_header.reserved2 = i + 1; // para identificar el share
        if (!shares[i]) {
            fprintf(stderr, "Error in the creation of a share %d\n", i+1);
            return -1;
        }
    }

    // 3) Buffers de coeficientes para cada píxel
    uint16_t* coeffs = malloc(shades_count * sizeof(uint16_t));

    // 4) Para cada píxel de la original, generar y evaluar polinomio
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            coeffs[0] = get_mod257_value(original->pixels[y][x]);
            for (int j = 1; j < shades_count; ++j)
                coeffs[j] = rand() % 257;

            for (int i = 0; i < images_count; ++i) {
                if (!shares[i] || !shares[i]->pixels || !shares[i]->pixels[y]) {
                    fprintf(stderr, "NULL pointer detected: share %d, y %d\n", i, y);
                    exit(1);
                }
                uint16_t xv = i + 1, yv = 0, pow = 1;
                for (int j = 0; j < shades_count; ++j) {
                    yv = mod257_add(yv,
                        mod257_mul(coeffs[j], pow));
                    pow = mod257_mul(pow, xv);
                }
                shares[i]->pixels[y][x] = original->pixels[y][x];
            }
        }
    }

    free(coeffs);
    free_bmp257_image(original);

    // 5) Ocultamos cada share en su portadora correspondiente
    for (int i = 0; i < images_count; ++i) {
        char out[256];
        snprintf(out, sizeof(out), "stego_share_%d.bmp", i+1);
        if (hide_share_bit_in_cover(shares[i], cover_files[i], out, 0) != 0) {
            fprintf(stderr, "Fallo ocultando share %d\n", i+1);
            return -1;
        }
        free_bmp257_image(shares[i]);
    }
    return 0;
}


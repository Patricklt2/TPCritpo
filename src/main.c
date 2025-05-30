
#include <parser.h>
#include <encryption.h>
#include <decryption.h>

 int main(int argc, char *argv[]) {
    Args *arguments = malloc(sizeof(Args));
    if (!arguments) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    } 
    if (parse_args(argc, argv, arguments)) {
        printf("Parsing succeeded\n");
        printf("Stating processing...\n");
        // Here you would call the appropriate functions based on the parsed arguments
        const char* coverfiles[] = {"../src/assets/Alfredssd.bmp","../src/assets/Albertssd.bmp","../src/assets/Audreyssd.bmp","../src/assets/Evassd.bmp","../src/assets/Facundo.bmp","../src/assets/Gustavossd.bmp","../src/assets/Jamesssd.bmp","../src/assets/Marilynssd.bmp"};
        const char* encodedFiles[] = {"../src/encodings/stego_share_1.bmp", "../src/encodings/stego_share_2.bmp", "../src/encodings/stego_share_3.bmp", "../src/encodings/stego_share_4.bmp", "../src/encodings/stego_share_5.bmp", "../src/encodings/stego_share_6.bmp", "../src/encodings/stego_share_7.bmp", "../src/encodings/stego_share_8.bmp"};
        if ( arguments->distribute ){
            if (  arguments->n < arguments->k ) {
                fprintf(stderr, "Error: n must be greater than or equal to k\n");
                goto free;
            }
            if ( arguments->secret == NULL ) {
                fprintf(stderr, "Error: --secret must be specified for distribution\n");
                goto free;
            }
            printf("Distributing secret image: %s\n", arguments->secret);
            shamir_distribute(arguments->k, arguments->secret, arguments->n, coverfiles); // Assuming you will handle the output files later
        }else if ( arguments->recover ){
            printf("Recovering secret image from shares with k=%d\n", arguments->k);
            shamir_recover(arguments->k, "hola.bmp", 8, encodedFiles); // Assuming you will handle the input files later
        } else {
            fprintf(stderr, "Error: Invalid operation specified\n");
            goto free;
        }
        free(arguments);
        printf("Processing done!\n");
        return 0;
    } else {
        fprintf(stderr, "Parsing failed\n");
        fprintf(stderr, "Usage: %s (-d | -r) --secret <image.bmp> -k <number> [-n <number>] [--dir <directory>]\n", argv[0]);
        goto free;
    }

    free:
    free(arguments);
    return 1;
}


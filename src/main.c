
#include "./utils/headers/parser.h"
#include "./utils/headers/encryption.h"

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
        const char* coverfiles[] = {"Alfredssd.bmp","Albertssd.bmp","Audreyssd.bmp","Evassd.bmp","Facundo.bmp","Gustavossd.bmp","Jamesssd.bmp","Marilyssd.bmp"};
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


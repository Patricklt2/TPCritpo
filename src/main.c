
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

        //Put cover file paths tour Current Working Directory (base path) will be ./src/ at first. Keep that in mind when assigning path. Must be null terminated.
        char** coverfiles;
       
        //Put encoded file paths your Current Working Directory (base path) will be ./src/ at first. Keep that in mind when assigning path. Must be NULL terminated
        char** encodedFiles;
                    //Put encoded file paths your Current Working Directory (base path) will be ./src/ at first. Keep that in mind when assigning path. Must be NULL terminated
        
        
        if (arguments->dir != NULL) {
            if (arguments->distribute){
                coverfiles = list_files_with_null(arguments->dir);
                
            }else if (arguments->recover){
                // Here you would set the directory for recovery
                printf("Using directory: %s\n", arguments->dir);
                encodedFiles = list_files_with_null(arguments->dir);
            }else {
                fprintf(stderr, "Error: Invalid operation specified\n");
                goto free;
            }
        }else{
            if (arguments->distribute){
                coverfiles = list_files_with_null(NULL);
                
            }else if (arguments->recover){
                encodedFiles = list_files_with_null(NULL);
            }else {
                fprintf(stderr, "Error: Invalid operation specified\n");
                goto free;
            }
        }

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
            shamir_distribute(arguments->k, arguments->secret,arguments->n, coverfiles);
        }else if ( arguments->recover ){
            printf("Recovering secret image from shares with k=%d\n", arguments->k);

            shamir_recover(arguments->k, arguments->secret, arguments->n, encodedFiles); // Assuming you will handle the input files later
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


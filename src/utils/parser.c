
#include <parser.h>

static bool file_exists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

static bool is_bmp_file(const char *filename) {
    size_t len = strlen(filename);
    return len >= 4 && strcmp(filename + len - 4, ".bmp") == 0;
}

static struct option long_options[] = {
    {"secret", required_argument, 0, 's'},
    {"dir", required_argument, 0, 'i'}, // using i flag because d flag already exists
    {0, 0, 0, 0}
};

bool parse_args(int argc, char **argv, Args *args) {
    int c;
    bool distribute = false;
    bool recover = false;
    bool secret_set = false;
    bool k_set = false;
    int k = 0;
    int n = 0;
    char *secret_file = NULL; 
    char *dir = NULL;
    char *endptr;

    while ((c = getopt_long(argc, argv, "hdrk:n:", long_options, NULL)) != -1) {
        switch (c) {
            case 'd':
                printf("Detected -d flag\n");
                if (recover) {
                    fprintf(stderr, "Error: -d and -r are mutually exclusive\n");
                    free(secret_file);
                    free(dir);
                    return false;
                }
                distribute = true;
                break;
            case 'r':
                printf("Detected -r flag\n");
                if (distribute) {
                    fprintf(stderr, "Error: -d and -r are mutually exclusive\n");
                    free(secret_file);
                    free(dir);
                    return false;
                }
                recover = true;
                break;
            case 'k':
                printf("Detected -k flag with value %s\n", optarg);
                k = strtol(optarg, &endptr, 10);
                if (*endptr != '\0' || k < 2 || k > 10) {
                    fprintf(stderr, "Error: -k requires a positive integer between 2 and 10\n");
                    free(secret_file);
                    return false;
                }
                k_set = true;
                break;
            case 'n':       // optional
                printf("Detected -n flag with value %s\n", optarg);
                /*
                if (recover) {
                    fprintf(stderr, "Error: -n is only valid with -d\n");
                    free(secret_file);
                    return false;
                }*/
                n = strtol(optarg, &endptr, 10);
                if (*endptr != '\0' || n < 2) {
                    fprintf(stderr, "Error: -n requires a positive integer\n");
                    free(secret_file);
                    free(dir);
                    return false;
                }
                break;
            case 's':
                printf("Detected --secret flag with value %s\n", optarg);
                if (!is_bmp_file(optarg)) {
                    fprintf(stderr, "Error: --secret must be a .bmp file\n");
                    free(secret_file);
                    free(dir);
                    return false;
                }
                secret_file = strdup(optarg); 
                secret_set = true;
                break;
            case 'i': // --dir
                printf("Detected --dir flag with value %s\n", optarg);
                dir = strdup(optarg);
                break;
            case 'h':
                printf("Usage: %s (-d | -r) --secret <image.bmp> -k <number> [-n <number>] [--dir <directory>]\n", argv[0]);
                printf("Options:\n");
                printf("  -h,                 For help with the commands\n");
                printf("  -d,                 Distribute the secret image\n");
                printf("  -r                  Recover the secret image\n");
                printf("  -k <number>         Set the number of shares (2-10)\n");
                printf("  -n <number>         Set the total number of images (optional, only with -d)\n");
                printf("  --secret <file>     Specify the secret image file (must be .bmp)\n");
                printf("  --dir <directory>   Specify the output directory (optional)\n");
                return false;
            default:
                fprintf(stderr, "Error: Unknown option\n");
                free(secret_file);
                free(dir);
                return false;
        }
    }

 
    if (!distribute && !recover) {
        fprintf(stderr, "Error: Either -d or -r must be specified\n");
        free(secret_file);
        free(dir);
        return false;
    }
    if (!secret_set) {
        fprintf(stderr, "Error: --secret is required\n");
        free(secret_file);
        free(dir);
        return false;
    }
    if (!k_set) {
        fprintf(stderr, "Error: -k is required\n");
        free(secret_file);
        free(dir);
        return false;
    }

    char* base_path = getcwd(NULL, 0);
    char* complete_filename = malloc(strlen(secret_file) + strlen(base_path) + 1);
    complete_filename = strcpy(complete_filename, base_path);

    if (distribute && secret_file && !file_exists(complete_filename)) {
        fprintf(stderr, "Error: Secret file %s does not exist for -d\n", secret_file);
        free(secret_file);
        free(dir);
        free(complete_filename);
        return false;
    }
    free(complete_filename);
    if (n != 0 && n < k) {
        fprintf(stderr, "Error: n must be >= k\n");
        free(secret_file);
        free(dir);
        return false;
    }

    // Asignar valores a args
    args->distribute = distribute;
    args->recover = recover;
    args->secret = secret_file; 
    args->k = k;
    args->n = n ? n : k; // Si n no se especifica, usar k
    args->dir = dir;

    return true;
}
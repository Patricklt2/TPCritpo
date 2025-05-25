

#include <stdio.h>
#include "./utils/parser.h"

 int main(int argc, char *argv[]) {
    if (parse_args(argc, argv)) {
        printf("Parsing succeeded\n");
        return 0;
    } else {
        fprintf(stderr, "Parsing failed\n");
        fprintf(stderr, "Usage: %s (-d | -r) --secret <image.bmp> -k <number> [-n <number>] [--dir <directory>]\n", argv[0]);
        return 1;
    }
}


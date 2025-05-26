#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#define FLAGS_AMOUNT 7

typedef struct {
    bool distribute;
    bool recover;
    char *secret;
    int k;
    int n;
    char *dir;
} Args;

bool parse_args(int argc, char **argv);

#endif // PARSER_H
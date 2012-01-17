/*
 * owon-utils - a set of programs to use with OWON Oscilloscopes
 * Copyright (c) 2012  Levi Larsen <levi.larsen@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <getopt.h>
#include <limits.h>
#include "owon.h"
#include "parse.h"

#define __(x) #x
#define PROGRAM __(owonparse)
#define PACKAGE __(owon-utils)
#define VERSION __(0.1)
#define AUTHORS __(Levi Larsen)

static char *invocation_name;

struct {
    int verbose;
    char *format;
    char *delim;
    int header;
} options;

/* For long options that have no equivalent short option, use a
   non-character as a pseudo short option, starting with CHAR_MAX + 1.  */
enum {
    OPTION_HELP = CHAR_MAX + 1,
    OPTION_VERSION
};

static const char *optstring = "f:d:h";
static const struct option longopts[] = {
    {"format", required_argument, NULL, 'f'},
    {"delimiter", required_argument, NULL, 'd'},
    {"header", no_argument, NULL, 'h'},
    {"help", no_argument, NULL, OPTION_HELP},
    {"version", no_argument, NULL, OPTION_VERSION},
    {NULL, no_argument, NULL, 0}
};

void usage(int status) {
    if (status != EXIT_SUCCESS) {
        fprintf(stderr, "Try `%s --help' for more information\n", 
                invocation_name);
    } else {
        printf("Usage: %s [OPTION]... FILEIN FILEOUT\n", invocation_name);
        fputs(
"Parse FILEIN created by owondump and print to FILEOUT in specified FORMAT.\n"
"\n"
"  -f, --format=FORMAT   output in FORMAT\n"
"  -d, --delimiter=DELIM use DELIM as a delimiter for supported formats\n"
"                        (default is \\t)\n"
"  -h, --noheader        do not include header in formats that support it\n"
"  --help                display this help and exit\n"
"  --version             output version information and exit\n"
"\n"
"When FILEIN is -, read from standard input.\n"
"When FILEOUT is -, write to standard output.\n"
"\n"
"Supported formats:\n"
"  delim   Delimited values, use -d to specify delimiter, \n"
"          use -h to include header\n"
, stdout);
    }
    exit(status);
}

void version() {
    printf("%s (%s) %s\n", PROGRAM, PACKAGE, VERSION);
    fputs(
"License GPLv3+: GNU GPL version 3 or later "
"<http://gnu.org/licenses/gpl.html>.\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n"
"\n"
, stdout);
    printf("Written by %s\n", AUTHORS);
    exit(EXIT_SUCCESS);
}

/* Convert floating point number `d` to an appropriate string 
 * representation with SI prefix. Supports kilo (k), milli (m), micro (u), 
 * and nano (n) only. */
char *ftosi(double d) {
    char *format;
    if (d >= 1e6) {
        return NULL;
    } else if (d >= 1e3) {
        d /= 1e3;
        format = "%g k";
    } else if (d >= 1) {
        format = "%g ";
    } else if(d >= 1e-3) {
        d /= 1e-3;
        format = "%g m";
    } else if(d >= 1e-6) {
        d /= 1e-6;
        format = "%g u"; 
    } else if(d >= 1e-9) {
        d /= 1e-9;
        format = "%g n";
    } else {
        return NULL;
    }
    int size = snprintf(NULL, 0, format, d) + 1;
    char *str = malloc(size);
    snprintf(str, size, format, d);
    return str;
}

int main(int argc, char **argv) {
    // make copy because basename might modify path
    char *argv0 = strdup(argv[0]);
    // make copy because basename might reuse pointer
    invocation_name = strdup(basename(argv0));
    free(argv0);
   
    // default options
    options.format = "delim";
    options.delim = "\t";
    options.header = 1;

    int opt = getopt_long(argc, argv, optstring, longopts, NULL);
    while (opt > -1) {
        switch (opt) {
            case 'f':
                options.format = optarg;
                break;
            case 'd':
                options.delim = optarg;
                break;
            case 'h':
                options.header = 0;
                break;
            case OPTION_HELP:
                usage(EXIT_SUCCESS);
            case OPTION_VERSION:
                version();
            default:
                usage(EXIT_FAILURE);
        }
        opt = getopt_long(argc, argv, optstring, longopts, NULL);
    }

    int fargc = argc - optind;
    
    if (fargc < 2) {
        fprintf(stderr, "FILEIN and FILEOUT are required.\n");
        usage(EXIT_FAILURE);
    }
    
    if (fargc > 2) {
        fprintf(stderr, "Too many file arguments.\n");
        usage(EXIT_FAILURE);
    }

    char *filein = NULL;
    char *fileout = NULL;

    // read from stdin
    if (*argv[optind] != '-') {
        filein = argv[optind];
    }

    // write to stdout
    if (*argv[optind + 1] != '-') {
        fileout = argv[optind + 1];
    }

    FILE *finp;
    if (NULL == filein) {
        finp = stdin;
    } else {
        finp = fopen(filein, "rb");
        if (NULL == finp) {
            fprintf(stderr, "Unable to open %s\n", filein);
            exit(EXIT_FAILURE);
        }
    }
    
    struct owon_capture capture;
    int ret = owon_parse(&capture, finp);
    if (ret != OWON_SUCCESS) {
        switch (ret) {
            case OWON_ERROR_UNSUPPORTED:
                fprintf(stderr, "The osocilloscope model or feature is not "
                                "currently supported.\n");
                exit(EXIT_FAILURE);
            case OWON_ERROR_MEMORY:
                fprintf(stderr, "Unable to allocate adquate memory.\n");
                exit(EXIT_FAILURE);
            case OWON_ERROR_READ:
                fprintf(stderr, "A read error occured.\n");
                exit(EXIT_FAILURE);
            case OWON_ERROR_HEADER:
                fprintf(stderr, "This file is not in the correct format.\n");
                exit(EXIT_FAILURE);
            default:
                fprintf(stderr, "An unknown error occurred.\n");
                exit(EXIT_FAILURE);
        }
    }
    
    // Only close if actual file (not stdin)
    if (NULL == filein) {
        fclose(finp);
    }

    FILE *foutp;
    if (NULL == fileout) {
        foutp = stdout;
    } else {
        foutp = fopen(fileout, "wb");
        if (NULL == foutp) {
            fprintf(stderr, "Unable to open %s\n", fileout);
            exit(EXIT_FAILURE);
        }
    }
   
    if (0 == strcmp(options.format, "delim")) {
        owon_write_delim(&capture, options.delim, "\n", 
                options.header, foutp);
    } else {
        fprintf(stderr, "Unrecognized format.\n");
        usage(EXIT_FAILURE);
    }

    // Only close if actual file (not stdout)
    if (NULL == fileout) {
        fclose(foutp);
    }
    
    owon_free_capture(&capture);
    
    free(invocation_name);

    return EXIT_SUCCESS;
}

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
#include <getopt.h>
#include <limits.h> // CHAR_MAX
#include "owon.h"
#include "usb.h"

#define __(x) #x
#define PROGRAM __(owondump)
#define PACKAGE __(owon-utils)
#define VERSION __(0.1)
#define AUTHORS __(Levi Larsen)

static char *invocation_name;

enum {
    OPTION_HELP = CHAR_MAX + 1,
    OPTION_VERSION
};

static const char *optstring = "";
static const struct option longopts[] = {
    {"help", no_argument, NULL, OPTION_HELP},
    {"version", no_argument, NULL, OPTION_VERSION},
    {NULL, 0, NULL, 0}
};

void usage(int status) {
    if (status != EXIT_SUCCESS) {
        fprintf(stderr, "Try `%s --help' for more information.\n", 
                invocation_name);
    } else {
        printf("Usage: %s [OPTION]... FILE\n", invocation_name);
        fputs(
"Download data from OWON oscilloscopes to FILE.\n"
"\n"
"  --help                display this help and exit\n"
"  --version             output version information and exit\n"
"\n"
"When FILE is -, write to standard output.\n"
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
    printf("Written by %s.\n", AUTHORS);
    exit(EXIT_SUCCESS);
}

int main (int argc, char **argv) {
    invocation_name = argv[0];
    
    //TODO: add verbose option
    //TODO: add option to download continuously
    int opt = getopt_long(argc, argv, optstring, longopts, NULL);
    while (opt > -1) {
        switch (opt) {
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
    
    if (fargc < 1) {
        fprintf(stderr, "FILE argument required.\n");
        usage(EXIT_FAILURE);
    }

    if (fargc > 1) {
        fprintf(stderr, "Too many file arguments.\n");
        usage(EXIT_FAILURE);
    }

    char *fileout = NULL;

    if (*argv[optind] != '-') {
        fileout = argv[optind];
    }

    // Get file pointer to file or stdout.
    FILE *fp;
    if (NULL == fileout) {
        fp = stdout;
    } else {
        fp = fopen(fileout, "wb");
        if (NULL == fp) {
            fprintf(stderr, "Unable to open %s\n", fileout);
            exit(EXIT_FAILURE);
        }
    }

    owon_usb_init();
    struct usb_device *dev = owon_usb_get_device();
    if (NULL == dev) {
        fprintf(stderr, "No devices found\n");
        exit(EXIT_FAILURE);
    }
    struct usb_dev_handle *dev_handle = owon_usb_open(dev);
    if (NULL == dev_handle) {
        fprintf(stderr, "Unable to open device\n");
        exit(EXIT_FAILURE);
    }
    char *buffer;
    long length = 0;
    length = owon_usb_read(dev_handle, &buffer);
    if (0 > length) {
        fprintf(stderr, "Error reading from device: %li\n", length);
    }
    owon_usb_close(dev_handle);

    // Write data out
    fwrite(buffer, sizeof(char), length, fp);
    free(buffer);

    // Only close fp if it's an actually file (don't close stdout).
    if (NULL != fileout) {
        fclose(fp);
    }

    return 0;
}


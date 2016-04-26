/* This file is part of brightlight v2
** Copyright (C) 2016 David Miller <multiplexd@gmx.com>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <bsd/string.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define BACKLIGHT_PATH "/sys/class/backlight/intel_backlight"
#define MAX_PATH_LEN 200
#define EXTRA_PATH_LEN 20
#define CHAR_ARG_LEN 10
#define PROGRAM_NAME "brightlight"
#define PROGRAM_VERSION "2"

unsigned int get_backlight;
unsigned int set_backlight;
unsigned int max_brightness;
unsigned int inc_brightness;
unsigned int dec_brightness;
int brightness;               /* Signed because of file I/O testing done in read_maximum_brightness() which requires signed ints */
unsigned int maximum;
unsigned int values_as_percentages;
unsigned int delta_brightness;
unsigned int current_brightness;
char *argv0;
char backlight_path[MAX_PATH_LEN];

enum errors {
   ERR_OPEN_FILE,          /* Error trying to open a file */
   ERR_READ_FILE,          /* Error reading from a file */
   ERR_WRITE_FILE,         /* Error writing to a file */
   ERR_NO_OPTS,            /* No options specified */
   ERR_OPT_CONFLICT,       /* Conflicting options given */
   ERR_PARSE_OPTS,         /* Error parsing options */
   ERR_INVAL_OPT,          /* Invalid option argument */
   ERR_FILE_IS_NOT_DIR,    /* Specified control directory is a file */
   ERR_ACCES_ON_DIR,       /* Got permission denied accessing the control directory */
   ERR_CONTROL_DIR,        /* Control directory couldn't be accessed */
   ERR_FILE_ACCES,         /* Couldn't access a file inside the control directory */
   ERR_OPT_NOT_KNOWN,      /* Unkown option specified */
   ERR_OPT_INCOMPLETE,     /* Option requires an argument */
   ERR_ARG_OVERLOAD        /* Too may arguments were given */
};

static const struct option longopts[] = {
   {"decrement", required_argument, NULL, 'd'},
   {"decrease", required_argument, NULL, 'd'},
   {"file", required_argument, NULL, 'f'},
   {"help", no_argument, NULL, 'h'},
   {"increment", required_argument, NULL, 'i'},
   {"increase", required_argument, NULL, 'i'},
   {"maximum", no_argument, NULL, 'm'},
   {"percentage", no_argument, NULL, 'p'},
   {"read", no_argument, NULL, 'r'},
   {"version", no_argument, NULL, 'v'},
   {"write", required_argument, NULL, 'w'},
   {0, 0, 0, 0},
};

void change_existing_brightness();
unsigned int get_value_from_file(char* path_suffix);
void parse_args(int argc, char* argv[]);
unsigned int parse_cmdline_int(char* arg_to_parse);
void read_backlight_brightness();
void throw_error(enum errors, char* opt_arg);
void usage();
void validate_args();
void validate_control_directory();
void validate_decrement(unsigned int reference_value); 
void validate_increment(unsigned int reference_value);
void version();
void write_backlight_brightness();
void write_brightness_to_file(unsigned int bright);


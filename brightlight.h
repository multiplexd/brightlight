/* This file is part of brightlight v4
** Copyright (C) 2016, 2017 multiplex'd <multiplexd@gmx.com>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** file LICENSE for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#define BACKLIGHT_PATH "/sys/class/backlight/intel_backlight"
#define MAX_PATH_LEN 200
#define EXTRA_PATH_LEN 20
#define CHAR_ARG_LEN 10
#define PROGRAM_NAME "brightlight"
#define PROGRAM_VERSION "5"

/* Global variables */

unsigned int backlight_operation = 0;

int brightness;               /* Signed because of file I/O testing done in read_maximum_brightness() which requires signed ints */
unsigned int maximum;
unsigned int values_as_percentages;
unsigned int delta_brightness;
unsigned int current_brightness;
char *argv0;
char backlight_path[MAX_PATH_LEN];

/* Flags for backlight_operation */

#define BL_GET_BRIGHTNESS 1
#define BL_SET_BRIGHTNESS 2
#define BL_MAX_BRIGHTNESS 3
#define BL_INC_BRIGHTNESS 4
#define BL_DEC_BRIGHTNESS 5

/* List of errors */

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
   ERR_OPT_GETOPT,         /* Getopt encountered an error when parsing options */
   ERR_ARG_OVERLOAD        /* Too may arguments were given */
};

/* Getopt_long options structure */

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

/* Function prototypes */

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


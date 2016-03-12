/* This file is part of brightlight v2-rc2
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define BACKLIGHT_PATH "/sys/class/backlight/intel_backlight"
#define MAX_PATH_LEN 200
#define EXTRA_PATH_LEN 20
#define PROGRAM_NAME "brightlight"
#define PROGRAM_VERSION "2-rc2"

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
   ERR_OPEN_FILE,
   ERR_READ_FILE,
   ERR_WRITE_FILE,
   ERR_NO_OPTS,
   ERR_OPT_CONFLICT,
   ERR_PARSE_OPTS,
   ERR_INVAL_OPT,
   ERR_FILE_IS_NOT_DIR,
   ERR_ACCES_ON_DIR,
   ERR_CONTROL_DIR,
   ERR_FILE_ACCES,
};

void change_existing_brightness();
unsigned int get_value_from_file(char* path_suffix);
void parse_args(int argc, char* argv[]);
unsigned int parse_cmdline_int(char* arg_to_parse);
void read_backlight_brightness();
void read_maximum_brightness();
void throw_error(enum errors, char* opt_arg);
void usage();
void validate_args();
void validate_control_directory();
void validate_decrement(unsigned int reference_value); 
void validate_increment(unsigned int reference_value);
void version();
void write_backlight_brightness();
void write_brightness_to_file(unsigned int bright);


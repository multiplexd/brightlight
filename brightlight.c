/* brightlight v5 - change the screen backlight brightness on Linux systems
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

/*
** This program requires libbsd <libbsd.freedesktop.org> or a BSD-compatible 
** implementation of strlcpy() and strlcat().
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

#include "brightlight.h"

int main(int argc, char* argv[]) {

   /* Record the program invocation name, should it be needed */
   argv0 = argv[0];

   /* Parse the command line arguments */
   parse_args(argc, argv);

   if(backlight_operation == 0)
      /* No operation was specified, backlight_operation is unchanged */
            throw_error(ERR_PARSE_OPTS, "");

   /* Make sure the sysfs control directory is accessible */
   validate_control_directory();

   /* The maximum backlight value isn't always needed, but it is required often
      enough to always read it */
   maximum = get_value_from_file("/max_brightness");


   /* Take action based on value of backlight_operation */
   switch(backlight_operation) {
   case BL_GET_BRIGHTNESS:
      read_backlight_brightness();
      break;
   case BL_SET_BRIGHTNESS:
      validate_args(); /* Check command line arguments are sane. Needs the maximum brightness. */
      write_backlight_brightness();
      break;
   case BL_MAX_BRIGHTNESS:
      printf("Maximum backlight brightness is: %d.\n", maximum); /* Maximum brightness 
								    is known at this point. */
      break;
   case BL_INC_BRIGHTNESS:
      change_existing_brightness();
      break;
   case BL_DEC_BRIGHTNESS:
      change_existing_brightness();
      break;
   }
   
   /* Things must have worked if we get here; exit cleanly */
   exit(0);
}

void change_existing_brightness() {

   /* Increment or decrement the backlight brightness */
   
   unsigned int oldval;
   unsigned int current;
   unsigned int val_to_write;
   char* out_string_end;       /* Placeholder strings to format the program output nicely */
   char* out_string_filler;

   /* The current backlight brightness is required so we can tell if our
      command line arguments are sane or not */
   current = get_value_from_file("/brightness");

   /* Calculate the value that should be written to the control file, account
      for values as percentages and generate placeholder strings for formatting
      the program output */
   if(backlight_operation == BL_INC_BRIGHTNESS) {

      /* Check the command line arguments */
      validate_increment(current);
      
      if(values_as_percentages) {
         val_to_write = current + ((delta_brightness * maximum) / 100);
         oldval = (current * 100) / maximum;
         out_string_end = "%.";
         out_string_filler = "% ";
      } else {
         val_to_write = current + delta_brightness;
         oldval = current;
         out_string_end = ".";
         out_string_filler = " ";
      }

      /* Actually write the required value to the control file */
      write_brightness_to_file(val_to_write);
      
      /* Display an informational message on stdout */
      printf("Changed backlight brightness: %d%s=> %d%s\n", oldval, out_string_filler, oldval + delta_brightness, out_string_end);
      
   } else if(backlight_operation == BL_DEC_BRIGHTNESS){

      /* Check the command line arguments */
      validate_decrement(current);
      
      if(values_as_percentages) {
         val_to_write = current - ((delta_brightness * maximum) / 100);
         oldval = (current * 100) / maximum;
         out_string_end = "%.";
         out_string_filler = "% ";
      } else {
         val_to_write = current - delta_brightness;
         oldval = current;
         out_string_end = ".";
         out_string_filler = " ";
      }

      /* Actually write the required value to the control file */
      write_brightness_to_file(val_to_write);

      /* Display an informational message on stdout */
      printf("Changed backlight brightness: %d%s=> %d%s\n", oldval, out_string_filler, oldval - delta_brightness, out_string_end);
      
   }

   return;
}

unsigned int get_value_from_file(char* path_suffix) {

   /* Read a file from the backlight brightness control directory */

   int value = -1;  /* Initialise to error value to fail safe */
   FILE* file_to_read;
   char path[MAX_PATH_LEN + EXTRA_PATH_LEN]; /* Buffer to hold the path of the file we are going to read */

   /* Safely copy the path to the backlight control directory into the buffer
      and append the subpath of the file we are going to read */
   strlcpy(path, backlight_path, MAX_PATH_LEN);
   strlcat(path, path_suffix, MAX_PATH_LEN + EXTRA_PATH_LEN);

   /* Try to open the file we are going to read */
   file_to_read = fopen(path, "r");
   if(file_to_read == NULL)
      /* Something went wrong and the file couldn't be opened */
      throw_error(ERR_OPEN_FILE, path_suffix);

   /* Try to read a value from the file */
   fscanf(file_to_read, "%d", &value);
   if(value < 0)
      /* The value variable wasn't changed, so something went wrong */
      throw_error(ERR_READ_FILE, path_suffix);

   /* Clean up after ourselves */
   fclose(file_to_read);

   /* Cast the value read from the file to an unsigned value as used by the 
      rest of the program */
   return (unsigned int) value;
}

void parse_args(int argc, char* argv[]) {

   /* Pares the command line arguments */
   
   char opt; /* Used with getopt() below */
   int conflicting_args, show_version, show_help;
   char* cmdline_brightness = "";
  
   /* Initialise variables */
   brightness = 0;
   conflicting_args = 0;
   show_version = 0;
   show_help = 0;

   /* Safely copy the default control directory path to the buffer holding the path */
   strlcpy(backlight_path, BACKLIGHT_PATH, MAX_PATH_LEN);

   if(argc == 1)
      throw_error(ERR_NO_OPTS, ""); /* Arguments are required */

   /* Let getopt_long detect bad options */
   /* opterr = 0; */ /* We'll do our own error handling here */

   /* Parse our command line options; see the usage summary in usage() */
   while((opt = getopt_long(argc, argv, "d:f:hi:mprvw:", longopts, NULL)) != -1) {
      switch(opt) {
      case 'd':
         if(conflicting_args)
            throw_error(ERR_OPT_CONFLICT, "");
         backlight_operation = BL_DEC_BRIGHTNESS;
         conflicting_args = 1;
         cmdline_brightness = optarg;
         break;
      case 'f':
         strlcpy(backlight_path, optarg, MAX_PATH_LEN); /* Use a different control directory */
         break;
      case 'h':
         show_help = 1;
         break;
      case 'i':
         if(conflicting_args)
            throw_error(ERR_OPT_CONFLICT, "");
         backlight_operation = BL_INC_BRIGHTNESS;
         conflicting_args = 1;
         cmdline_brightness = optarg;
         break;
      case 'm':
         if(conflicting_args)
            throw_error(ERR_OPT_CONFLICT, "");
         backlight_operation = BL_MAX_BRIGHTNESS;
         conflicting_args = 1;
         break;
      case 'p':
         values_as_percentages = 1;
         break;
      case 'r':
         if(conflicting_args)
            throw_error(ERR_OPT_CONFLICT, "");
         backlight_operation = BL_GET_BRIGHTNESS;
         conflicting_args = 1;
         break;
      case 'v':
         show_version = 1;
         break;
      case 'w':
         if(conflicting_args)
            throw_error(ERR_OPT_CONFLICT, "");
         backlight_operation = BL_SET_BRIGHTNESS;
         conflicting_args = 1;
         cmdline_brightness = optarg;
         break;
      case '?':
         /* getopt_long will output its own error message at this point, but we also want
            to provide information about the --help flag */
         throw_error(ERR_OPT_GETOPT, "");
      }
         
   }

   if(optind != argc)
      /* We have too many arguments */
      throw_error(ERR_ARG_OVERLOAD, "");

   /* Show a help and usage message if neccesary */
   if(show_help || show_version) { /* If either -v or -h are specified */
      if(show_version)
         version();

      if(show_version && show_help) /* If we get *both* -v and -h */
         putchar('\n'); /* Print a newline */

      if(show_help)
         usage();
      exit(0);
   }

   if(backlight_operation == BL_SET_BRIGHTNESS)
      /* Check that the argument we recieved is okay */
      brightness = parse_cmdline_int(cmdline_brightness);
   
   if(backlight_operation == BL_INC_BRIGHTNESS ||
      backlight_operation == BL_DEC_BRIGHTNESS)
      /* Same as immediately above */
      delta_brightness = parse_cmdline_int(cmdline_brightness);

   return;
}


unsigned int parse_cmdline_int(char* arg_to_parse) {

   /* Check that any 'integers' we have received on the command line
      are actually parseable ints */

   int character = 0;

   /* Check that all the characters of the argument are digits */
   while(arg_to_parse[character] != '\0') {

      /* Don't allow arguments longer than five characters */
      if(character >= 5 || isdigit(arg_to_parse[character]) == 0)
         throw_error(ERR_INVAL_OPT, "");
      character++;

   }

   /* Return the passed argument as an unsigned integer */
   return (unsigned int) atoi(arg_to_parse);
}

void read_backlight_brightness() {

   /* Read and print the backlight brightness */
   
   unsigned int outval;
   char* out_string_end; /* Placeholder string to format the program output nicely */

   /* Actually read the value from the file */
   brightness = get_value_from_file("/brightness");

   /* Account for any values as percentages and set the placeholder string */
   if(values_as_percentages) {
      outval = (brightness * 100) / maximum;
      out_string_end = "%.";
   } else {
      outval = brightness;
      out_string_end = ".";
   }

   /* Display an informational message on stdout */
   printf("Current backlight brightness is: %d%s\n", outval, out_string_end);

   return;
}

void throw_error(enum errors err, char* opt_arg) {

   /* Display failure messages upon encountering errors */
   /* See brightlight.h */
   
   switch(err) {
   case ERR_OPEN_FILE:
      fprintf(stderr, "Error occured while trying to open %s file.\n", opt_arg);
      break;
   case ERR_READ_FILE:
      fprintf(stderr, "Could not read from %s file.\n", opt_arg);
      break;
   case ERR_WRITE_FILE:
      fputs("Could not write brightness to brightness file.\n", stderr);
      break;
   case ERR_NO_OPTS:
      fputs("No options specified. Pass the --help flag for help.\n", stderr);
      break;
   case ERR_OPT_CONFLICT:
      fputs("Conflicting options given! Pass the --help flag for help.\n", stderr);
      break;
   case ERR_PARSE_OPTS:
      fputs("Error parsing options. Pass the --help flag for help.\n", stderr);
      break;
   case ERR_INVAL_OPT:
      fputs("Invalid argument. Pass the --help flag for help.\n", stderr);
      break;
   case ERR_FILE_IS_NOT_DIR:
      fprintf(stderr, "%s is not a directory.\n", opt_arg);
      break;
   case ERR_ACCES_ON_DIR:
      fprintf(stderr, "Could not access %s: Permission denied.\n", opt_arg);
      break;
   case ERR_CONTROL_DIR:
      fputs("Could not access control directory.\n", stderr);
      break;
   case ERR_FILE_ACCES:
      fprintf(stderr, "Control directory exists but could not find %s control file.\n", opt_arg);
      break;
   case ERR_OPT_GETOPT:
      fprintf(stderr, "Pass the --help flag for help.\n");
      break;
   case ERR_ARG_OVERLOAD:
      fputs("Too many arguments. Pass the --help flag for help.\n", stderr);
      break;
   }

   /* Exit indicating failure */
   exit(1);
}

void usage() {

   /* Display usage instructions on stdout */
   
   printf("Usage: %s [OPTIONS]\n", argv0);
   printf("\
Options:\n\
\n\
  -v, --version          Print program version and exit.\n\
  -h, --help             Show this help message.\n\
  -p, --percentage       Read or write the brightness level as a percentage \n\
                         (0 to 100) instead of the internal scale the kernel \n\
                         uses (such as e.g. 0 to 7812).\n\
  -r, --read             Read the backlight brightness level.\n\
  -w, --write <val>      Set the backlight brightness level to <val>, where \n\
                         <val> is a positive integer.\n\
  -i, --increment <val>  Increment/increase the backlight brightness level by\n\
                         <val>, where <val> is a positive integer.\n\
      --increase <val>   Same as --increment.\n\
  -d, --decrement <val>  Decrement/decrease the backlight brightness level by\n\
                         <val>, where <val> is a positive integer.\n\
      --decrease <val>   Same as --decrement.\n\
  -f, --file <path>      Specify alternative path to backlight control \n\
                         directory. This is likely to be a subdirectory under\n\
                         \"/sys/class/backlight/\".\n\
  -m, --maximum          Show maximum brightness level of the screen \n\
                         backlight on the kernel's scale. The compile-time \n\
                         default control directory is used if -f or --file is\n\
                         not specified. The -p and --percentage flags are \n\
                         ignored when this option is specified.\n\n");

   printf("The flags -r, -w, -m, -i, -d and their corresponding long options are mutually \nexclusive, \
however one of them is required.\n");

   return;
}

void validate_args() {

   /* Check that the brightness specified on the command line is an allowable value. 
      Don't check if the brightness is an int, already done by parse_cmdline_int(). */

   if(values_as_percentages) {
      if(brightness < 0 || brightness > 100) 
         throw_error(ERR_INVAL_OPT, "");
   } else {
      if(brightness < 0 || brightness > maximum) 
         throw_error(ERR_INVAL_OPT, "");
   }

   return;
}

void validate_control_directory() {

   /* Make sure that the control directory exists and contains the expected files */
   
   DIR* control_dir;
   char path[MAX_PATH_LEN + EXTRA_PATH_LEN]; /* Buffer for holding the path to the directory */

   /* Open the directory */
   control_dir = opendir(backlight_path);

   if(control_dir == NULL) {
      /* Something went wrong in trying to access the directory */
      if(errno == ENOTDIR) 
         throw_error(ERR_FILE_IS_NOT_DIR, backlight_path);
      else if(errno == EACCES) 
         throw_error(ERR_ACCES_ON_DIR, backlight_path);
      else 
         throw_error(ERR_CONTROL_DIR, "");
   } else {
      /* Clean up and close the directory */
      closedir(control_dir);
   }
   
   /* Safely copy the path to the backlight control directory into the buffer
      and append the subpath of the file we are going to read */
   strlcpy(path, backlight_path, MAX_PATH_LEN);
   strlcat(path, "/brightness", MAX_PATH_LEN + EXTRA_PATH_LEN);

   if(access(path, F_OK) != 0)
      /* The file specified does not exist */
      throw_error(ERR_FILE_ACCES, "brightness");

   /* Same as preceding example */
   strlcpy(path, backlight_path, MAX_PATH_LEN);
   strlcat(path, "/max_brightness", MAX_PATH_LEN + EXTRA_PATH_LEN);

   if(access(path, F_OK) != 0) 
      throw_error(ERR_FILE_ACCES, "max_brightness");
      
   return;
}

void validate_decrement(unsigned int reference_value) {

   /* Check that provided decrement value does not take the brightness below zero,
      accounting for values as percentages */
   
   if(values_as_percentages) {
      int current = (reference_value * 100) / maximum;
      if(current - (int) delta_brightness < 0) 
         throw_error(ERR_INVAL_OPT, "");
   } else {
      if((int) reference_value - (int) delta_brightness < 0) 
         throw_error(ERR_INVAL_OPT, "");
   }

   return;
}

void validate_increment(unsigned int reference_value) {

   /* Check that provided increment value does not take the brightness above the
      maximum permissable value, accounting for values as percentages */
   
   if(values_as_percentages) {
      unsigned int current = (reference_value * 100) / maximum;
      if(current + delta_brightness > 100) 
         throw_error(ERR_INVAL_OPT, "");
   } else {
      if(reference_value + delta_brightness > maximum) 
         throw_error(ERR_INVAL_OPT, "");
   }

   return;
}

void version() {

   /* Display program version information on stdout */
   
   printf("%s v%s\n", PROGRAM_NAME, PROGRAM_VERSION);
   puts("Copyright (C) 2016, 2017 multiplex'd <multiplexd@gmx.com>");
   printf("\
This is free software under the terms of the GNU General Public License, \n\
version 2 or later. You are free to use, modify and redistribute it, however \n\
there is NO WARRANTY; please see <https://gnu.org/licenses/gpl.html> for \n\
further information.\n");
}

void write_backlight_brightness() {

   /* Write the backlight brightness to the control file */
   
   unsigned int oldval;
   unsigned int val_to_write;
   unsigned int current;
   char* out_string_end;
   char* out_string_filler; /* Placeholder strings to format the program output nicely */

   /* Get the current value from the control file */
   current = get_value_from_file("/brightness");

   /* Set the values to be written and set the placeholder strings, accounting for
      values as percentages.*/
   if(values_as_percentages) {
      val_to_write = (brightness * maximum) / 100;
      oldval = (current * 100) / maximum;
      out_string_end = "%.";
      out_string_filler = "% ";
   } else {
      val_to_write = brightness;
      oldval = current;
      out_string_end = ".";
      out_string_filler = " ";
   }

   /* Actually perform the write to the file. */
   write_brightness_to_file(val_to_write);

   /* Display an informational message on stdout*/
   printf("Changed backlight brightness: %d%s=> %d%s\n", oldval, out_string_filler, brightness, out_string_end);      

   return;
}

void write_brightness_to_file(unsigned int bright) {

   /* Write a backlight brightness value to the control file */

   FILE* brightness_file;
   char path[MAX_PATH_LEN + EXTRA_PATH_LEN]; /* Buffer for holding the path to the file that is being written to*/

   /* Safely copy the path to the backlight control directory into the buffer
      and append the subpath of the file we are going to read */
   strlcpy(path, backlight_path, MAX_PATH_LEN);
   strlcat(path, "/brightness", MAX_PATH_LEN + EXTRA_PATH_LEN);

   /* Try to open the backlight file for writing */
   brightness_file = fopen(path, "w");
   if(brightness_file == NULL)
      /* Something went wrong when opening the file */
      throw_error(ERR_OPEN_FILE, "brightness");

   if(fprintf(brightness_file, "%d", bright) < 0)
      /* Something went wrong when writing to the file */
      throw_error(ERR_WRITE_FILE, "");

   /* Tidy up and close the file */
   fclose(brightness_file);

   return;
}


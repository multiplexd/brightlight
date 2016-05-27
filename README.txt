##########################
    brightlight v3-rc1
##########################

WARNING: This software version is a RELEASE CANDIDATE - it may not be stable.

Overview
========

This is brightlight, version v3-rc1. brightlight is a program that can get and
set the screen backlight brightness on Linux systems using the kernel sysfs 
interface. I wrote this program because the backlight keys on my laptop's 
keyboard didn't work after I installed Linux on it, and the bash script I was 
using to perform the task wasn't very flexible nor portable.

This program requires libbsd or a BSD-compatible implementation of strlcpy() 
and strlcat() (see the comment at the top of the source code).


Installation
============

brightlight can be compiled with:

  $ gcc -o brightlight brightlight.c -lbsd

(assuming you use gcc of course; clang works as well). Then, place the 
resulting binary somewhere in your $PATH.

You can change the path #defined by the preprocessor macro BACKLIGHT_PATH to 
suit your own system. It must point to the directory where the files 
'brightness' and 'max_brightness' reside. On my laptop, for example, this path 
is "/sys/class/backlight/intel_backlight/", however yours may be different. 
The compile-time default can always be overridden using the -f flag (see below).


Usage
=====

Options:

  -v, --version          Print program version and exit.
  -h, --help             Show this help message.
  -p, --percentage       Read or write the brightness level as a percentage 
                         (0 to 100) instead of the internal scale the kernel 
                         uses (such as e.g. 0 to 7812).
  -r, --read             Read the backlight brightness level.
  -w, --write <val>      Set the backlight brightness level to <val>, where 
                         <val> is a positive integer.
  -i, --increment <val>  Increment/increase the backlight brightness level by
      --increase <val>   <val>, where <val> is a positive integer.
  -d, --decrement <val>  Decrement/decrease the backlight brightness level by
      --decrease <val>   <val>, where <val> is a positive integer.
  -f, --file <path>      Specify alternative path to backlight control 
                         directory. This is likely to be a subdirectory under
                         "/sys/class/backlight/". Must be an absolute path 
                         with a trailing slash.
  -m, --maximum          Show maximum brightness level of the screen 
                         backlight on the kernel's scale. The compile-time 
                         default control directory is used if -f or --file is
                         not specified. The -p and --percentage flags are 
                         ignored when this option is specified.

The flags -r, -w, -m, -i, -d and their corresponding long options are mutually
exclusive however one of them is required.


Changelog:
==========

v1, 07/02/2016 - First version of brighlight.
v2, 26/04/2016 - Tidied up program internals, added new operations
                 and long options.


License
=======

Copyright (C) 2016 multiplex'd <multiplexd@gmx.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
file LICENSE for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

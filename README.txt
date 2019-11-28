######################
    brightlight v8
######################

Overview
========

This is brightlight, version v8. brightlight is a program that can get and
set the screen backlight brightness on Linux systems using the kernel sysfs 
interface. I wrote this program because the backlight keys on my laptop's 
keyboard didn't work after I installed Linux on it, and the bash script I was 
using to perform the task wasn't very flexible nor portable.

This program requires libbsd or a BSD-compatible implementation of strtonum().

Please note that versions less than or equal to 5 are licenced under the GPL,
version 2 or later, while versions 6 and later are licenced under an ISC-like
licence.

Installation
============

Fedora
------

A package is available in Fedora 25 or later.

  sudo dnf install brightlight

Source
------

brightlight can be compiled by issuing 'make'. Compiler options can be changed
by editing the Makefile. The default compiler is gcc, however clang works too.
You can then place the resulting binary somewhere in your $PATH. The default
linker flags link libbsd for the strtonum() function, however it is possible to
obtain the strtonum() source seperately and link it into the final executable.

You can change the path #defined by the preprocessor macro DEFAULT_CTRL_DIR to 
suit your own system. It must point to the directory where the files 
'brightness' and 'max_brightness' reside. On my laptop, for example, this path 
is "/sys/class/backlight/intel_backlight/", however yours may be different. 
The compile-time default can always be overridden using the -f flag (see below).

By default, the files in sysfs are owned by user and group root, so the
brightness may only be set by root (though any user may read it). One way around
this would be to install brightlight setuid root, however this is not
recommended, as a regular user could use the -f option to read and write files
owned by other users. It is, however, possible to change the permissions on these
files in sysfs, which could be set at boot time using the "anacron" functionality
in some cron implementations, systemd-tmpfiles or /etc/rc.local to a group which
would permit a controlled subset of users to change the backlight brightness
without the requirement for a setuid root executable.

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
                           <val>, where <val> is a positive integer.
        --increase <val>   Same as --increment.
    -d, --decrement <val>  Decrement/decrease the backlight brightness level by
                           <val>, where <val> is a positive integer.
        --decrease <val>   Same as --decrement.
    -f, --file <path>      Specify alternative path to backlight control
                           directory. This is likely to be a subdirectory under
                           "/sys/class/backlight/".
    -m, --maximum          Show maximum brightness level of the screen
                           backlight on the kernel's scale. The compile-time
                           default control directory is used if -f or --file is
                           not specified. The -p and --percentage flags are
                           ignored when this option is specified.

The flags -r, -w, -m, -i, -d and their corresponding long options are mutually
exclusive. If none of these flags are explicitly specified then -r is the
default.

Changelog:
==========

v1, 07/02/2016 - First version of brighlight.
v2, 26/04/2016 - Tidied up program internals, added new operations
                 and long options.
v3, 31/07/2016 - Tidied up program logic and internals, added build system.
v4, 08/01/2017 - Minor changes to fix bugs found by clang static analyzer.
v5, 21/10/2017 - Tweak to the Makefile, contributed by Igor Gnatenko.
v6, 25/07/2018 - From-scratch rewrite, relicence under an ISC licence.
v7, 08/10/2018 - Bugfix release.
v8, 28/11/2019 - Make the read operation the default when none is otherwise
                 specified.

License
=======

See LICENSE.txt.

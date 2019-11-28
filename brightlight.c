/* brightlight v8 - change backlight brightness on Linux systems
 * Copyright (C) 2016, 2017, 2018, 2019 multiplexd <multi@in-addr.xyz>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Requires the BSD libc function strtonum(). Either add '-lbsd' to the linker
 * flags or get a copy of the strtonum() source file and link it into the final
 * executable.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PROGNAM "brightlight"
#define PROGVER 8

#define DEFAULT_CTRL_DIR  "/sys/class/backlight/intel_backlight"
#define MAXBRIGHT         "max_brightness"
#define CURBRIGHT         "brightness"

#define OP_READ 0x01
#define OP_WRIT 0x02
#define OP_INCR 0x04
#define OP_DECR 0x08
#define OP_MAX  0x10

#define rounded_div(a, b) ((a + b / 2) / b)

extern long long strtonum(const char *, long long, long long, const char **);

void usage(void) {
    printf("Usage: brightlight [OPTIONS]\n"
           "Options:\n\n"
           "    -v, --version          Print program version and exit.\n"
           "    -h, --help             Show this help message.\n"
           "    -p, --percentage       Read or write the brightness level as a percentage\n"
           "                           (0 to 100) instead of the internal scale the kernel\n"
           "                           uses (such as e.g. 0 to 7812).\n"
           "    -r, --read             Read the backlight brightness level.\n"
           "    -w, --write <val>      Set the backlight brightness level to <val>, where\n"
           "                           <val> is a positive integer.\n"
           "    -i, --increment <val>  Increment/increase the backlight brightness level by\n"
           "                           <val>, where <val> is a positive integer.\n"
           "        --increase <val>   Same as --increment.\n"
           "    -d, --decrement <val>  Decrement/decrease the backlight brightness level by\n"
           "                           <val>, where <val> is a positive integer.\n"
           "        --decrease <val>   Same as --decrement.\n"
           "    -f, --file <path>      Specify alternative path to backlight control\n"
           "                           directory. This is likely to be a subdirectory under\n"
           "                           \"/sys/class/backlight/\".\n"
           "    -m, --maximum          Show maximum brightness level of the screen\n"
           "                           backlight on the kernel's scale. The compile-time\n"
           "                           default control directory is used if -f or --file is\n"
           "                           not specified. The -p and --percentage flags are\n"
           "                           ignored when this option is specified.\n\n"
           "The flags -r, -w, -m, -i, -d and their corresponding long options are mutually\n"
           "exclusive. If none of these flags are explicitly specified then -r is the\n"
           "default.\n");
}

void versioninfo(void) {
    printf("%s, v%u\n"
           "Copyright (C) 2016, 2017, 2018, 2019 multiplexd <multi@in-addr.xyz>\n\n"
           "This is free software, licenced under an ISC-like licence. You are\n"
           "free to use, redistribute and modify this software as you wish, but\n"
           "there is NO WARRANTY.\n", PROGNAM, PROGVER);
}

void checkargs(uint8_t *flags) {
    uint8_t num = 0;

    for (int i = 0; i < 8; i++)
        num += (*flags >> i) % 2;

    if (num > 1)
        errx(1, "conflicting arguments (pass '-h' for help)");
    else if (num == 0)
        *flags = OP_READ;

    return;
}

uint32_t read_file(int dirfd, char *path) {
    char buf[200], *p;
    int fd;
    ssize_t rv;
    uint32_t ret;
    const char *errstr;

    if ((fd = openat(dirfd, path, O_RDONLY)) < 0)
        err(1, "could not open \"%s\" file", path);

    p = buf;
    while ((rv = read(fd, p, sizeof(buf) - (p - buf))) != 0) {
        if (rv == -1) {
            if (errno == EINTR || errno == EAGAIN)
                continue;

            err(1, "could not read from \"%s\" file", path);
        }

        p += rv;
    }

    if (close(fd) < 0)
        warn("could not close file descriptor to \"%s\" file (continuing)", path);

    p = buf;
    while (p < (buf + sizeof(buf) - 1) && isdigit(*p))
        p++;

    *p = '\0';

    ret = strtonum(buf, 0, UINT32_MAX, &errstr);
    if (errstr)
        errx(1, "invalid value \"%s\" found in \"%s\" file", buf, path);

    return ret;
}

void write_file(int dirfd, char *path, uint32_t val) {
    char buf[200], *p;
    int fd;
    ssize_t rv;

    if (snprintf(buf, sizeof(buf), "%" PRIu32 "\n", val) >= sizeof(buf))
        errx(1, "could not format output buffer");
    
    if ((fd = openat(dirfd, path, O_RDWR)) < 0)
        err(1, "could not open \"%s\" file", path);

    p = buf;
    while ((rv = write(fd, p, sizeof(buf) - (p - buf))) != 0) {
        if (rv == -1) {
            if (errno == EINTR || errno == EAGAIN)
                continue;

            err(1, "could not write to \"%s\" file", path);
        }

        p += rv;
    }

    if (close(fd) < 0)
        warn("could not close file descriptor to \"%s\" file (continuing)", path);

    return;
}

int main(int argc, char *argv[]) {
    uint8_t flags = 0;
    uint32_t delta = 0;
    uint32_t max, current, newb = 0;
    uint8_t pflag = 0;
    char *ctrldir = NULL;
    char *numarg = NULL;
    int ch, dirfd;
    const char *errstr;

    struct option longopts[] = {
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
        {0, 0, 0, 0}
    };

    while ((ch = getopt_long(argc, argv, "+:d:f:hi:mprvw:", longopts, NULL)) != -1) {
        switch (ch) {
        case 'd':
            flags |= OP_DECR;
            numarg = optarg;
            break;
        case 'f':
            ctrldir = optarg;
            break;
        case 'h':
            usage();
            exit(0);
            break;
        case 'i':
            flags |= OP_INCR;
            numarg = optarg;
            break;
        case 'm':
            flags |= OP_MAX;
            break;
        case 'p':
            pflag = 1;
            break;
        case 'r':
            flags |= OP_READ;
            break;
        case 'v':
            versioninfo();
            exit(0);
            break;
        case 'w':
            flags |= OP_WRIT;
            numarg = optarg;
            break;
        case '?':
            errx(1, "invalid option: '%c'", optopt);
            break;
        default:
            errx(1, "error parsing options");
            break;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc != 0)
        errx(1, "too many arguments (pass '-h' for help)");

    checkargs(&flags);

    if (ctrldir == NULL) {
        ctrldir = DEFAULT_CTRL_DIR;
    }

    if (strlen(ctrldir) > PATH_MAX)
        errx(1, "path to control directory exceeds maximum path length");
    
    if (flags & (OP_WRIT | OP_INCR | OP_DECR)) {
        delta = strtonum(numarg, 0, UINT32_MAX, &errstr);
        if (errstr)
            errx(1, "invalid numeric argument: %s", errstr);
    }

    if ((dirfd = open(ctrldir, O_RDONLY | O_DIRECTORY)) == -1)
        err(1, "could not open control directory \"%s\"", ctrldir);

    max = read_file(dirfd, MAXBRIGHT);

    if (flags & OP_MAX) {
        printf("maximum backlight brightness is: %" PRIu32 "\n", max);
        goto end;
    }

    current = read_file(dirfd, CURBRIGHT);

#define current_pct() (rounded_div(current * 100, max))
#define maybe_pct()   (pflag ? "%" : "")
    
    if (flags & OP_READ) {
        printf("current backlight brightness is: %" PRIu32 "%s\n",
               (pflag ? current_pct() : current), maybe_pct());
        goto end;
    }

    switch (flags) {
    case OP_INCR:
        if (pflag ? (current_pct() + delta > 100) : (current + delta > max))
            errx(1, "invalid increment: %" PRIu32 "%s", delta, maybe_pct());

        newb = (pflag ? current + rounded_div(delta * max, 100) : current + delta);
        break;
    case OP_DECR:
        if (pflag ? (current_pct() < delta) : (current < delta))
            errx(1, "invalid decrement: %" PRIu32"%s", delta, maybe_pct());

        newb = (pflag ? current - rounded_div(delta * max, 100) : current - delta);
        break;
    case OP_WRIT:
        /* the underflow case is handled by strtonum above */
        if (pflag ? (delta > 100) : (delta > max))
            errx(1, "invalid argument: %" PRIu32 "%s", delta, maybe_pct());

        newb = (pflag ? rounded_div(delta * max, 100) : delta);
        break;
    }

    write_file(dirfd, CURBRIGHT, newb);

    printf("changed backlight brightness: %" PRIu32 "%s => %" PRIu32 "%s\n",
           (pflag ? current_pct() : current), maybe_pct(),
           (pflag ? rounded_div(newb * 100, max) : newb), maybe_pct());

end:
    if (close(dirfd) < 0)
        warn("could not close control directory file descriptor");
    
    exit(0);

#undef current_pct
#undef maybe_pct
}

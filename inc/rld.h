/* ----------------------------------------------------------------------- */
/*  This file is part of rld.                                              */
/* ----------------------------------------------------------------------- */
/*  Copyright (c) 2000 stfsux <stfsux@tuxfamily.org>                       */
/*  This work is free. You can redistribute it and/or modify it under the  */
/*  terms of the Do What The Fuck You Want To Public License, Version 2,   */
/*  as published by Sam Hocevar. See the COPYING file for more details.    */
/* ----------------------------------------------------------------------- */

#ifndef __RLD_H_
#define __RLD_H_

extern const char * __progname;

#define __RLD_VERSION "rape'n'load linker v0.001 (experimental) for GNU/Linux\n"\
                      "This program is free software; you may redistribute it under the terms of\n"\
                      "the Don't be a Dick Public Licence.\n"\
                      "This program has absolutely no warranty.\n"

#define __RLD_USAGE   "rld [options] file...\n"\
                      "Options:\n"\
                      "-A ARCH, --architecture ARCH\n"\
                      "                            Set a specific architecture\n"\
                      "--compress                  Pack the elf output with lzma\n"\
                      "-h, --help                  Print this help\n"\
                      "-L DIRECTORY, --library-path DIRECTORY\n"\
                      "                            Add a DIRECTORY to library search path.\n"\
                      "-l LIBNAME, --library LIBNAME\n"\
                      "                            Search for library LIBNAME\n"\
                      "-o FILE, --output FILE      Set output file name\n"\
                      "--verbose                   Output lots of bullshit during link\n"\
                      "-v, --version               Print version information\n"\
                      "\n"\
                      "Troll or report bugs to <stfsux[at]tuxfamily{dot}org>\n"

#define __RLD_PLATFORM_X86      0
#define __RLD_PLATFORM_AMD64    1
#define __RLD_PLATFORM_ARM      2

#if defined(__i386)
#define __RLD_DEFAULT_PLATFORM  __RLD_PLATFORM_X86
#elif defined(__amd64)
#define __RLD_DEFAULT_PLATFORM __RLD_PLATFORM_AMD64
#error "Architecture amd64 is currently not supported."
#elif defined(__arm)
#define __RLD_DEFAULT_PLATFORM __RLD_PLATFORM_ARM
#error "Architecture arm is currently not supported."
#endif

#define _rld_set_bit(x,n)     (x=x|(1<<n))
#define _rld_clr_bit(x,n)     (x=x&(~(1<<n)))
#define _rld_bit_is_set(x,n)  (x&(1<<n))
#define _rld_bit_is_clr(x,n)  (!(x&(1<<n)))

#endif


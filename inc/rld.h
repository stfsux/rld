
#ifndef __RLD_H_
#define __RLD_H_

extern const char * __progname;

#define __RLD_VERSION "rape'n'load linker v0.1 for GNU/Linux\n"\
                      "This program is free software; you may redistribute it under the terms of\n"\
                      "the Don't be a Dick Public Licence.\n"\
                      "This program has absolutely no warranty.\n"

#define __RLD_USAGE   "rld [options] file...\n"\
                      "Options:\n"\
                      "-A ARCH, --architecture ARCH\n"\
                      "-h, --help                  Print this help\n"\
                      "                            Set a specific architecture\n"\
                      "-L DIRECTORY, --library-path DIRECTORY\n"\
                      "                            Add a DIRECTORY to library search path.\n"\
                      "-l LIBNAME, --library LIBNAME\n"\
                      "                            Search for library LIBNAME\n"\
                      "-o FILE, --output FILE      Set output file name\n"\
                      "--verbose                   Output lots of bullshit during link\n"\
                      "-v, --version               Print version information\n"\
                      "\n"\
                      "Troll or report bugs to <stfsux[at]tuxfamily{dot}org>\n"
#endif



PROJECT			= rld

PREFIX		= /usr

CC				= gcc
AS_X86		= as
MAKE      = make
SRCS			= $(wildcard src/*.c)
SRCS			+= $(wildcard src/platform/*.c)
SRCSRT		= $(wildcard src/runtime/*.asm) # runtime source files
OBJSRT		= $(SRCSRT:.asm=.o)							# runtime object files
OBJS			= $(SRCS:.c=.o)

PLATFORM_X86	= yes
PLATFORM_X64	= no
PLATFORM_ARM	= no
PLATFORM_PPC	= no

DEBUG			= yes

CFLAGS			= -I./inc -Wall -Wshadow -Wextra -std=c99 -D_BSD_SOURCE
LDFLAGS			= 

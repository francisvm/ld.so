CC=/home/francisvm/Projects/llvm/build/bin/clang
CFLAGS=-std=c99 -pedantic -Wall -Wextra -ffreestanding -fPIC -fvisibility=hidden -g
CXX=/home/francisvm/Projects/llvm/build/bin/clang++
CXXFLAGS=-std=c++1z -stdlib=libc++ -pedantic -Wall -Wextra -ffreestanding \
	 -fno-exceptions -fno-rtti -fPIC -fvisibility=hidden -g
CPPFLAGS=-MMD -Iinclude
LDFLAGS=-nostdlib -shared

LIB=ld.so
SRC=main.cc    \
    env.cc     \
    dynamic.cc \
    dso.cc
CPPSRC=vendor/hash.cpp
CSRC=vendor/ceilf.c
ASM=crt1.S

OBJ=${SRC:.cc=.o} ${CPPSRC:.cpp=.o} ${CSRC:.c=.o} ${ASM:.S=.o}

all: ${LIB}

${LIB}: ${OBJ}
	${CXX} $^ -o $@ ${LDFLAGS}

# Track makefile modifications.
${OBJ}: Makefile

-include *.d

CC=/home/francisvm/Projects/llvm/build/bin/clang
CFLAGS=-std=c99 -pedantic -Wall -Wextra -ffreestanding -fPIC -fvisibility=hidden -g
CXX=/home/francisvm/Projects/llvm/build/bin/clang++
CXXFLAGS=-std=c++1z -stdlib=libc++ -pedantic -Wall -Wextra -ffreestanding \
	 -fno-exceptions -fno-rtti -fPIC -fvisibility=hidden -g
CPPFLAGS=-MMD -Iinclude
LDFLAGS=-nostdlib -shared
LDLIBS=-lc++abi

LIB=ld.so
SRC=main.cc \
    env.cc  \
    dynamic.cc

ASM=crt1.S
OBJ=${SRC:.cc=.o} ${ASM:.S=.o}

all: ${LIB}

${LIB}: ${OBJ}
	${CXX} $^ -o $@ ${LDFLAGS} ${LDLIBS}

# Track makefile modifications.
${OBJ}: Makefile

-include *.d

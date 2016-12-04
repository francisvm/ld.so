#pragma once

#include <asm/unistd.h>
#include <cstddef>
#include <detail/syscall.hh>
#include <fcntl.h>
#include <sys/mman.h>

namespace ldso::sys {

static inline auto _exit = detail::syscall<__NR_exit, void(int)>{};
static inline auto open = detail::syscall<__NR_open, int(const char *, int)>{};
static inline auto mmap =
    detail::syscall<__NR_mmap, void *(void *, size_t, int, int, int, off_t)>{};
static inline auto munmap = detail::syscall<__NR_munmap, int(void *, size_t)>{};
static inline auto fstat =
    detail::syscall<__NR_fstat, int(int, struct stat *)>{};
static inline auto write =
    detail::syscall<__NR_write, ssize_t(int, const void *, size_t)>{};
static inline auto mprotect =
    detail::syscall<__NR_mprotect, int(void *, size_t, int)>{};

} // namespace ldso::sys

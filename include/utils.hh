#pragma once

#include <syscall.hh>
#include <unistd.h>

namespace ldso {

[[noreturn]] static inline void unreachable_fail(const char *msg, size_t len) {
  sys::write(STDERR_FILENO, msg, len);
  sys::_exit(1);
  while (1)
    ; // Tell the compiler that this function doesn't return.
}

#define unreachable_msg(Message)                                               \
  ::ldso::unreachable_fail(Message, __builtin_strlen(Message))

#define unreachable_line(Message, Line, File)                                  \
  unreachable_msg("unreachable code reached: " Message " (" File ":" #Line     \
                  ")\n")

#define unreachable_internal(Message, Line, File)                              \
  unreachable_line(Message, Line, File)

#define unreachable(Message) unreachable_internal(Message, __LINE__, __FILE__)

#define ldso_assert(Cond)                                                      \
  do {                                                                         \
    if (!(Cond))                                                               \
      unreachable("Assert failed");                                            \
  } while (false)

template <typename T> struct mapped_file {
  mapped_file(const char *filename) {
    auto fd = sys::open(filename, O_RDONLY);
    if (fd < -1)
      unreachable("unable to open file");

    struct stat s {};
    if (sys::fstat(fd, &s) < 0)
      unreachable("unable to stat file");

    size = s.st_size;

    file =
        static_cast<T *>(sys::mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (file == MAP_FAILED)
      unreachable("unable to mmap file");
  }

  mapped_file(mapped_file &&other) {
    file = other.file;
    size = other.size;
    other.file = nullptr;
    other.size = 0;
  }

  ~mapped_file() {
    if (sys::munmap(const_cast<void *>(reinterpret_cast<const void *>(file)),
                    size) < 0)
      unreachable("munmap failed");
  }

  T *file = nullptr;
  size_t size = 0;
};

template <int fd, size_t N> constexpr void print(const char (&a)[N]) {
  sys::write(fd, a, N);
}

} // namespace ldso

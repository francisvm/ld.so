#pragma once

#include <error.hh>
#include <syscall.hh>
#include <unistd.h>

namespace ldso {

template <typename T> struct mapped_file {
  mapped_file(const char * filename) {
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

} // namespace ldso

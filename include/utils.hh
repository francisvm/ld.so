#pragma once

#include <stl.hh>
#include <syscall.hh>
#include <unistd.h>

namespace ldso {

template <int fd, size_t N> constexpr void print(const char (&a)[N]) {
  sys::write(fd, a, N);
}

template <int fd> void print(string_view str) {
  sys::write(fd, str.data(), str.size());
}

} // namespace ldso

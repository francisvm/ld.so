#pragma once

#include <syscall.hh>
#include <unistd.h>

namespace ldso {

template <size_t N>
[[noreturn]] static void unreachable_fail(const char (&a)[N]) {
  sys::write(STDERR_FILENO, a, N);
  sys::_exit(1);
  while (1)
    ; // Tell the compiler that this function doesn't return.
}

#define unreachable_msg(Message) ::ldso::unreachable_fail(Message)

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

} // namespace ldso

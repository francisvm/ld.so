#pragma once

#include <cstdint>
#include <experimental/type_traits>
#include <type_traits>

namespace ldso::detail {

template <typename T>
static constexpr auto is_void = std::experimental::is_void_v<T>;

template <typename T>
using or_int_t = std::conditional_t<is_void<T>, uint64_t, T>;

template <auto Nb, typename> struct syscall;

template <auto Nb, typename Return, typename A1, typename A2, typename A3,
          typename A4, typename A5, typename A6>
struct syscall<Nb, Return(A1, A2, A3, A4, A5, A6)> {
  Return operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) {
    register A4 r10 asm("r10") = a4;
    register A5 r8 asm("r8") = a5;
    register A6 r9 asm("r9") = a6;
    or_int_t<Return> ret;

    __asm__ __volatile__("syscall\n"
                         : "=a"(ret)
                         : "a"(Nb), "D"(a1), "S"(a2), "d"(a3), "r"(r10),
                           "r"(r8), "r"(r9)
                         : "rcx", "r11");
    if constexpr (!is_void<Return>)
      return ret;
  }
};

template <auto Nb, typename Return, typename A1, typename A2, typename A3,
          typename A4, typename A5>
struct syscall<Nb, Return(A1, A2, A3, A4, A5)> {
  Return operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    register A4 r10 asm("r10") = a4;
    register A5 r8 asm("r8") = a5;
    or_int_t<Return> ret;

    __asm__ __volatile__("syscall\n"
                         : "=a"(ret)
                         : "a"(Nb), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8)
                         : "rcx", "r11");
    if constexpr (!is_void<Return>)
      return ret;
  }
};

template <auto Nb, typename Return, typename A1, typename A2, typename A3,
          typename A4>
struct syscall<Nb, Return(A1, A2, A3, A4)> {
  Return operator()(A1 a1, A2 a2, A3 a3, A4 a4) {
    register A4 r10 asm("r10") = a4;
    or_int_t<Return> ret;

    __asm__ __volatile__("syscall\n"
                         : "=a"(ret)
                         : "a"(Nb), "D"(a1), "S"(a2), "d"(a3), "r"(r10)
                         : "rcx", "r11");
    if constexpr (!is_void<Return>)
      return ret;
  }
};

template <auto Nb, typename Return, typename A1, typename A2, typename A3>
struct syscall<Nb, Return(A1, A2, A3)> {
  Return operator()(A1 a1, A2 a2, A3 a3) {
    or_int_t<Return> ret;

    __asm__ __volatile__("syscall\n"
                         : "=a"(ret)
                         : "a"(Nb), "D"(a1), "S"(a2), "d"(a3)
                         : "rcx", "r11");
    if constexpr (!is_void<Return>)
      return ret;
  }
};

template <auto Nb, typename Return, typename A1, typename A2>
struct syscall<Nb, Return(A1, A2)> {
  Return operator()(A1 a1, A2 a2) {
    or_int_t<Return> ret;

    __asm__ __volatile__("syscall\n"
                         : "=a"(ret)
                         : "a"(Nb), "D"(a1), "S"(a2)
                         : "rcx", "r11");
    if constexpr (!is_void<Return>)
      return ret;
  }
};

template <auto Nb, typename Return, typename A1>
struct syscall<Nb, Return(A1)> {
  Return operator()(A1 a1) {
    or_int_t<Return> ret;
    __asm__ __volatile__("syscall\n"
                         : "=a"(ret)
                         : "a"(Nb), "D"(a1)
                         : "rcx", "r11");

    if constexpr (!is_void<Return>)
      return ret;
  }
};

template <auto Nb, typename Return> struct syscall<Nb, Return()> {
  Return operator()() {
    or_int_t<Return> ret;

    __asm__ __volatile__("syscall\n" : "=a"(ret) : "a"(Nb) : "rcx", "r11");
    if constexpr (!is_void<Return>)
      return ret;
  }
};
} // namespace ldso::detail

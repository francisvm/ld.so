#pragma once

#include <cstring>
#include <elf.h>
#include <mapped_file.hh>
#include <stl.hh>
#include <utils.hh>

namespace ldso::elf {

template <typename Dst, typename Src, typename Size>
static inline std::enable_if_t<!std::experimental::is_same_v<Src, Elf64_Ehdr>,
                               Dst *>
get(Src *ptr, Size off) {
  if constexpr (std::experimental::is_const_v<Dst>)
    return reinterpret_cast<Dst *>(reinterpret_cast<const char *>(ptr) + off);
  else
    return reinterpret_cast<Dst *>(reinterpret_cast<char *>(ptr) + off);
}

template <typename Dst, typename Src, typename Size>
static inline std::enable_if_t<std::experimental::is_same_v<Src, Elf64_Ehdr>,
                               Dst *>
get(Src *ehdr, Size off) {
  ldso_assert(strcmp(reinterpret_cast<char *>(ehdr->e_ident), ELFMAG));
  if constexpr (std::experimental::is_const_v<Dst>)
    return reinterpret_cast<Dst *>(reinterpret_cast<const char *>(ehdr) + off);
  else
    return reinterpret_cast<Dst *>(reinterpret_cast<char *>(ehdr) + off);
}

inline array_view<const Elf64_Phdr>
get_phdrs(const ldso::mapped_file<const Elf64_Ehdr> &file) {
  auto *ehdr = file.file;
  return {get<const Elf64_Phdr>(ehdr, ehdr->e_phoff), ehdr->e_phnum};
}

ldso::array_view<const Elf64_auxv_t> auxv(const void **argv);

ldso::array_view<const char *> envp(const void **argv);

ldso::array_view<const Elf64_Phdr>
auxv_phdrs(ldso::array_view<const Elf64_auxv_t> auxvs);

} // namespace ldso::elf

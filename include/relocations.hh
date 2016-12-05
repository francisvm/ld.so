#pragma once

#include <stl.hh>

namespace ldso {

template <typename T> void relocate(T &elf) {
  auto jmprel = std::find_if(elf.dyns.begin(), elf.dyns.end(),
                             [](auto &dyn) { return dyn.d_tag == DT_JMPREL; });
  auto pltrelsz = std::find_if(elf.dyns.begin(), elf.dyns.end(), [](auto &dyn) {
    return dyn.d_tag == DT_PLTRELSZ;
  });
  auto ehdr = [&] {
    if constexpr (std::experimental::is_same_v<T, bin>)
      return static_cast<const Elf64_Ehdr *>(nullptr);
    else
      return elf.file.file;
  }();
  array_view<const Elf64_Rela> rels{
      elf::get<const Elf64_Rela>(ehdr, jmprel->d_un.d_val),
      pltrelsz->d_un.d_val / sizeof(Elf64_Rela)};

  for (auto& rel : rels) {
    auto& sym = elf.symtab[ELF64_R_SYM(rel.r_info)];
    auto* name = elf.strtab + sym.st_name;
    for (const dso *dep : elf.deps) {
      if (auto sym = dep->symbol(name)) {
        *const_cast<uint64_t *>(
            reinterpret_cast<const uint64_t *>(rel.r_offset)) = (uint64_t)sym;
      }
    }
  }
}

} // namespace ldso

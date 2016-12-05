#pragma once

#include <dso.hh>
#include <stl.hh>

namespace ldso {

void relocate(uint8_t *, uint8_t, sym_t);

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
      if (auto found = dep->symbol(name)) {
        auto dst = [&] {
          if constexpr (std::experimental::is_same_v<T, bin>)
            return const_cast<uint8_t *>(
                reinterpret_cast<const uint8_t *>(rel.r_offset));
          else
            return reinterpret_cast<uint8_t*>(dep->segments + rel.r_offset);
        }();
        relocate(dst, ELF64_R_TYPE(rel.r_info), found);
      }
    }
  }
}

} // namespace ldso

#include <dynamic.hh>
#include <elf.hh>
#include <stl.hh>

namespace ldso::elf::dynamic {

array_view<const Elf64_Dyn> get_dyns(const Elf64_Ehdr *ehdr,
                                     const Elf64_Off off) {
  auto b = get<const Elf64_Dyn>(ehdr, off);
  auto e = std::find_if(b, reinterpret_cast<const Elf64_Dyn *>(-1),
                        [](auto &dyn) { return dyn.d_tag == DT_NULL; });

  return {b, e};
}

array_view<const Elf64_Sym> get_symtab(const Elf64_Ehdr *ehdr,
                                       const Elf64_Off off) {
  // .dynsym is not mapped in ehdr found in the auxv.
  // Don't look for the size, let's suppose the .dynsym is infinite.
  if (ehdr->e_type == ET_EXEC)
    return {reinterpret_cast<const Elf64_Sym *>(off), static_cast<size_t>(-1)};

  array_view<const Elf64_Shdr> shdrs{get<const Elf64_Shdr>(ehdr, ehdr->e_shoff),
                                     ehdr->e_shnum};
  auto dynsym = std::find_if(shdrs.begin(), shdrs.end(), [&](auto &shdr) {
    return shdr.sh_type == SHT_DYNSYM;
  });

  assert(dynsym != shdrs.end() && ".dynsym section not found");

  auto b = get<const Elf64_Sym>(ehdr, off);

  return {b, dynsym->sh_size / sizeof(Elf64_Sym)};
}

} // namespace ldso::dynamic

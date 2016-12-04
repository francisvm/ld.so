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
  auto b = get<const Elf64_Sym>(ehdr, off);
  auto e = std::find_if(std::next(b), reinterpret_cast<const Elf64_Sym *>(-1),
                        [](auto &sym) { return sym.st_info == 0; });

  return {b, e};
}

} // namespace ldso::dynamic

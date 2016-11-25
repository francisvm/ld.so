#include <dynamic.hh>
#include <elf.hh>
#include <stl.hh>

namespace ldso::elf::dynamic {

ldso::array_view<const Elf64_auxv_t> auxv(const void **argv) {
  while (*argv) // skip argv.
    ++argv;
  ++argv;
  while (*argv) // skip envp.
    ++argv;
  ++argv;
  auto begin = reinterpret_cast<const Elf64_auxv_t *>(argv);
  auto end = std::find_if(begin, reinterpret_cast<const Elf64_auxv_t *>(-1),
                          [](auto &auxv) { return auxv.a_type == AT_NULL; });

  return {begin, end};
}

ldso::array_view<const Elf64_Phdr>
auxv_phdrs(ldso::array_view<const Elf64_auxv_t> auxvs) {
  auto phnum = std::find_if(auxvs.begin(), auxvs.end(), [](auto &auxv) {
                 return auxv.a_type == AT_PHNUM;
               })->a_un.a_val;

  auto phdr = std::find_if(auxvs.begin(), auxvs.end(), [](auto &auxv) {
                return auxv.a_type == AT_PHDR;
              })->a_un.a_val;
  return { reinterpret_cast<const Elf64_Phdr*>(phdr), phnum };
}

ldso::array_view<const Elf64_Dyn> get_dyns(const Elf64_Ehdr *ehdr,
                                           const Elf64_Off off) {
  auto b = get<const Elf64_Dyn>(ehdr, off);
  auto e = std::find_if(b, reinterpret_cast<const Elf64_Dyn *>(-1),
                        [](auto &dyn) { return dyn.d_tag == DT_NULL; });

  return {b, e};
}
} // namespace ldso::dynamic

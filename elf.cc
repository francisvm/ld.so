#include <elf.hh>
#include <stl.hh>

namespace ldso::elf {

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

ldso::array_view<const char *> envp(const void **argv) {
  while (*argv) // skip argv.
    ++argv;
  ++argv;
  ldso::array_view<string_view> res;
  auto begin = reinterpret_cast<const char **>(argv);
  auto end = std::find(begin, reinterpret_cast<const char **>(-1), nullptr);

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
  return {reinterpret_cast<const Elf64_Phdr *>(phdr), phnum};
}

} // namespace ldso::elf

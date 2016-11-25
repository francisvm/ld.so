#include <dynamic.hh>
#include <elf.hh>
#include <stl.hh>
#include <syscall.hh>
#include <utils.hh>

extern "C" void load(int argc, const char *argv[]) {
  (void)argc;

  auto auxvs = ldso::elf::dynamic::auxv(reinterpret_cast<const void **>(argv));
  auto phdrs = ldso::elf::dynamic::auxv_phdrs(auxvs);
  auto *ehdr = reinterpret_cast<const Elf64_Ehdr *>(
      reinterpret_cast<const char *>(phdrs.begin()) - sizeof(Elf64_Ehdr));
  auto found =
      std::find_if(phdrs.begin(), phdrs.end(), [](const Elf64_Phdr &phdr) {
        return phdr.p_type == PT_DYNAMIC;
      });
  assert(found != phdrs.end());

  auto dyns = ldso::elf::dynamic::get_dyns(ehdr, found->p_offset);
  auto strtab_off = std::find_if(dyns.begin(), dyns.end(), [](auto &dyn) {
    return dyn.d_tag == DT_STRTAB;
  });
  assert(strtab_off != dyns.end());
  auto strtab = reinterpret_cast<const char *>(strtab_off->d_un.d_ptr);

  ldso::vector<const Elf64_Dyn *> needed;
  ldso::transform_if(dyns.begin(), dyns.end(), std::back_inserter(needed),
                     [](auto &dyn) { return &dyn; },
                     [](auto &dyn) { return dyn.d_tag == DT_NEEDED; });

  for (auto *dyn : needed) {
    auto *str = strtab + dyn->d_un.d_val;
    ldso::sys::write(STDOUT_FILENO, str, strlen(str));
    ldso::sys::write(STDOUT_FILENO, "\n", 1);
  }

  ldso::sys::_exit(0);
}

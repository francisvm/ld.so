#include <dso.hh>
#include <dynamic.hh>
#include <elf.hh>
#include <functional>
#include <stl.hh>
#include <string>
#include <syscall.hh>
#include <utils.hh>

extern "C" void load(int argc, const char *argv[]) {
  (void)argc;

  auto auxvs = ldso::elf::dynamic::auxv(reinterpret_cast<const void **>(argv));
  auto phdrs = ldso::elf::dynamic::auxv_phdrs(auxvs);
  auto *ehdr = reinterpret_cast<const Elf64_Ehdr *>(
      reinterpret_cast<const char *>(phdrs.begin()) - sizeof(Elf64_Ehdr));

  ldso::unordered_set<ldso::dso> dsos;
  build_graph(ehdr, dsos);

  for (auto &dso : dsos) {
    ldso::sys::write(STDOUT_FILENO, dso.name.c_str(), dso.name.size());
    ldso::sys::write(STDOUT_FILENO, "\n", 1);
  }

  ldso::sys::_exit(0);
}

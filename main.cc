#include <dso.hh>
#include <dynamic.hh>
#include <elf.hh>
#include <env.hh>
#include <functional>
#include <relocations.hh>
#include <stl.hh>
#include <string>
#include <syscall.hh>
#include <utils.hh>

extern "C" void load(int argc, const char *argv[]) {
  ldso::exe.auxv = ldso::elf::auxv(reinterpret_cast<const void **>(argv));
  ldso::exe.envp = ldso::elf::envp(reinterpret_cast<const void **>(argv));

  auto phdrs = ldso::elf::auxv_phdrs(ldso::exe.auxv);
  auto *ehdr = reinterpret_cast<const Elf64_Ehdr *>(
      reinterpret_cast<const char *>(phdrs.begin()) - sizeof(Elf64_Ehdr));

  ldso::bin exe{ehdr};

  ldso::unordered_set<ldso::dso> dsos;
  build_graph(exe, dsos);

  auto i = dsos.begin()->symbol("foo");

  if (auto trace = ldso::get_env("LD_TRACE_LOADED_OBJECTS"); trace == "1") {
    for (auto &dso : dsos) {
      ldso::sys::write(STDOUT_FILENO, dso.name.c_str(), dso.name.size());
      ldso::sys::write(STDOUT_FILENO, "\n", 1);
    }
    ldso::sys::_exit(0);
  }

  relocate(exe);

  auto main =
      reinterpret_cast<int (*)(int, const char **, const char *const *)>(
          ehdr->e_entry);

  ldso::sys::_exit(main(argc, argv, ldso::exe.envp.data()));
}

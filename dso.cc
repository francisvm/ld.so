#include <dso.hh>
#include <dynamic.hh>
#include <elf.hh>

namespace ldso {

void build_graph(const Elf64_Ehdr *ehdr, ldso::unordered_set<dso> &dsos) {
  auto phb = ldso::elf::get<const Elf64_Phdr>(ehdr, ehdr->e_phoff);
  auto phdrs = ldso::array_view<const Elf64_Phdr>(phb, ehdr->e_phnum);
  auto found =
      std::find_if(phdrs.begin(), phdrs.end(), [](const Elf64_Phdr &phdr) {
        return phdr.p_type == PT_DYNAMIC;
      });
  ldso_assert(found != phdrs.end() && "No dynamic phdr found.");
  auto dyns = ldso::elf::dynamic::get_dyns(ehdr, found->p_offset);
  auto strtab_off = std::find_if(dyns.begin(), dyns.end(), [](auto &dyn) {
    return dyn.d_tag == DT_STRTAB;
  });
  ldso_assert(strtab_off != dyns.end());

  const char *strtab = [&] {
    if (ehdr->e_type == ET_EXEC)
      return reinterpret_cast<const char *>(strtab_off->d_un.d_val);
    else if (ehdr->e_type == ET_DYN)
      return ldso::elf::get<const char>(ehdr, strtab_off->d_un.d_val);
    else
      unreachable("Unsupported ELF type");
  }();

  for (auto &dyn : dyns) {
    if (dyn.d_tag == DT_NEEDED &&
        std::find_if(dsos.begin(), dsos.end(), [&](auto &dso) {
          return dso.name == strtab + dyn.d_un.d_val;
        }) == dsos.end()) {
      auto it = dsos.emplace(strtab + dyn.d_un.d_val);
      build_graph(it.first->file.file, dsos);
    }
  }
}

dso::dso(ldso::string name_in)
    : file{(ldso::string{"/lib/"} + name_in).c_str()},
      name{std::move(name_in)} {}

} // namespace ldso

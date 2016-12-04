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
      name{std::move(name_in)} {
  auto ehdr = file.file;
  auto phb = ldso::elf::get<const Elf64_Phdr>(ehdr, ehdr->e_phoff);
  auto phdrs = ldso::array_view<const Elf64_Phdr>(phb, ehdr->e_phnum);
  ldso::transform_if(phdrs.begin(), phdrs.end(), std::back_inserter(segments),
                     [this](auto &phdr) { return load_segment(phdr); },
                     [](auto &phdr) { return phdr.p_type == PT_LOAD; });

  auto found =
      std::find_if(phdrs.begin(), phdrs.end(), [](const Elf64_Phdr &phdr) {
        return phdr.p_type == PT_DYNAMIC;
      });
  auto dyns = ldso::elf::dynamic::get_dyns(ehdr, found->p_offset);
  auto strtab_off = std::find_if(dyns.begin(), dyns.end(), [](auto &dyn) {
    return dyn.d_tag == DT_STRTAB;
  });
  strtab = ldso::elf::get<const char>(ehdr, strtab_off->d_un.d_val);
}

segment dso::load_segment(const Elf64_Phdr &phdr) {
  auto *mapped = static_cast<uint8_t *>(
      sys::mmap(NULL, phdr.p_memsz, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  ldso_assert(mapped != MAP_FAILED);

  auto *segment = elf::get<const uint8_t>(file.file, phdr.p_offset);

  memcpy(mapped, segment, phdr.p_filesz);
  memset(mapped + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);

  auto prot = phdr.p_flags & PF_X ? PROT_EXEC : 0;
  prot = prot | (phdr.p_flags & PF_W ? PROT_WRITE : 0);
  prot = prot | (phdr.p_flags & PF_R ? PROT_READ : 0);

  if (prot != (PROT_READ | PROT_WRITE))
    sys::mprotect(mapped, phdr.p_memsz, prot);

  return {mapped, phdr.p_vaddr, phdr.p_vaddr + phdr.p_memsz};
}

} // namespace ldso

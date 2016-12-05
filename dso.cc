#include <dso.hh>
#include <dynamic.hh>
#include <elf.hh>

namespace ldso {

dso::dso(string name_in)
    : file{(string{name_in == "libsimple.so" ? "test/" : "/lib/"} + name_in)
               .c_str()},
      name{std::move(name_in)} {
  auto ehdr = file.file;
  auto phb = elf::get<const Elf64_Phdr>(ehdr, ehdr->e_phoff);
  auto phdrs = array_view<const Elf64_Phdr>(phb, ehdr->e_phnum);
  transform_if(phdrs.begin(), phdrs.end(), std::back_inserter(segments),
               [this](auto &phdr) { return load_segment(phdr); },
               [](auto &phdr) { return phdr.p_type == PT_LOAD; });

  auto found =
      std::find_if(phdrs.begin(), phdrs.end(), [](const Elf64_Phdr &phdr) {
        return phdr.p_type == PT_DYNAMIC;
      });
  dyns = elf::dynamic::get_dyns(ehdr, found->p_offset);
  auto strtab_off = std::find_if(dyns.begin(), dyns.end(), [](auto &dyn) {
    return dyn.d_tag == DT_STRTAB;
  });

  strtab = elf::get<const char>(ehdr, strtab_off->d_un.d_val);

  auto symtab_off = std::find_if(dyns.begin(), dyns.end(), [](auto &dyn) {
    return dyn.d_tag == DT_SYMTAB;
  });

  symtab = elf::dynamic::get_symtab(ehdr, symtab_off->d_un.d_val);
}

segment dso::load_segment(const Elf64_Phdr &phdr) {
  // RE segments can be backed by the file.
  if (phdr.p_flags == (PF_X | PF_R)) {
    ldso_assert(phdr.p_memsz == phdr.p_filesz);
    auto *file = static_cast<uint8_t *>(
        sys::mmap(NULL, phdr.p_memsz, PROT_READ | PROT_EXEC, MAP_PRIVATE,
                  this->file.fd, phdr.p_offset));
    ldso_assert(file != MAP_FAILED);

    return {file, phdr.p_vaddr, phdr.p_vaddr + phdr.p_memsz};
  } else {
    auto *map = static_cast<uint8_t *>(
        sys::mmap(NULL, phdr.p_memsz, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    ldso_assert(map != MAP_FAILED);
    auto *segment = elf::get<const uint8_t>(file.file, phdr.p_offset);
    memcpy(map, segment, phdr.p_filesz);
    auto prot = phdr.p_flags & PF_X ? PROT_EXEC : 0;
    prot = prot | (phdr.p_flags & PF_W ? PROT_WRITE : 0);
    prot = prot | (phdr.p_flags & PF_R ? PROT_READ : 0);

    if (prot != (PROT_READ | PROT_WRITE))
      sys::mprotect(map, phdr.p_memsz, prot);

    return {map, phdr.p_vaddr, phdr.p_vaddr + phdr.p_memsz};
  }
}

dso::sym_t dso::symbol(string_view str) const {
  auto find = std::find_if(symtab.begin(), symtab.end(), [&](auto &sym) {
    return strtab + sym.st_name == str;
  });

  if (find == symtab.end())
    return nullptr;

  auto off = find->st_value;
  auto segment =
      std::find_if(segments.begin(), segments.end(), [&](auto &segm) {
        return off >= segm.begin_off && off <= segm.end_off;
      });

  assert(segment != segments.end());

  return segment->file + off - segment->begin_off;
}

bin::bin(const Elf64_Ehdr *ehdr) : file{ehdr} {
  auto nullhdr = static_cast<const Elf64_Ehdr *>(nullptr);
  auto phb = elf::get<const Elf64_Phdr>(ehdr, ehdr->e_phoff);
  auto phdrs = array_view<const Elf64_Phdr>(phb, ehdr->e_phnum);
  auto found =
      std::find_if(phdrs.begin(), phdrs.end(), [](const Elf64_Phdr &phdr) {
        return phdr.p_type == PT_DYNAMIC;
      });
  dyns = elf::dynamic::get_dyns(ehdr, found->p_offset);
  auto strtab_off = std::find_if(dyns.begin(), dyns.end(), [](auto &dyn) {
    return dyn.d_tag == DT_STRTAB;
  });

  strtab = elf::get<const char>(nullhdr, strtab_off->d_un.d_val);

  auto symtab_off = std::find_if(dyns.begin(), dyns.end(), [](auto &dyn) {
    return dyn.d_tag == DT_SYMTAB;
  });

  symtab = elf::dynamic::get_symtab(ehdr, symtab_off->d_un.d_val);
}

} // namespace ldso

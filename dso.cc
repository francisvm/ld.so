#include <dso.hh>
#include <dynamic.hh>
#include <elf.hh>

namespace ldso {

dso::dso(string name_in)
    : file{(string{"test/"} + name_in)
               .c_str()},
      name{std::move(name_in)} {
  auto ehdr = file.file;
  auto phb = elf::get<const Elf64_Phdr>(ehdr, ehdr->e_phoff);
  auto phdrs = array_view<const Elf64_Phdr>(phb, ehdr->e_phnum);
  segments = load_segments(phdrs);

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

uint8_t *dso::load_segments(const array_view<const Elf64_Phdr> &phdrs) {
  ldso_assert(std::count_if(phdrs.begin(), phdrs.end(), [&](auto &phdr) {
                return phdr.p_type == PT_LOAD;
              }) == 2);
  auto &code = *std::find_if(phdrs.begin(), phdrs.end(), [&](auto &phdr) {
    return phdr.p_type == PT_LOAD && (phdr.p_flags == (PF_X | PF_R));
  });
  auto &data = *std::find_if(phdrs.begin(), phdrs.end(), [&](auto &phdr) {
    return phdr.p_type == PT_LOAD && (phdr.p_flags == (PF_W | PF_R));
  });

  // Reserve memory for both code and data.
  auto *mapped = static_cast<uint8_t *>(
      sys::mmap(NULL, code.p_memsz + data.p_vaddr + data.p_memsz,
                PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  ldso_assert(mapped != MAP_FAILED);

  // Map code at a fixed address.
  if (sys::mmap(mapped, code.p_memsz, PROT_READ | PROT_EXEC,
                MAP_PRIVATE | MAP_FIXED, file.fd, code.p_offset) == MAP_FAILED)
    unreachable("mapping the code segment failed");

  return mapped;
}

sym_t dso::symbol(string_view str) const {
  auto find = std::find_if(symtab.begin(), symtab.end(), [&](auto &sym) {
    return strtab + sym.st_name == str;
  });

  if (find == symtab.end())
    return nullptr;

  auto off = find->st_value;
  return segments + off;
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

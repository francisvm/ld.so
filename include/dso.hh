#pragma once

#include <dynamic.hh>
#include <elf.h>
#include <elf.hh>
#include <functional>
#include <mapped_file.hh>
#include <utils.hh>

namespace ldso {

using sym_t = void *;

struct dso {
  dso(string);
  mapped_file<const Elf64_Ehdr> file;
  string name;
  unordered_set<const dso *> deps;
  array_view<const Elf64_Dyn> dyns;
  const char *strtab;
  array_view<const Elf64_Sym> symtab;
  uint8_t *segments = nullptr;

  uint8_t *load_segments(const array_view<const Elf64_Phdr> &);

  sym_t symbol(string_view) const;
};

struct bin {
  bin(const Elf64_Ehdr *ehdr);
  const Elf64_Ehdr *file;
  unordered_set<const dso *> deps;
  array_view<const Elf64_Dyn> dyns;
  const char *strtab;
  array_view<const Elf64_Sym> symtab;
};

inline bool operator==(const dso &a, const dso &b) { return a.name == b.name; }
}

namespace std {

template <> struct hash<ldso::dso> {
  using argument_type = ldso::dso;
  using result_type = std::size_t;
  result_type operator()(argument_type const &s) const {
    return std::hash<ldso::string>{}(s.name);
  }
};

} // namespace std

namespace ldso {

template <typename T> void build_graph(T &obj, unordered_set<dso> &dsos) {
  const Elf64_Ehdr *ehdr = obj.file;
  auto phb = elf::get<const Elf64_Phdr>(ehdr, ehdr->e_phoff);
  auto phdrs = array_view<const Elf64_Phdr>(phb, ehdr->e_phnum);
  auto found =
      std::find_if(phdrs.begin(), phdrs.end(), [](const Elf64_Phdr &phdr) {
        return phdr.p_type == PT_DYNAMIC;
      });
  ldso_assert(found != phdrs.end() && "No dynamic phdr found.");
  auto dyns = elf::dynamic::get_dyns(ehdr, found->p_offset);
  auto strtab_off = std::find_if(dyns.begin(), dyns.end(), [](auto &dyn) {
    return dyn.d_tag == DT_STRTAB;
  });
  ldso_assert(strtab_off != dyns.end());

  const char *strtab = [&] {
    if (ehdr->e_type == ET_EXEC)
      return reinterpret_cast<const char *>(strtab_off->d_un.d_val);
    else if (ehdr->e_type == ET_DYN)
      return elf::get<const char>(ehdr, strtab_off->d_un.d_val);
    else
      unreachable("Unsupported ELF type");
  }();

  for (auto &dyn : dyns) {
    if (dyn.d_tag == DT_NEEDED &&
        std::find_if(dsos.begin(), dsos.end(), [&](auto &dso) {
          return dso.name == strtab + dyn.d_un.d_val;
        }) == dsos.end()) {
      auto it = dsos.emplace(strtab + dyn.d_un.d_val);
      obj.deps.insert(&*it.first);
      build_graph(const_cast<std::decay_t<decltype(*it.first)> &>(*it.first),
                  dsos);
    }
  }
}

} // namespace ldso

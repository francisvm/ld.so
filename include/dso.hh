#pragma once

#include <elf.h>
#include <functional>
#include <utils.hh>
#include <mapped_file.hh>

namespace ldso {

struct segment {
  uint8_t *file;
  size_t begin_off;
  size_t end_off;
};

struct dso {
  dso(ldso::string name_in);
  mapped_file<const Elf64_Ehdr> file;
  ldso::string name;
  ldso::unordered_set<const dso *> deps;
  const char *strtab;
  ldso::vector<segment> segments;

  segment load_segment(const Elf64_Phdr &phdr);
};

inline bool operator==(const dso &a, const dso &b) { return a.name == b.name; }

void build_graph(const Elf64_Ehdr *ehdr, ldso::unordered_set<dso> &dsos);

} // namespace ldso

namespace std {
template <> struct hash<ldso::dso> {
  using argument_type = ldso::dso;
  using result_type = std::size_t;
  result_type operator()(argument_type const &s) const {
    return std::hash<ldso::string>{}(s.name);
  }
};
} // namespace std

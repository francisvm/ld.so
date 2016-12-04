#pragma once

#include <elf.h>
#include <functional>
#include <mapped_file.hh>
#include <utils.hh>

namespace ldso {

struct segment {
  uint8_t *file;
  size_t begin_off;
  size_t end_off;
};

struct dso {
  dso(string);
  mapped_file<const Elf64_Ehdr> file;
  string name;
  unordered_set<const dso *> deps;
  const char *strtab;
  vector<segment> segments;

  segment load_segment(const Elf64_Phdr &);
};

inline bool operator==(const dso &a, const dso &b) { return a.name == b.name; }

void build_graph(const Elf64_Ehdr *, unordered_set<dso> &);

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

#pragma once

#include <elf.h>
#include <functional>
#include <stl.hh>
#include <utils.hh>

namespace ldso {

struct dso {
  dso(ldso::string name_in);
  mapped_file<const Elf64_Ehdr> file;
  ldso::string name;
  ldso::unordered_set<const dso *> deps;
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

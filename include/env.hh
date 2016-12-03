#pragma once

#include <stl.hh>

namespace ldso {
struct program {
  array_view<const Elf64_auxv_t> auxv;
  array_view<const char *> envp;
};

inline program exe;

inline string_view get_env(string_view name) {
  auto found = std::find_if(exe.envp.begin(), exe.envp.end(), [&](auto env) {
    return string_view(env).split('=').first == name;
  });

  if (found == exe.envp.end())
    return {};
  return string_view{*found}.split('=').second;
}
} // namespace ldso

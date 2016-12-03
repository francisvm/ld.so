#pragma once

#include <elf.h>
#include <stl.hh>
#include <utils.hh>

namespace ldso::elf::dynamic {

ldso::array_view<const Elf64_Dyn> get_dyns(const Elf64_Ehdr *ehdr,
                                           const Elf64_Off off);

} // namespace ldso::elf::dynamic

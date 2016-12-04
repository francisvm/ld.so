#pragma once

#include <elf.h>
#include <stl.hh>
#include <utils.hh>

namespace ldso::elf::dynamic {

array_view<const Elf64_Dyn> get_dyns(const Elf64_Ehdr *, const Elf64_Off);

array_view<const Elf64_Sym> get_symtab(const Elf64_Ehdr *, const Elf64_Off);

} // namespace ldso::elf::dynamic

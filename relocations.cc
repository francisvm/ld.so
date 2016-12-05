#include <elf.h>
#include <relocations.hh>

namespace ldso {

void relocate(uint8_t *dst, uint8_t type, sym_t sym) {
  switch (type) {
  case R_X86_64_JUMP_SLOT:
    memcpy(dst, reinterpret_cast<void *>(&sym), sizeof(void *));
    break;
  default:
    unreachable("relocation type not supported");
  }
}

} // namespace ldso

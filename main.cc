#include <allocator.hh>
#include <syscall.hh>
#include <utils.hh>
#include <vector>

extern "C" void load(int argc, const char *argv[]) {
  if (argc < 2)
    unreachable("usage: ./ld.so LIBRARY");

  auto filename = argv[1];

  {
    ldso::mapped_file lib{filename};
    std::vector<ldso::mapped_file, ldso::allocator<ldso::mapped_file>> f;
    f.push_back(std::move(lib));
  }

  ldso::sys::_exit(0);
}

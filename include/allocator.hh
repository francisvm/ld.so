#pragma once

#include <syscall.hh>
#include <error.hh>

namespace ldso {

extern void *allocator_base;
extern size_t allocator_size;
extern size_t allocator_capacity;

static constexpr size_t page_size = 4096;

template <typename T> struct allocator {
  using value_type = T;

  static void *new_page(size_t n) {
    return sys::mmap(NULL, n, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  }

  static void request_page() {
    allocator_base = new_page(page_size);
    if (allocator_base == MAP_FAILED)
      unreachable("allocator mmap failed");
    allocator_size = 0;
    allocator_capacity = page_size;
  }

  T *allocate(size_t nb) {
    // mmap for sizes >= page_size / 2.
    if (auto n = nb * sizeof (T); n >= page_size / 2) {
      if (auto *mem = new_page(n); mem == MAP_FAILED)
        unreachable("allocator mmap failed");
      else
        return reinterpret_cast<T *>(mem);
    } else { // use a dummy bumpptr allocator.
      if (!allocator_base || (allocator_size + n > allocator_capacity))
        request_page();
      auto mem = reinterpret_cast<uint8_t *>(allocator_base) + allocator_size;
      allocator_size += n;
      return reinterpret_cast<T *>(mem);
    }
  }

  void deallocate(T *p, size_t nb) {
    auto n = nb * sizeof(T);
    if (n >= page_size / 2) {
      if (sys::munmap(p, n) < 0)
        unreachable("munmap failed");
    }
  }
};

} // namespace ldso

#include <cstddef>

namespace ldso {
void *allocator_base = nullptr;
size_t allocator_size;
size_t allocator_capacity;
} // namespace ldso

extern "C" {
void *memcpy(void *dest, const void *src, size_t n) {
  auto *d = reinterpret_cast<char *>(dest);
  auto *s = reinterpret_cast<const char *>(src);
  for (size_t i = 0; i < n; ++i)
    d[i] = s[i];

  return d;
}

void *memset(void *s, int c, size_t n) {
  auto *b = reinterpret_cast<char *>(s);
  for (size_t i = 0; i < n; ++i)
    b[i] = c;

  return b;
}
}

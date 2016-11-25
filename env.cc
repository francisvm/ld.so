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

int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s1 == *s2) {
    ++s1;
    ++s2;
  }

  return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while (n && *s1 && *s1 == *s2) {
    ++s1;
    ++s2;
    --n;
  }

  return n == 0 ? 0 : *s1 - *s2;
}

size_t strlen(const char *s) {
  size_t res = 0;
  while (*s) {
    ++res;
    ++s;
  }

  return res;
}
}

#include <cstddef>
#include <error.hh>

namespace ldso {

void *allocator_base = nullptr;
size_t allocator_size;
size_t allocator_capacity;
} // namespace ldso

extern "C" {
void __assert_fail(const char * /*expr*/, const char * /*file*/,
                   unsigned int /*line*/, const char * /*func*/) {
  unreachable("Assert failed");
}

}

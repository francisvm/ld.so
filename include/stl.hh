#pragma once

#include <allocator.hh>
#include <llvm/ADT/ArrayRef.h>
#include <vector>

namespace ldso {

template <typename T> using vector = std::vector<T, ldso::allocator<T>>;

template <typename T> using array_view = llvm::ArrayRef<T>;

template <class InputIt, class OutputIt, class UnaryOperation,
          class UnaryPredicate>
void transform_if(InputIt first, InputIt last, OutputIt result,
                      UnaryOperation op, UnaryPredicate pred) {
  for (; first != last; ++first, ++result)
    if (pred(*first))
      *result = op(*first);
}

} // namespace ldso

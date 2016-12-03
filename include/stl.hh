#pragma once

#include <allocator.hh>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

namespace ldso {

template <typename T> using vector = std::vector<T, ldso::allocator<T>>;

template <typename T> using set = std::set<T, ldso::allocator<T>>;

using string =
    std::basic_string<char, std::char_traits<char>, ldso::allocator<char>>;

template <typename T>
using unordered_set =
    std::unordered_set<T, std::hash<T>, std::equal_to<T>, ldso::allocator<T>>;

template <typename T> using array_view = llvm::ArrayRef<T>;

using string_view = llvm::StringRef;

template <class InputIt, class OutputIt, class UnaryOperation,
          class UnaryPredicate>
void transform_if(InputIt first, InputIt last, OutputIt result,
                  UnaryOperation op, UnaryPredicate pred) {
  for (; first != last; ++first, ++result)
    if (pred(*first))
      *result = op(*first);
}

} // namespace ldso

#pragma once

#include <llvm/ADT/SmallVector.h>

namespace Krys
{
  /// @brief This is a 'vector' (really, a variable-sized array), optimized
  /// for the case when the array is small.  It contains some number of elements
  /// in-place, which allows it to avoid heap allocation when the actual number of
  /// elements is below that threshold.  This allows normal "small" cases to be
  /// fast without losing generality for large inputs.
  /// @note
  /// In the absence of a well-motivated choice for the number of inlined
  /// elements \p N, it is recommended to use \c SmallVector<T> (that is,
  /// omitting the \p N). This will choose a default number of inlined elements
  /// reasonable for allocation on the stack (for example, trying to keep \c
  /// sizeof(SmallVector<T>) around 64 bytes).
  /// @warning This does not attempt to be exception safe.
  /// @see https://llvm.org/docs/ProgrammersManual.html#llvm-adt-smallvector-h
  template <typename T, unsigned N = llvm::CalculateSmallVectorDefaultInlinedElements<T>::value>
  using SmallList = llvm::SmallVector<T, N>;
}
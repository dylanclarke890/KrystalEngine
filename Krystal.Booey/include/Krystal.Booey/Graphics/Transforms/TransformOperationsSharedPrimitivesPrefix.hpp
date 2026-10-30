#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <algorithm>

namespace krys::boo
{
  class TransformOperations;

  // This class is used to find a shared prefix of transform function primitives (as
  // defined by CSS Transforms Level 1 & 2). Given a series of transform function ranges in
  // the keyframes of an animation. After `update()` is called with the transform function range
  // of every keyframe, `primitives()` will return the prefix of primitives that are shared
  // by all keyframes passed to `update()`.
  template <typename PrimitiveType>
  class TransformOperationsSharedPrimitivesPrefix final
  {
  public:
    void Update(const auto &) noexcept;

    KRYS_NODISCARD bool HadIncompatibleTransformFunctions() const noexcept
    {
      return _indexOfFirstMismatch.has_value();
    }
    KRYS_NODISCARD const List<PrimitiveType> &Primitives() const noexcept
    {
      return _primitives;
    }

  private:
    Maybe<size_t> _indexOfFirstMismatch;
    List<PrimitiveType> _primitives;
  };

  template <typename PrimitiveType>
  void TransformOperationsSharedPrimitivesPrefix<PrimitiveType>::Update(const auto &operations) noexcept
  {
    size_t maxIteration = operations.size();
    if (_indexOfFirstMismatch.has_value())
    {
      maxIteration = std::min(*_indexOfFirstMismatch, maxIteration);
    }

    for (size_t i = 0; i < maxIteration; ++i)
    {
      const auto &operation = operations[i];

      // If we haven't seen an operation at this index before, we can simply use our primitive type.
      if (i >= _primitives.size())
      {
        krys_debug_assert(i == _primitives.size());
        _primitives.push_back(operation->PrimitiveType());
        continue;
      }

      if (auto sharedPrimitive = operation->SharedPrimitiveType(_primitives[i]))
      {
        _primitives[i] = *sharedPrimitive;
      }
      else
      {
        _indexOfFirstMismatch = i;
        _primitives.resize(i);
        return;
      }
    }
  }
}

/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

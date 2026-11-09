/*
 * Copyright (C) 2009-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Layers/GraphicsLayerAnimationValue.hpp"
#include "Krystal.Booey/Graphics/Layers/GraphicsLayerClient.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief Used to store a series of values in a keyframe list. Values will all be of the same type, which
  /// can be inferred from the property.
  class GraphicsLayerKeyframeValueList
  {
  protected:
    List<UniquePtr<const GraphicsLayerAnimationValue>> _values;
    AnimatedProperty _property;

  public:
    explicit GraphicsLayerKeyframeValueList(AnimatedProperty property) noexcept : _property(property)
    {
    }

    explicit GraphicsLayerKeyframeValueList(const GraphicsLayerKeyframeValueList &other) noexcept
        : _property(other.Property())
    {
      _values.reserve(other._values.size());
      for (const auto &value : other._values)
      {
        _values.push_back(value->Clone());
      }
    }
    GraphicsLayerKeyframeValueList(GraphicsLayerKeyframeValueList &&) noexcept = default;

    GraphicsLayerKeyframeValueList &operator=(const GraphicsLayerKeyframeValueList &other) noexcept
    {
      GraphicsLayerKeyframeValueList copy(other);
      Swap(copy);
      return *this;
    }
    GraphicsLayerKeyframeValueList &operator=(GraphicsLayerKeyframeValueList &&) noexcept = default;

    void Swap(GraphicsLayerKeyframeValueList &other) noexcept
    {
      std::swap(_property, other._property);
      _values.swap(other._values);
    }

    KRYS_NODISCARD AnimatedProperty Property() const noexcept
    {
      return _property;
    }

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _values.size();
    }
    KRYS_NODISCARD const GraphicsLayerAnimationValue &At(size_t i) const noexcept
    {
      return *_values.at(i);
    }

    // Insert, sorted by keyTime.
    void Insert(UniquePtr<const GraphicsLayerAnimationValue> value) noexcept;
  };
}

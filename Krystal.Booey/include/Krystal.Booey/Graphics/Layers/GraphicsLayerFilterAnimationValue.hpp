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

#include "Krystal.Booey/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Booey/Graphics/Layers/GraphicsLayerAnimationValue.hpp"

namespace krys::boo
{
  /// @brief Used to store one filter value in a keyframe list.
  class GraphicsLayerFilterAnimationValue : public GraphicsLayerAnimationValue
  {
  private:
    FilterOperations _value;

  public:
    GraphicsLayerFilterAnimationValue(double keyTime, const FilterOperations &value,
                                      TimingFunction *timingFunction = nullptr) noexcept
        : GraphicsLayerAnimationValue(keyTime, timingFunction), _value(value)
    {
    }

    explicit GraphicsLayerFilterAnimationValue(const GraphicsLayerFilterAnimationValue &other) noexcept
        : GraphicsLayerAnimationValue(other), _value(other._value.Clone())
    {
    }
    GraphicsLayerFilterAnimationValue(GraphicsLayerFilterAnimationValue &&) noexcept = default;

    KRYS_NODISCARD UniquePtr<GraphicsLayerAnimationValue> Clone() const noexcept override
    {
      return CreateUnique<GraphicsLayerFilterAnimationValue>(*this);
    }

    KRYS_NODISCARD const FilterOperations &Value() const noexcept
    {
      return _value;
    }
  };
}

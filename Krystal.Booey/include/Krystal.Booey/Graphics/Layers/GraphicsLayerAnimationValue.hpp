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

#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  /// @brief Base class for animation values (also used for transitions). Here to represent values for
  /// properties being animated via the GraphicsLayer, without pulling in style-related data from outside of
  /// the platform directory.
  class GraphicsLayerAnimationValue
  {
  private:
    double _keyTime;
    const RefPtr<TimingFunction> _timingFunction;

  public:
    virtual ~GraphicsLayerAnimationValue() noexcept = default;

    KRYS_NODISCARD double KeyTime() const noexcept
    {
      return _keyTime;
    }
    KRYS_NODISCARD const TimingFunction *GetTimingFunction() const noexcept
    {
      return _timingFunction.get();
    }

    KRYS_NODISCARD virtual UniquePtr<GraphicsLayerAnimationValue> Clone() const noexcept = 0;

    void operator=(const GraphicsLayerAnimationValue &) = delete;

  protected:
    GraphicsLayerAnimationValue(double keyTime, TimingFunction *timingFunction = nullptr) noexcept
        : _keyTime(keyTime), _timingFunction(ShareRefPtr(timingFunction))
    {
    }

    explicit GraphicsLayerAnimationValue(const GraphicsLayerAnimationValue &other) noexcept
        : _keyTime(other._keyTime),
          _timingFunction(other._timingFunction ? RefPtr<TimingFunction> {other._timingFunction->Clone()}
                                                : nullptr)
    {
    }
    GraphicsLayerAnimationValue(GraphicsLayerAnimationValue &&) noexcept = default;
  };
}

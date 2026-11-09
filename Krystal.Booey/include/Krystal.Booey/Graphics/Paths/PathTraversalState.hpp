/*
 * Copyright (C) 2006, 2007 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"

namespace krys::boo
{
  class PathTraversalState
  {
  public:
    enum class Action
    {
      TotalLength,
      VectorAtLength,
      SegmentAtLength,
    };

    PathTraversalState(Action, float desiredLength = 0) noexcept;

  public:
    KRYS_NODISCARD bool ProcessPathElement(PathElement::Type, std::span<const FloatPoint>) noexcept;
    KRYS_NODISCARD bool ProcessPathElement(const PathElement &element) noexcept
    {
      return ProcessPathElement(element.ElementType, element.Points);
    }

    KRYS_NODISCARD Action GetAction() const noexcept
    {
      return _action;
    }
    void SetAction(Action action) noexcept
    {
      _action = action;
    }
    KRYS_NODISCARD float DesiredLength() const noexcept
    {
      return _desiredLength;
    }
    void SetDesiredLength(float desiredLength) noexcept
    {
      _desiredLength = desiredLength;
    }

    // Traversing output -- should be read only
    KRYS_NODISCARD bool Success() const noexcept
    {
      return _success;
    }
    KRYS_NODISCARD float TotalLength() const noexcept
    {
      return _totalLength;
    }
    KRYS_NODISCARD FloatPoint Current() const noexcept
    {
      return _current;
    }
    KRYS_NODISCARD float NormalAngle() const noexcept
    {
      return _normalAngle;
    }

  private:
    void CloseSubpath() noexcept;
    void MoveTo(const FloatPoint &) noexcept;
    void LineTo(const FloatPoint &) noexcept;
    void QuadraticBezierTo(const FloatPoint &, const FloatPoint &) noexcept;
    void CubicBezierTo(const FloatPoint &, const FloatPoint &, const FloatPoint &) noexcept;

    KRYS_NODISCARD bool FinalizeAppendPathElement() noexcept;
    KRYS_NODISCARD bool AppendPathElement(PathElement::Type, std::span<const FloatPoint>) noexcept;

  private:
    Action _action;
    bool _success {false};

    FloatPoint _current;
    FloatPoint _start;

    float _totalLength {0};
    float _desiredLength {0};

    // For normal calculations
    FloatPoint _previous;
    float _normalAngle {0}; // degrees
    bool _isZeroVector {false};
  };
}

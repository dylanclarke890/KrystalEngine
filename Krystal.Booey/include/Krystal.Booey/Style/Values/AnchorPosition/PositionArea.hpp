/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Style/Values/AnchorPosition/PositionAreaSpan.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo
{
  class WritingMode;
  enum class BoxAxis : uint8;
  enum class ItemPosition : uint8;
  enum class LogicalBoxAxis : uint8;

  namespace style
  {
    // <'position-area'> = none | <position-area>
    // https://drafts.csswg.org/css-anchor-position-1/#propdef-position-area

    // A non-`none` position-area is formed by two spans of opposite axes, that uniquely determine
    // the area of the containing block.
    struct PositionAreaValue
    {
      PositionAreaValue(PositionAreaSpan blockOrXAxis, PositionAreaSpan inlineOrYAxis) noexcept;

      KRYS_NODISCARD constexpr PositionAreaSpan BlockOrXAxis() const noexcept
      {
        return _blockOrXAxis;
      }
      KRYS_NODISCARD constexpr PositionAreaSpan InlineOrYAxis() const noexcept
      {
        return _inlineOrYAxis;
      }

      KRYS_NODISCARD PositionAreaSpan SpanForAxis(BoxAxis physicalAxis, WritingMode containerWritingMode,
                                                  WritingMode selfWritingMode) const noexcept;
      KRYS_NODISCARD PositionAreaSpan SpanForAxis(LogicalBoxAxis logicalAxis,
                                                  WritingMode containerWritingMode,
                                                  WritingMode selfWritingMode) const noexcept;

      // Start/end based on container's coordinate-increasing direction (RenderBox coordinates)
      KRYS_NODISCARD PositionAreaTrack CoordMatchedTrackForAxis(BoxAxis, WritingMode containerWritingMode,
                                                                WritingMode selfWritingMode) const noexcept;

      KRYS_NODISCARD ItemPosition DefaultAlignmentForAxis(BoxAxis, WritingMode containerWritingMode,
                                                          WritingMode selfWritingMode) const noexcept;

      KRYS_NODISCARD constexpr bool operator==(const PositionAreaValue &) const noexcept = default;

    private:
      PositionAreaSpan _blockOrXAxis;
      PositionAreaSpan _inlineOrYAxis;
    };

    struct PositionArea
    {
      PositionArea(css::keywords::None) noexcept : _value {}
      {
      }
      PositionArea(PositionAreaValue value) noexcept : _value {value}
      {
      }

      KRYS_NODISCARD bool IsNone() const noexcept
      {
        return !_value;
      }
      KRYS_NODISCARD bool IsValue() const noexcept
      {
        return !!_value;
      }
      KRYS_NODISCARD Maybe<PositionAreaValue> TryValue() const noexcept
      {
        return _value;
      }

      template <typename... F>
      decltype(auto) SwitchOn(F &&...f) const noexcept
      {
        auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
        if (IsNone())
        {
          return visitor(css::keywords::None {});
        }
        return visitor(*_value);
      }

      KRYS_NODISCARD bool operator==(const PositionArea &) const noexcept = default;

    private:
      Maybe<PositionAreaValue> _value;
    };

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<PositionArea>
    {
      KRYS_NODISCARD PositionArea operator()(BuilderState &, const css::Value &) noexcept;
    };
    template <>
    struct CSSValueCreation<PositionAreaValue>
    {
      KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                                const PositionAreaValue &) noexcept;
    };

    // MARK: - Serialization

    template <>
    struct Serialiser<PositionAreaValue>
    {
      void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                      const PositionAreaValue &) noexcept;
    };

  }
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::PositionArea)

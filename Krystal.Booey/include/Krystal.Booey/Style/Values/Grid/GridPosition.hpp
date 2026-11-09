/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 * Copyright (C) 2013-2017 Igalia S.L.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridPositionSide.hpp"
#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::style
{
  // <grid-line-explicit> = [ [ <integer [-∞,-1]> | <integer [1,∞]> ] && <custom-ident>? ]
  struct GridPositionExplicit
  {
    // NOTE: We don't currently have an efficient way to represent disjoint numeric ranges so an unconstrained
    // range is used here instead.
    using PositionType = Integer<>;

    PositionType Position {1};
    css::CustomIdentifier Name {css::CSSOMStringAtom::Null()};

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (Name.Value == css::CSSOMStringAtom::Null())
      {
        return visitor(Position);
      }

      return visitor(css::SpaceSeparatedTuple {Position, Name});
    }

    KRYS_NODISCARD bool operator==(const GridPositionExplicit &) const noexcept = default;
  };

  // <grid-line-span> = [ span && [ <integer [1,∞]> || <custom-ident>  ] ]
  struct GridPositionSpan
  {
    using PositionType = Integer<css::Range {1, css::Range::Inf}>;

    PositionType Position {1};
    css::CustomIdentifier Name {css::CSSOMStringAtom::Null()};

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (Name.Value == css::CSSOMStringAtom::Null())
      {
        return visitor(css::SpaceSeparatedTuple {css::keywords::Span {}, Position});
      }

      if (Position.Value == 1)
      {
        return visitor(css::SpaceSeparatedTuple {css::keywords::Span {}, Name});
      }

      return visitor(css::SpaceSeparatedTuple {css::keywords::Span {}, Position, Name});
    }

    KRYS_NODISCARD bool operator==(const GridPositionSpan &) const noexcept = default;
  };

  // <grid-line> = auto | <custom-ident> | <grid-line-explicit> | <grid-line-span>
  // https://drafts.csswg.org/css-grid/#typedef-grid-row-start-grid-line
  // FIXME: The standard calls this type "grid-line". We should consider matching it.
  struct GridPosition
  {
    using Explicit = GridPositionExplicit;
    using Span = GridPositionSpan;

  private:
    enum class GridPositionType : uint8
    {
      Auto,
      Explicit,
      Span,
      NamedGridArea
    };

    GridPositionType _type {GridPositionType::Auto};
    int _integerPosition {1};
    css::CustomIdentifier _namedGridLine {css::CSSOMStringAtom::Null()};

  public:
    GridPosition(css::keywords::Auto) noexcept
    {
    }

    GridPosition(Explicit explicitPosition) noexcept;
    GridPosition(Span spanPosition) noexcept;
    GridPosition(css::CustomIdentifier namedGridAreaPosition) noexcept;

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return _type == GridPositionType::Auto;
    }

    KRYS_NODISCARD bool IsExplicit() const noexcept
    {
      return _type == GridPositionType::Explicit;
    }

    KRYS_NODISCARD bool IsSpan() const noexcept
    {
      return _type == GridPositionType::Span;
    }

    KRYS_NODISCARD bool IsNamedGridArea() const noexcept
    {
      return _type == GridPositionType::NamedGridArea;
    }

    KRYS_NODISCARD int ExplicitPosition() const noexcept;
    KRYS_NODISCARD int SpanPosition() const noexcept;
    KRYS_NODISCARD css::CSSOMString NamedGridLine() const noexcept;

    KRYS_NODISCARD bool ShouldBeResolvedAgainstOppositePosition() const noexcept
    {
      return IsAuto() || IsSpan();
    }

    // Note that grid line 1 is internally represented by the index 0, that's why the max value for
    // a position is gridMaxTracks instead of gridMaxTracks + 1.
    KRYS_NODISCARD static int Max() noexcept;
    KRYS_NODISCARD static int Min() noexcept;

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      switch (_type)
      {
        case GridPositionType::Auto:          return visitor(css::keywords::Auto {});
        case GridPositionType::Explicit:      return visitor(Explicit {{_integerPosition}, _namedGridLine});
        case GridPositionType::Span:          return visitor(Span {{_integerPosition}, _namedGridLine});
        case GridPositionType::NamedGridArea: return visitor(_namedGridLine);
      }

      krys_unreachable();
    }

    KRYS_NODISCARD bool operator==(const GridPosition &) const noexcept = default;

    static void SetMaxPositionForTesting(unsigned maxPosition) noexcept;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<GridPosition>
  {
    KRYS_NODISCARD GridPosition operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::GridPosition)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::GridPosition::Explicit)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::GridPosition::Span)

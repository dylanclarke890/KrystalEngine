#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueAggregates.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // <grid-line-explicit> = [ [ <integer [-∞,-1]> | <integer [1,∞]> ] && <custom-ident>? ]
  struct GridPositionExplicit
  {
    // NOTE: We don't currently have an efficient way to represent disjoint numeric ranges so an unconstrained
    // range is used here instead.
    using Position = Integer<>;

    Position position {1};
    CustomIdentifier name {CSSOMStringAtom::Null()};

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = Krys::CreateVisitor(std::forward<F>(f)...);

      if (name.value == CSSOMStringAtom::Null())
      {
        return visitor(position);
      }

      return visitor(SpaceSeparatedTuple {position, name});
    }

    bool operator==(const GridPositionExplicit &) const = default;
  };

  // <grid-line-span> = [ span && [ <integer [1,∞]> || <custom-ident>  ] ]
  struct GridPositionSpan
  {
    using Position = Integer<CSSRange {1, CSSRange::Inf}>;

    Position position {1};
    CustomIdentifier name {CSSOMStringAtom::Null()};

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = Krys::CreateVisitor(std::forward<F>(f)...);

      if (name.value == CSSOMStringAtom::Null())
      {
        return visitor(SpaceSeparatedTuple {Keywords::Span {}, position});
      }

      if (position == IntegerRaw<CSSRange {1, CSSRange::Inf}> {1})
      {
        return visitor(SpaceSeparatedTuple {Keywords::Span {}, name});
      }

      return visitor(SpaceSeparatedTuple {Keywords::Span {}, position, name});
    }

    bool operator==(const GridPositionSpan &) const = default;
  };

  // <grid-line> = auto | <custom-ident> | <grid-line-explicit> | <grid-line-span>
  // https://drafts.csswg.org/css-grid/#typedef-grid-row-start-grid-line
  // FIXME: The standard calls this type "grid-line". We should consider matching it.
  struct GridPosition
  {
    using Explicit = GridPositionExplicit;
    using Span = GridPositionSpan;

    GridPosition(Keywords::Auto)
    {
    }

    GridPosition(Explicit &&);
    GridPosition(Span &&);
    GridPosition(CustomIdentifier &&);

    bool isAuto() const
    {
      return m_type == GridPositionType::Auto;
    }
    bool isExplicit() const
    {
      return m_type == GridPositionType::Explicit;
    }
    bool isSpan() const
    {
      return m_type == GridPositionType::Span;
    }
    bool isNamedGridArea() const
    {
      return m_type == GridPositionType::NamedGridArea;
    }

    int explicitPosition() const;
    int spanPosition() const;
    CSSOMString namedGridLine() const;

    bool shouldBeResolvedAgainstOppositePosition() const
    {
      return isAuto() || isSpan();
    }

    // Note that grid line 1 is internally represented by the index 0, that's why the max value for
    // a position is gridMaxTracks instead of gridMaxTracks + 1.
    static int max();
    static int min();

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = Krys::CreateVisitor(std::forward<F>(f)...);

      switch (m_type)
      {
        case GridPositionType::Auto:          return visitor(Keywords::Auto {});
        case GridPositionType::Explicit:      return visitor(Explicit {{m_integerPosition}, m_namedGridLine});
        case GridPositionType::Span:          return visitor(Span {{m_integerPosition}, m_namedGridLine});
        case GridPositionType::NamedGridArea: return visitor(m_namedGridLine);
      }

      std::unreachable();
    }

    bool operator==(const GridPosition &) const = default;

    static void setMaxPositionForTesting(unsigned);

  private:
    enum class GridPositionType : uint8
    {
      Auto,
      Explicit,
      Span,
      NamedGridArea
    };

    GridPositionType m_type {GridPositionType::Auto};
    int m_integerPosition {1};
    CustomIdentifier m_namedGridLine {CSSOMStringAtom::Null()};
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(Krys::HTML::GridPosition)
DEFINE_VARIANT_LIKE_CONFORMANCE(Krys::HTML::GridPosition::Explicit)
DEFINE_VARIANT_LIKE_CONFORMANCE(Krys::HTML::GridPosition::Span)
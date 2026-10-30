#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'grid-auto-flow'> = normal | [ row | column ] || dense
  // https://drafts.csswg.org/css-grid-1/#grid-auto-flow-property
  struct GridAutoFlow
  {
    enum class Direction : uint8
    {
      Normal,
      Row,
      Column
    };
    enum class Packing : bool
    {
      Dense,
      Sparse
    };

    constexpr GridAutoFlow(css::keywords::Normal) noexcept
    {
    }
    constexpr GridAutoFlow(css::keywords::Row) noexcept : _direction {static_cast<uint8>(Direction::Row)}
    {
    }
    constexpr GridAutoFlow(css::keywords::Row, css::keywords::Dense) noexcept
        : _direction {static_cast<uint8>(Direction::Row)}, _packing {static_cast<uint8>(Packing::Dense)}
    {
    }
    constexpr GridAutoFlow(css::keywords::Column) noexcept
        : _direction {static_cast<uint8>(Direction::Column)}
    {
    }
    constexpr GridAutoFlow(css::keywords::Column, css::keywords::Dense) noexcept
        : _direction {static_cast<uint8>(Direction::Column)}, _packing {static_cast<uint8>(Packing::Dense)}
    {
    }
    constexpr GridAutoFlow(css::keywords::Dense) noexcept : _packing {static_cast<uint8>(Packing::Dense)}
    {
    }

    KRYS_NODISCARD constexpr Direction GetDirection() const noexcept
    {
      return static_cast<Direction>(_direction);
    }
    KRYS_NODISCARD constexpr Packing GetPacking() const noexcept
    {
      return static_cast<Packing>(_packing);
    }

    KRYS_NODISCARD constexpr bool IsRow() const noexcept
    {
      return GetDirection() == Direction::Row;
    }
    KRYS_NODISCARD constexpr bool IsColumn() const noexcept
    {
      return GetDirection() == Direction::Column;
    }
    KRYS_NODISCARD constexpr bool IsDense() const noexcept
    {
      return GetPacking() == Packing::Dense;
    }
    KRYS_NODISCARD constexpr bool IsSparse() const noexcept
    {
      return GetPacking() == Packing::Sparse;
    }

    void SetDirection(Direction direction) noexcept
    {
      _direction = static_cast<uint8>(direction);
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...) const;

    KRYS_NODISCARD constexpr bool operator==(const GridAutoFlow &) const noexcept = default;

  private:
    uint8 _direction : 2 {static_cast<uint8>(Direction::Normal)};
    uint8 _packing : 1 {static_cast<uint8>(Packing::Sparse)};
  };

  template <typename... F>
  decltype(auto) GridAutoFlow::SwitchOn(F &&...f) const
  {
    auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

    switch (GetDirection())
    {
      case Direction::Column:
        switch (GetPacking())
        {
          case Packing::Dense:
            return visitor(css::SpaceSeparatedTuple {css::keywords::Column {}, css::keywords::Dense {}});
          case Packing::Sparse: return visitor(css::keywords::Column {});
        }
      case Direction::Row:
        switch (GetPacking())
        {
          case Packing::Dense:
            return visitor(css::SpaceSeparatedTuple {css::keywords::Row {}, css::keywords::Dense {}});
          case Packing::Sparse: return visitor(css::keywords::Row {});
        }
      case Direction::Normal:
        switch (GetPacking())
        {
          case Packing::Dense:  return visitor(css::keywords::Dense {});
          case Packing::Sparse: return visitor(css::keywords::Normal {});
        }
    }
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<GridAutoFlow>
  {
    KRYS_NODISCARD GridAutoFlow operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::GridAutoFlow)

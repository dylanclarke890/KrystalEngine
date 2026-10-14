#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Rendering/Style/StyleContentAlignmentData.hpp"
#include "Krystal.Booey/Style/Values/Align/OverflowPosition.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'justify-content'> = normal | <content-distribution> | <overflow-position>? [ <content-position> | left
  // | right ] https://drafts.csswg.org/css-align/#propdef-justify-content
  struct JustifyContent
  {
    constexpr JustifyContent(css::keywords::Normal) noexcept;
    constexpr JustifyContent(css::keywords::SpaceBetween) noexcept;
    constexpr JustifyContent(css::keywords::SpaceAround) noexcept;
    constexpr JustifyContent(css::keywords::SpaceEvenly) noexcept;
    constexpr JustifyContent(css::keywords::Stretch) noexcept;
    constexpr JustifyContent(css::keywords::Center, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyContent(css::keywords::Start, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyContent(css::keywords::End, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyContent(css::keywords::FlexStart, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyContent(css::keywords::FlexEnd, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyContent(css::keywords::Left, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyContent(css::keywords::Right, Maybe<OverflowPosition> = null) noexcept;

    KRYS_NODISCARD constexpr bool IsNormal() const noexcept
    {
      return Primary() == PrimaryKind::Normal;
    }
    KRYS_NODISCARD constexpr bool IsSpaceBetween() const noexcept
    {
      return Primary() == PrimaryKind::SpaceBetween;
    }
    KRYS_NODISCARD constexpr bool IsSpaceAround() const noexcept
    {
      return Primary() == PrimaryKind::SpaceAround;
    }
    KRYS_NODISCARD constexpr bool IsSpaceEvenly() const noexcept
    {
      return Primary() == PrimaryKind::SpaceEvenly;
    }
    KRYS_NODISCARD constexpr bool IsStretch() const noexcept
    {
      return Primary() == PrimaryKind::Stretch;
    }
    KRYS_NODISCARD constexpr bool IsCenter() const noexcept
    {
      return Primary() == PrimaryKind::Center;
    }
    KRYS_NODISCARD constexpr bool IsStart() const noexcept
    {
      return Primary() == PrimaryKind::Start;
    }
    KRYS_NODISCARD constexpr bool IsEnd() const noexcept
    {
      return Primary() == PrimaryKind::End;
    }
    KRYS_NODISCARD constexpr bool IsFlexStart() const noexcept
    {
      return Primary() == PrimaryKind::FlexStart;
    }
    KRYS_NODISCARD constexpr bool IsFlexEnd() const noexcept
    {
      return Primary() == PrimaryKind::FlexEnd;
    }
    KRYS_NODISCARD constexpr bool IsLeft() const noexcept
    {
      return Primary() == PrimaryKind::Left;
    }
    KRYS_NODISCARD constexpr bool IsRight() const noexcept
    {
      return Primary() == PrimaryKind::Right;
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...) const;

    KRYS_NODISCARD constexpr bool operator==(const JustifyContent &) const noexcept = default;

    KRYS_NODISCARD StyleContentAlignmentData Resolve(Maybe<StyleContentAlignmentData> = null) const noexcept;

  private:
    enum class PrimaryKind : uint8
    {
      Normal,
      SpaceBetween,
      SpaceAround,
      SpaceEvenly,
      Stretch,
      Center,
      Start,
      End,
      FlexStart,
      FlexEnd,
      Left,
      Right,
    };

    KRYS_NODISCARD constexpr static bool IsContentPosition(PrimaryKind) noexcept;

    constexpr JustifyContent(PrimaryKind) noexcept;
    constexpr JustifyContent(PrimaryKind, Maybe<OverflowPosition>) noexcept;

    KRYS_NODISCARD constexpr PrimaryKind Primary() const noexcept;
    KRYS_NODISCARD constexpr OverflowPositionKind GetOverflowPosition() const noexcept;

    uint8 _primary : 4 {static_cast<uint8>(PrimaryKind::Normal)};
    uint8 _secondary : 2 {0}; // unused or OverflowPositionKind, depending on value of _primary
  };
  static_assert(sizeof(JustifyContent) == 1);

  constexpr JustifyContent::JustifyContent(css::keywords::Normal) noexcept
      : JustifyContent {PrimaryKind::Normal}
  {
  }

  constexpr JustifyContent::JustifyContent(css::keywords::SpaceBetween) noexcept
      : JustifyContent {PrimaryKind::SpaceBetween}
  {
  }

  constexpr JustifyContent::JustifyContent(css::keywords::SpaceAround) noexcept
      : JustifyContent {PrimaryKind::SpaceAround}
  {
  }

  constexpr JustifyContent::JustifyContent(css::keywords::SpaceEvenly) noexcept
      : JustifyContent {PrimaryKind::SpaceEvenly}
  {
  }

  constexpr JustifyContent::JustifyContent(css::keywords::Stretch) noexcept
      : JustifyContent {PrimaryKind::Stretch}
  {
  }

  constexpr JustifyContent::JustifyContent(css::keywords::Center, Maybe<OverflowPosition> overflow) noexcept
      : JustifyContent {PrimaryKind::Center, overflow}
  {
  }

  constexpr JustifyContent::JustifyContent(css::keywords::Start, Maybe<OverflowPosition> overflow) noexcept
      : JustifyContent {PrimaryKind::Start, overflow}
  {
  }

  constexpr JustifyContent::JustifyContent(css::keywords::End, Maybe<OverflowPosition> overflow) noexcept
      : JustifyContent {PrimaryKind::End, overflow}
  {
  }

  constexpr JustifyContent::JustifyContent(css::keywords::FlexStart,
                                           Maybe<OverflowPosition> overflow) noexcept
      : JustifyContent {PrimaryKind::FlexStart, overflow}
  {
  }

  constexpr JustifyContent::JustifyContent(css::keywords::FlexEnd, Maybe<OverflowPosition> overflow) noexcept
      : JustifyContent {PrimaryKind::FlexEnd, overflow}
  {
  }

  constexpr JustifyContent::JustifyContent(css::keywords::Left, Maybe<OverflowPosition> overflow) noexcept
      : JustifyContent {PrimaryKind::Left, overflow}
  {
  }

  constexpr JustifyContent::JustifyContent(css::keywords::Right, Maybe<OverflowPosition> overflow) noexcept
      : JustifyContent {PrimaryKind::Right, overflow}
  {
  }

  constexpr JustifyContent::JustifyContent(PrimaryKind primary) noexcept
      : _primary {static_cast<uint8>(primary)}
  {
    krys_debug_assert(!IsContentPosition(primary));
  }

  constexpr JustifyContent::JustifyContent(PrimaryKind primary, Maybe<OverflowPosition> overflow) noexcept
      : _primary {static_cast<uint8>(primary)}, _secondary {static_cast<uint8>(ComputeKind(overflow))}
  {
    krys_debug_assert(IsContentPosition(primary));
  }

  constexpr JustifyContent::PrimaryKind JustifyContent::Primary() const noexcept
  {
    return static_cast<PrimaryKind>(_primary);
  }

  constexpr OverflowPositionKind JustifyContent::GetOverflowPosition() const noexcept
  {
    krys_assert(IsContentPosition(Primary()));
    return static_cast<OverflowPositionKind>(_secondary);
  }

  constexpr bool JustifyContent::IsContentPosition(PrimaryKind primary) noexcept
  {
    switch (primary)
    {
      case PrimaryKind::Normal:
      case PrimaryKind::SpaceBetween:
      case PrimaryKind::SpaceAround:
      case PrimaryKind::SpaceEvenly:
      case PrimaryKind::Stretch:      return false;
      case PrimaryKind::Center:
      case PrimaryKind::Start:
      case PrimaryKind::End:
      case PrimaryKind::FlexStart:
      case PrimaryKind::FlexEnd:
      case PrimaryKind::Left:
      case PrimaryKind::Right:        return true;
    }
    krys_unreachable();
  }

  template <typename... F>
  constexpr decltype(auto) JustifyContent::SwitchOn(F &&...f) const
  {
    auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

    auto visitContentPosition = [&](auto primaryKeyword)
    {
      switch (GetOverflowPosition())
      {
        case OverflowPositionKind::None: return visitor(primaryKeyword);
        case OverflowPositionKind::Unsafe:
          return visitor(css::SpaceSeparatedTuple {css::keywords::Unsafe {}, primaryKeyword});
        case OverflowPositionKind::Safe:
          return visitor(css::SpaceSeparatedTuple {css::keywords::Safe {}, primaryKeyword});
      }
      krys_unreachable();
    };

    switch (Primary())
    {
      case PrimaryKind::Normal:       return visitor(css::keywords::Normal {});
      case PrimaryKind::SpaceBetween: return visitor(css::keywords::SpaceBetween {});
      case PrimaryKind::SpaceAround:  return visitor(css::keywords::SpaceAround {});
      case PrimaryKind::SpaceEvenly:  return visitor(css::keywords::SpaceEvenly {});
      case PrimaryKind::Stretch:      return visitor(css::keywords::Stretch {});
      case PrimaryKind::Center:       return visitContentPosition(css::keywords::Center {});
      case PrimaryKind::Start:        return visitContentPosition(css::keywords::Start {});
      case PrimaryKind::End:          return visitContentPosition(css::keywords::End {});
      case PrimaryKind::FlexStart:    return visitContentPosition(css::keywords::FlexStart {});
      case PrimaryKind::FlexEnd:      return visitContentPosition(css::keywords::FlexEnd {});
      case PrimaryKind::Left:         return visitContentPosition(css::keywords::Left {});
      case PrimaryKind::Right:        return visitContentPosition(css::keywords::Right {});
    }
    krys_unreachable();
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<JustifyContent>
  {
    KRYS_NODISCARD JustifyContent operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::JustifyContent)

#pragma once

#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"
#include "Krystal.Booey/Style/Values/Align/BaselineAlignmentPreference.hpp"
#include "Krystal.Booey/Style/Values/Align/OverflowPosition.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'justify-self'> = auto | normal | stretch | <baseline-position> | <overflow-position>? [ <self-position>
  // | left | right ] https://drafts.csswg.org/css-align/#propdef-justify-self Additional values,
  // `anchor-center` and `dialog` added to <self-position> by CSS Anchor Positioning.
  // FIXME: Add support for `dialog`.
  // https://drafts.csswg.org/css-anchor-position-1/#anchor-center
  struct JustifySelf
  {
    constexpr JustifySelf(css::keywords::Auto) noexcept;
    constexpr JustifySelf(css::keywords::Normal) noexcept;
    constexpr JustifySelf(css::keywords::Stretch) noexcept;
    constexpr JustifySelf(css::keywords::Baseline, Maybe<BaselineAlignmentPreference> = null) noexcept;
    constexpr JustifySelf(css::keywords::Center, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifySelf(css::keywords::Start, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifySelf(css::keywords::End, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifySelf(css::keywords::SelfStart, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifySelf(css::keywords::SelfEnd, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifySelf(css::keywords::FlexStart, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifySelf(css::keywords::FlexEnd, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifySelf(css::keywords::Left, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifySelf(css::keywords::Right, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifySelf(css::keywords::AnchorCenter, Maybe<OverflowPosition> = null) noexcept;

    KRYS_NODISCARD constexpr bool IsAuto() const noexcept
    {
      return Primary() == PrimaryKind::Auto;
    }
    KRYS_NODISCARD constexpr bool IsNormal() const noexcept
    {
      return Primary() == PrimaryKind::Normal;
    }
    KRYS_NODISCARD constexpr bool IsStretch() const noexcept
    {
      return Primary() == PrimaryKind::Stretch;
    }
    KRYS_NODISCARD constexpr bool IsBaseline() const noexcept
    {
      return Primary() == PrimaryKind::Baseline;
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
    KRYS_NODISCARD constexpr bool IsSelfStart() const noexcept
    {
      return Primary() == PrimaryKind::SelfStart;
    }
    KRYS_NODISCARD constexpr bool IsSelfEnd() const noexcept
    {
      return Primary() == PrimaryKind::SelfEnd;
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
    KRYS_NODISCARD constexpr bool IsAnchorCenter() const noexcept
    {
      return Primary() == PrimaryKind::AnchorCenter;
    }

    KRYS_NODISCARD constexpr bool IsFirstBaseline() const noexcept
    {
      return Primary() == PrimaryKind::Baseline
             && GetBaselineAlignmentPreference() == BaselineAlignmentPreferenceKind::First;
    }
    KRYS_NODISCARD constexpr bool IsLastBaseline() const noexcept
    {
      return Primary() == PrimaryKind::Baseline
             && GetBaselineAlignmentPreference() == BaselineAlignmentPreferenceKind::Last;
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...) const;

    KRYS_NODISCARD constexpr bool operator==(const JustifySelf &) const noexcept = default;

    KRYS_NODISCARD StyleSelfAlignmentData Resolve(Maybe<StyleSelfAlignmentData> = null) const noexcept;

  private:
    enum class PrimaryKind : uint8
    {
      Auto,
      Normal,
      Stretch,
      Baseline,
      Center,
      Start,
      End,
      SelfStart,
      SelfEnd,
      FlexStart,
      FlexEnd,
      Left,
      Right,
      AnchorCenter,
    };

    KRYS_NODISCARD constexpr static bool CanHaveBaselinePosition(PrimaryKind) noexcept;
    KRYS_NODISCARD constexpr static bool CanHaveOverflowPosition(PrimaryKind) noexcept;

    constexpr JustifySelf(PrimaryKind) noexcept;
    constexpr JustifySelf(PrimaryKind, Maybe<BaselineAlignmentPreference>) noexcept;
    constexpr JustifySelf(PrimaryKind, Maybe<OverflowPosition>) noexcept;

    KRYS_NODISCARD constexpr PrimaryKind Primary() const noexcept;
    KRYS_NODISCARD constexpr BaselineAlignmentPreferenceKind GetBaselineAlignmentPreference() const noexcept;
    KRYS_NODISCARD constexpr OverflowPositionKind GetOverflowPosition() const noexcept;

    uint8 _primary : 4 {static_cast<uint8>(PrimaryKind::Normal)};
    uint8 _secondary : 2 {
      0}; // unused, BaselineAlignmentPreferenceKind or OverflowPositionKind, depending on value of _primary
  };
  static_assert(sizeof(JustifySelf) == 1);

  constexpr JustifySelf::JustifySelf(css::keywords::Auto) noexcept : JustifySelf {PrimaryKind::Auto}
  {
  }

  constexpr JustifySelf::JustifySelf(css::keywords::Normal) noexcept : JustifySelf {PrimaryKind::Normal}
  {
  }

  constexpr JustifySelf::JustifySelf(css::keywords::Stretch) noexcept : JustifySelf {PrimaryKind::Stretch}
  {
  }

  constexpr JustifySelf::JustifySelf(css::keywords::Baseline,
                                     Maybe<BaselineAlignmentPreference> preference) noexcept
      : JustifySelf {PrimaryKind::Baseline, preference}
  {
  }

  constexpr JustifySelf::JustifySelf(css::keywords::Center, Maybe<OverflowPosition> overflow) noexcept
      : JustifySelf {PrimaryKind::Center, overflow}
  {
  }

  constexpr JustifySelf::JustifySelf(css::keywords::Start, Maybe<OverflowPosition> overflow) noexcept
      : JustifySelf {PrimaryKind::Start, overflow}
  {
  }

  constexpr JustifySelf::JustifySelf(css::keywords::End, Maybe<OverflowPosition> overflow) noexcept
      : JustifySelf {PrimaryKind::End, overflow}
  {
  }

  constexpr JustifySelf::JustifySelf(css::keywords::SelfStart, Maybe<OverflowPosition> overflow) noexcept
      : JustifySelf {PrimaryKind::SelfStart, overflow}
  {
  }

  constexpr JustifySelf::JustifySelf(css::keywords::SelfEnd, Maybe<OverflowPosition> overflow) noexcept
      : JustifySelf {PrimaryKind::SelfEnd, overflow}
  {
  }

  constexpr JustifySelf::JustifySelf(css::keywords::FlexStart, Maybe<OverflowPosition> overflow) noexcept
      : JustifySelf {PrimaryKind::FlexStart, overflow}
  {
  }

  constexpr JustifySelf::JustifySelf(css::keywords::FlexEnd, Maybe<OverflowPosition> overflow) noexcept
      : JustifySelf {PrimaryKind::FlexEnd, overflow}
  {
  }

  constexpr JustifySelf::JustifySelf(css::keywords::Left, Maybe<OverflowPosition> overflow) noexcept
      : JustifySelf {PrimaryKind::Left, overflow}
  {
  }

  constexpr JustifySelf::JustifySelf(css::keywords::Right, Maybe<OverflowPosition> overflow) noexcept
      : JustifySelf {PrimaryKind::Right, overflow}
  {
  }

  constexpr JustifySelf::JustifySelf(css::keywords::AnchorCenter, Maybe<OverflowPosition> overflow) noexcept
      : JustifySelf {PrimaryKind::AnchorCenter, overflow}
  {
  }

  constexpr JustifySelf::JustifySelf(PrimaryKind primary) noexcept : _primary {static_cast<uint8>(primary)}
  {
    krys_debug_assert(!CanHaveBaselinePosition(primary));
    krys_debug_assert(!CanHaveOverflowPosition(primary));
  }

  constexpr JustifySelf::JustifySelf(PrimaryKind primary,
                                     Maybe<BaselineAlignmentPreference> preference) noexcept
      : _primary {static_cast<uint8>(primary)}, _secondary {static_cast<uint8>(ComputeKind(preference))}
  {
    krys_debug_assert(CanHaveBaselinePosition(primary));
  }

  constexpr JustifySelf::JustifySelf(PrimaryKind primary, Maybe<OverflowPosition> overflow) noexcept
      : _primary {static_cast<uint8>(primary)}, _secondary {static_cast<uint8>(ComputeKind(overflow))}
  {
    krys_debug_assert(CanHaveOverflowPosition(primary));
  }

  constexpr JustifySelf::PrimaryKind JustifySelf::Primary() const noexcept
  {
    return static_cast<PrimaryKind>(_primary);
  }

  constexpr BaselineAlignmentPreferenceKind JustifySelf::GetBaselineAlignmentPreference() const noexcept
  {
    krys_assert(CanHaveBaselinePosition(Primary()));
    return static_cast<BaselineAlignmentPreferenceKind>(_secondary);
  }

  constexpr OverflowPositionKind JustifySelf::GetOverflowPosition() const noexcept
  {
    krys_assert(CanHaveOverflowPosition(Primary()));
    return static_cast<OverflowPositionKind>(_secondary);
  }

  constexpr bool JustifySelf::CanHaveBaselinePosition(PrimaryKind primary) noexcept
  {
    return primary == PrimaryKind::Baseline;
  }

  constexpr bool JustifySelf::CanHaveOverflowPosition(PrimaryKind primary) noexcept
  {
    switch (primary)
    {
      case PrimaryKind::Auto:
      case PrimaryKind::Normal:
      case PrimaryKind::Stretch:
      case PrimaryKind::Baseline:     return false;
      case PrimaryKind::Center:
      case PrimaryKind::Start:
      case PrimaryKind::End:
      case PrimaryKind::SelfStart:
      case PrimaryKind::SelfEnd:
      case PrimaryKind::FlexStart:
      case PrimaryKind::FlexEnd:
      case PrimaryKind::Left:
      case PrimaryKind::Right:
      case PrimaryKind::AnchorCenter: return true;
    }
    krys_unreachable();
  }

  template <typename... F>
  constexpr decltype(auto) JustifySelf::SwitchOn(F &&...f) const
  {
    auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

    switch (Primary())
    {
      case PrimaryKind::Auto:    return visitor(css::keywords::Auto {});
      case PrimaryKind::Normal:  return visitor(css::keywords::Normal {});
      case PrimaryKind::Stretch: return visitor(css::keywords::Stretch {});
      case PrimaryKind::Baseline:
        return VisitBaselineAlignmentPreference(css::keywords::Baseline {}, GetBaselineAlignmentPreference(),
                                                visitor);
      case PrimaryKind::Center:
        return VisitOverflowPosition(css::keywords::Center {}, GetOverflowPosition(), visitor);
      case PrimaryKind::Start:
        return VisitOverflowPosition(css::keywords::Start {}, GetOverflowPosition(), visitor);
      case PrimaryKind::End:
        return VisitOverflowPosition(css::keywords::End {}, GetOverflowPosition(), visitor);
      case PrimaryKind::SelfStart:
        return VisitOverflowPosition(css::keywords::SelfStart {}, GetOverflowPosition(), visitor);
      case PrimaryKind::SelfEnd:
        return VisitOverflowPosition(css::keywords::SelfEnd {}, GetOverflowPosition(), visitor);
      case PrimaryKind::FlexStart:
        return VisitOverflowPosition(css::keywords::FlexStart {}, GetOverflowPosition(), visitor);
      case PrimaryKind::FlexEnd:
        return VisitOverflowPosition(css::keywords::FlexEnd {}, GetOverflowPosition(), visitor);
      case PrimaryKind::Left:
        return VisitOverflowPosition(css::keywords::Left {}, GetOverflowPosition(), visitor);
      case PrimaryKind::Right:
        return VisitOverflowPosition(css::keywords::Right {}, GetOverflowPosition(), visitor);
      case PrimaryKind::AnchorCenter:
        return VisitOverflowPosition(css::keywords::AnchorCenter {}, GetOverflowPosition(), visitor);
    }
    krys_unreachable();
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<JustifySelf>
  {
    KRYS_NODISCARD JustifySelf operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::JustifySelf)

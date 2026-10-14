#pragma once

#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"
#include "Krystal.Booey/Style/Values/Align/BaselineAlignmentPreference.hpp"
#include "Krystal.Booey/Style/Values/Align/LegacyPosition.hpp"
#include "Krystal.Booey/Style/Values/Align/OverflowPosition.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'justify-items'> = normal | stretch | <baseline-position> | <overflow-position>? [ <self-position> |
  // left | right ] | legacy | legacy && [ left | right | center ]
  // https://drafts.csswg.org/css-align/#propdef-justify-items
  // Additional values, `anchor-center` and `dialog` added to <self-position> by CSS Anchor Positioning.
  // FIXME: Add support for `dialog`.
  // https://drafts.csswg.org/css-anchor-position-1/#anchor-center
  struct JustifyItems
  {
    constexpr JustifyItems(css::keywords::Normal) noexcept;
    constexpr JustifyItems(css::keywords::Stretch) noexcept;
    constexpr JustifyItems(css::keywords::Baseline, Maybe<BaselineAlignmentPreference> = null) noexcept;
    constexpr JustifyItems(css::keywords::Center, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyItems(css::keywords::Start, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyItems(css::keywords::End, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyItems(css::keywords::SelfStart, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyItems(css::keywords::SelfEnd, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyItems(css::keywords::FlexStart, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyItems(css::keywords::FlexEnd, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyItems(css::keywords::Left, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyItems(css::keywords::Right, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyItems(css::keywords::AnchorCenter, Maybe<OverflowPosition> = null) noexcept;
    constexpr JustifyItems(css::keywords::Legacy, Maybe<LegacyPosition> = null) noexcept;

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
    KRYS_NODISCARD constexpr bool IsLegacy() const noexcept
    {
      return Primary() == PrimaryKind::Legacy;
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

    KRYS_NODISCARD constexpr bool IsLegacyNone() const noexcept
    {
      return Primary() == PrimaryKind::Legacy && GetLegacyPosition() == LegacyPositionKind::None;
    }
    KRYS_NODISCARD constexpr bool IsLegacyLeft() const noexcept
    {
      return Primary() == PrimaryKind::Legacy && GetLegacyPosition() == LegacyPositionKind::Left;
    }
    KRYS_NODISCARD constexpr bool IsLegacyRight() const noexcept
    {
      return Primary() == PrimaryKind::Legacy && GetLegacyPosition() == LegacyPositionKind::Right;
    }
    KRYS_NODISCARD constexpr bool IsLegacyCenter() const noexcept
    {
      return Primary() == PrimaryKind::Legacy && GetLegacyPosition() == LegacyPositionKind::Center;
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...) const;

    KRYS_NODISCARD constexpr bool operator==(const JustifyItems &) const noexcept = default;

    KRYS_NODISCARD StyleSelfAlignmentData Resolve(Maybe<StyleSelfAlignmentData> = null) const noexcept;

  private:
    enum class PrimaryKind : uint8
    {
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
      Legacy,
    };

    KRYS_NODISCARD constexpr static bool CanHaveBaselinePosition(PrimaryKind) noexcept;
    KRYS_NODISCARD constexpr static bool CanHaveOverflowPosition(PrimaryKind) noexcept;
    KRYS_NODISCARD constexpr static bool CanHaveLegacyPosition(PrimaryKind) noexcept;

    constexpr JustifyItems(PrimaryKind) noexcept;
    constexpr JustifyItems(PrimaryKind, Maybe<BaselineAlignmentPreference>) noexcept;
    constexpr JustifyItems(PrimaryKind, Maybe<OverflowPosition>) noexcept;
    constexpr JustifyItems(PrimaryKind, Maybe<LegacyPosition>) noexcept;

    KRYS_NODISCARD constexpr PrimaryKind Primary() const noexcept;
    KRYS_NODISCARD constexpr BaselineAlignmentPreferenceKind GetBaselineAlignmentPreference() const noexcept;
    KRYS_NODISCARD constexpr OverflowPositionKind GetOverflowPosition() const noexcept;
    KRYS_NODISCARD constexpr LegacyPositionKind GetLegacyPosition() const noexcept;

    uint8 _primary : 4 {static_cast<uint8>(PrimaryKind::Normal)};
    uint8 _secondary : 2 {0}; // unused, BaselineAlignmentPreferenceKind, OverflowPositionKind or
                              // LegacyPositionKind, depending on value of _primary
  };
  static_assert(sizeof(JustifyItems) == 1);

  constexpr JustifyItems::JustifyItems(css::keywords::Normal) noexcept : JustifyItems {PrimaryKind::Normal}
  {
  }

  constexpr JustifyItems::JustifyItems(css::keywords::Stretch) noexcept : JustifyItems {PrimaryKind::Stretch}
  {
  }

  constexpr JustifyItems::JustifyItems(css::keywords::Baseline,
                                       Maybe<BaselineAlignmentPreference> preference) noexcept
      : JustifyItems {PrimaryKind::Baseline, preference}
  {
  }

  constexpr JustifyItems::JustifyItems(css::keywords::Center, Maybe<OverflowPosition> overflow) noexcept
      : JustifyItems {PrimaryKind::Center, overflow}
  {
  }

  constexpr JustifyItems::JustifyItems(css::keywords::Start, Maybe<OverflowPosition> overflow) noexcept
      : JustifyItems {PrimaryKind::Start, overflow}
  {
  }

  constexpr JustifyItems::JustifyItems(css::keywords::End, Maybe<OverflowPosition> overflow) noexcept
      : JustifyItems {PrimaryKind::End, overflow}
  {
  }

  constexpr JustifyItems::JustifyItems(css::keywords::SelfStart, Maybe<OverflowPosition> overflow) noexcept
      : JustifyItems {PrimaryKind::SelfStart, overflow}
  {
  }

  constexpr JustifyItems::JustifyItems(css::keywords::SelfEnd, Maybe<OverflowPosition> overflow) noexcept
      : JustifyItems {PrimaryKind::SelfEnd, overflow}
  {
  }

  constexpr JustifyItems::JustifyItems(css::keywords::FlexStart, Maybe<OverflowPosition> overflow) noexcept
      : JustifyItems {PrimaryKind::FlexStart, overflow}
  {
  }

  constexpr JustifyItems::JustifyItems(css::keywords::FlexEnd, Maybe<OverflowPosition> overflow) noexcept
      : JustifyItems {PrimaryKind::FlexEnd, overflow}
  {
  }

  constexpr JustifyItems::JustifyItems(css::keywords::Left, Maybe<OverflowPosition> overflow) noexcept
      : JustifyItems {PrimaryKind::Left, overflow}
  {
  }

  constexpr JustifyItems::JustifyItems(css::keywords::Right, Maybe<OverflowPosition> overflow) noexcept
      : JustifyItems {PrimaryKind::Right, overflow}
  {
  }

  constexpr JustifyItems::JustifyItems(css::keywords::AnchorCenter, Maybe<OverflowPosition> overflow) noexcept
      : JustifyItems {PrimaryKind::AnchorCenter, overflow}
  {
  }

  constexpr JustifyItems::JustifyItems(css::keywords::Legacy, Maybe<LegacyPosition> position) noexcept
      : JustifyItems {PrimaryKind::Legacy, position}
  {
  }

  constexpr JustifyItems::JustifyItems(PrimaryKind primary) noexcept : _primary {static_cast<uint8>(primary)}
  {
    krys_debug_assert(!CanHaveBaselinePosition(primary));
    krys_debug_assert(!CanHaveOverflowPosition(primary));
    krys_debug_assert(!CanHaveLegacyPosition(primary));
  }

  constexpr JustifyItems::JustifyItems(PrimaryKind primary,
                                       Maybe<BaselineAlignmentPreference> preference) noexcept
      : _primary {static_cast<uint8>(primary)}, _secondary {static_cast<uint8>(ComputeKind(preference))}
  {
    krys_debug_assert(CanHaveBaselinePosition(primary));
  }

  constexpr JustifyItems::JustifyItems(PrimaryKind primary, Maybe<OverflowPosition> overflow) noexcept
      : _primary {static_cast<uint8>(primary)}, _secondary {static_cast<uint8>(ComputeKind(overflow))}
  {
    krys_debug_assert(CanHaveOverflowPosition(primary));
  }

  constexpr JustifyItems::JustifyItems(PrimaryKind primary, Maybe<LegacyPosition> position) noexcept
      : _primary {static_cast<uint8>(primary)}, _secondary {static_cast<uint8>(ComputeKind(position))}
  {
    krys_debug_assert(CanHaveLegacyPosition(primary));
  }

  constexpr JustifyItems::PrimaryKind JustifyItems::Primary() const noexcept
  {
    return static_cast<PrimaryKind>(_primary);
  }

  constexpr BaselineAlignmentPreferenceKind JustifyItems::GetBaselineAlignmentPreference() const noexcept
  {
    krys_assert(CanHaveBaselinePosition(Primary()));
    return static_cast<BaselineAlignmentPreferenceKind>(_secondary);
  }

  constexpr OverflowPositionKind JustifyItems::GetOverflowPosition() const noexcept
  {
    krys_assert(CanHaveOverflowPosition(Primary()));
    return static_cast<OverflowPositionKind>(_secondary);
  }

  constexpr LegacyPositionKind JustifyItems::GetLegacyPosition() const noexcept
  {
    krys_assert(CanHaveLegacyPosition(Primary()));
    return static_cast<LegacyPositionKind>(_secondary);
  }

  constexpr bool JustifyItems::CanHaveBaselinePosition(PrimaryKind primary) noexcept
  {
    return primary == PrimaryKind::Baseline;
  }

  constexpr bool JustifyItems::CanHaveOverflowPosition(PrimaryKind primary) noexcept
  {
    switch (primary)
    {
      case PrimaryKind::Normal:
      case PrimaryKind::Stretch:
      case PrimaryKind::Baseline:
      case PrimaryKind::Legacy:       return false;
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

  constexpr bool JustifyItems::CanHaveLegacyPosition(PrimaryKind primary) noexcept
  {
    return primary == PrimaryKind::Legacy;
  }

  template <typename... F>
  constexpr decltype(auto) JustifyItems::SwitchOn(F &&...f) const
  {
    auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

    switch (Primary())
    {
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
      case PrimaryKind::Legacy:
        return VisitLegacyPosition(css::keywords::Legacy {}, GetLegacyPosition(), visitor);
    }
    krys_unreachable();
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<JustifyItems>
  {
    KRYS_NODISCARD JustifyItems operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::JustifyItems)

#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"
#include "Krystal.Booey/Style/Values/Align/BaselineAlignmentPreference.hpp"
#include "Krystal.Booey/Style/Values/Align/OverflowPosition.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'align-items'> = normal | stretch | <baseline-position> | <overflow-position>? <self-position>
  // https://drafts.csswg.org/css-align/#propdef-align-items
  // Additional values, `anchor-center` and `dialog` added to <self-position> by CSS Anchor Positioning.
  // FIXME: Add support for `dialog`.
  // https://drafts.csswg.org/css-anchor-position-1/#anchor-center
  struct AlignItems
  {
    constexpr AlignItems(css::keywords::Normal) noexcept;
    constexpr AlignItems(css::keywords::Stretch) noexcept;
    constexpr AlignItems(css::keywords::Baseline, Maybe<BaselineAlignmentPreference> = null) noexcept;
    constexpr AlignItems(css::keywords::Center, Maybe<OverflowPosition> = null) noexcept;
    constexpr AlignItems(css::keywords::Start, Maybe<OverflowPosition> = null) noexcept;
    constexpr AlignItems(css::keywords::End, Maybe<OverflowPosition> = null) noexcept;
    constexpr AlignItems(css::keywords::SelfStart, Maybe<OverflowPosition> = null) noexcept;
    constexpr AlignItems(css::keywords::SelfEnd, Maybe<OverflowPosition> = null) noexcept;
    constexpr AlignItems(css::keywords::FlexStart, Maybe<OverflowPosition> = null) noexcept;
    constexpr AlignItems(css::keywords::FlexEnd, Maybe<OverflowPosition> = null) noexcept;
    constexpr AlignItems(css::keywords::AnchorCenter, Maybe<OverflowPosition> = null) noexcept;

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

    KRYS_NODISCARD constexpr bool operator==(const AlignItems &) const noexcept = default;

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
      AnchorCenter,
    };

    KRYS_NODISCARD constexpr static bool CanHaveBaselinePosition(PrimaryKind) noexcept;
    KRYS_NODISCARD constexpr static bool CanHaveOverflowPosition(PrimaryKind) noexcept;

    constexpr AlignItems(PrimaryKind) noexcept;
    constexpr AlignItems(PrimaryKind, Maybe<BaselineAlignmentPreference>) noexcept;
    constexpr AlignItems(PrimaryKind, Maybe<OverflowPosition>) noexcept;

    KRYS_NODISCARD constexpr PrimaryKind Primary() const noexcept;
    KRYS_NODISCARD constexpr BaselineAlignmentPreferenceKind GetBaselineAlignmentPreference() const noexcept;
    KRYS_NODISCARD constexpr OverflowPositionKind GetOverflowPosition() const noexcept;

    uint8 _primary : 4 {static_cast<uint8>(PrimaryKind::Normal)};
    uint8 _secondary : 2 {
      0}; // unused, BaselineAlignmentPreferenceKind or OverflowPositionKind, depending on value of _primary
  };
  static_assert(sizeof(AlignItems) == 1);

  constexpr AlignItems::AlignItems(css::keywords::Normal) noexcept : AlignItems {PrimaryKind::Normal}
  {
  }

  constexpr AlignItems::AlignItems(css::keywords::Stretch) noexcept : AlignItems {PrimaryKind::Stretch}
  {
  }

  constexpr AlignItems::AlignItems(css::keywords::Baseline,
                                   Maybe<BaselineAlignmentPreference> preference) noexcept
      : AlignItems {PrimaryKind::Baseline, preference}
  {
  }

  constexpr AlignItems::AlignItems(css::keywords::Center, Maybe<OverflowPosition> overflow) noexcept
      : AlignItems {PrimaryKind::Center, overflow}
  {
  }

  constexpr AlignItems::AlignItems(css::keywords::Start, Maybe<OverflowPosition> overflow) noexcept
      : AlignItems {PrimaryKind::Start, overflow}
  {
  }

  constexpr AlignItems::AlignItems(css::keywords::End, Maybe<OverflowPosition> overflow) noexcept
      : AlignItems {PrimaryKind::End, overflow}
  {
  }

  constexpr AlignItems::AlignItems(css::keywords::SelfStart, Maybe<OverflowPosition> overflow) noexcept
      : AlignItems {PrimaryKind::SelfStart, overflow}
  {
  }

  constexpr AlignItems::AlignItems(css::keywords::SelfEnd, Maybe<OverflowPosition> overflow) noexcept
      : AlignItems {PrimaryKind::SelfEnd, overflow}
  {
  }

  constexpr AlignItems::AlignItems(css::keywords::FlexStart, Maybe<OverflowPosition> overflow) noexcept
      : AlignItems {PrimaryKind::FlexStart, overflow}
  {
  }

  constexpr AlignItems::AlignItems(css::keywords::FlexEnd, Maybe<OverflowPosition> overflow) noexcept
      : AlignItems {PrimaryKind::FlexEnd, overflow}
  {
  }

  constexpr AlignItems::AlignItems(css::keywords::AnchorCenter, Maybe<OverflowPosition> overflow) noexcept
      : AlignItems {PrimaryKind::AnchorCenter, overflow}
  {
  }

  constexpr AlignItems::AlignItems(PrimaryKind primary) noexcept : _primary {static_cast<uint8>(primary)}
  {
    krys_debug_assert(!CanHaveBaselinePosition(primary));
    krys_debug_assert(!CanHaveOverflowPosition(primary));
  }

  constexpr AlignItems::AlignItems(PrimaryKind primary,
                                   Maybe<BaselineAlignmentPreference> preference) noexcept
      : _primary {static_cast<uint8>(primary)}, _secondary {static_cast<uint8>(ComputeKind(preference))}
  {
    krys_debug_assert(CanHaveBaselinePosition(primary));
  }

  constexpr AlignItems::AlignItems(PrimaryKind primary, Maybe<OverflowPosition> overflow) noexcept
      : _primary {static_cast<uint8>(primary)}, _secondary {static_cast<uint8>(ComputeKind(overflow))}
  {
    krys_debug_assert(CanHaveOverflowPosition(primary));
  }

  constexpr AlignItems::PrimaryKind AlignItems::Primary() const noexcept
  {
    return static_cast<PrimaryKind>(_primary);
  }

  constexpr BaselineAlignmentPreferenceKind AlignItems::GetBaselineAlignmentPreference() const noexcept
  {
    krys_assert(CanHaveBaselinePosition(Primary()));
    return static_cast<BaselineAlignmentPreferenceKind>(_secondary);
  }

  constexpr OverflowPositionKind AlignItems::GetOverflowPosition() const noexcept
  {
    krys_assert(CanHaveOverflowPosition(Primary()));
    return static_cast<OverflowPositionKind>(_secondary);
  }

  constexpr bool AlignItems::CanHaveBaselinePosition(PrimaryKind primary) noexcept
  {
    return primary == PrimaryKind::Baseline;
  }

  constexpr bool AlignItems::CanHaveOverflowPosition(PrimaryKind primary) noexcept
  {
    switch (primary)
    {
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
      case PrimaryKind::AnchorCenter: return true;
    }
    krys_unreachable();
  }

  template <typename... F>
  constexpr decltype(auto) AlignItems::SwitchOn(F &&...f) const
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
      case PrimaryKind::AnchorCenter:
        return VisitOverflowPosition(css::keywords::AnchorCenter {}, GetOverflowPosition(), visitor);
    }
    krys_unreachable();
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<AlignItems>
  {
    KRYS_NODISCARD AlignItems operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::AlignItems)

#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Rendering/Style/StyleContentAlignmentData.hpp"
#include "Krystal.Booey/Style/Values/Align/BaselineAlignmentPreference.hpp"
#include "Krystal.Booey/Style/Values/Align/OverflowPosition.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'align-content'> = normal | <baseline-position> | <content-distribution> | <overflow-position>?
  // <content-position> https://drafts.csswg.org/css-align/#propdef-align-content
  struct AlignContent
  {
    constexpr AlignContent(css::keywords::Normal) noexcept;
    constexpr AlignContent(css::keywords::Baseline, Maybe<BaselineAlignmentPreference> = null) noexcept;
    constexpr AlignContent(css::keywords::SpaceBetween) noexcept;
    constexpr AlignContent(css::keywords::SpaceAround) noexcept;
    constexpr AlignContent(css::keywords::SpaceEvenly) noexcept;
    constexpr AlignContent(css::keywords::Stretch) noexcept;
    constexpr AlignContent(css::keywords::Center, Maybe<OverflowPosition> = null) noexcept;
    constexpr AlignContent(css::keywords::Start, Maybe<OverflowPosition> = null) noexcept;
    constexpr AlignContent(css::keywords::End, Maybe<OverflowPosition> = null) noexcept;
    constexpr AlignContent(css::keywords::FlexStart, Maybe<OverflowPosition> = null) noexcept;
    constexpr AlignContent(css::keywords::FlexEnd, Maybe<OverflowPosition> = null) noexcept;

    KRYS_NODISCARD constexpr bool IsNormal() const noexcept
    {
      return Primary() == PrimaryKind::Normal;
    }
    KRYS_NODISCARD constexpr bool IsBaseline() const noexcept
    {
      return Primary() == PrimaryKind::Baseline;
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

    KRYS_NODISCARD constexpr bool operator==(const AlignContent &) const noexcept = default;

    KRYS_NODISCARD StyleContentAlignmentData Resolve(Maybe<StyleContentAlignmentData> = null) const noexcept;

  private:
    enum class PrimaryKind : uint8
    {
      Normal,
      Baseline,
      SpaceBetween,
      SpaceAround,
      SpaceEvenly,
      Stretch,
      Center,
      Start,
      End,
      FlexStart,
      FlexEnd,
    };

    KRYS_NODISCARD constexpr static bool CanHaveBaselinePosition(PrimaryKind) noexcept;
    KRYS_NODISCARD constexpr static bool IsContentPosition(PrimaryKind) noexcept;

    constexpr AlignContent(PrimaryKind) noexcept;
    constexpr AlignContent(PrimaryKind, Maybe<BaselineAlignmentPreference>) noexcept;
    constexpr AlignContent(PrimaryKind, Maybe<OverflowPosition>) noexcept;

    KRYS_NODISCARD constexpr PrimaryKind Primary() const noexcept;
    KRYS_NODISCARD constexpr BaselineAlignmentPreferenceKind GetBaselineAlignmentPreference() const noexcept;
    KRYS_NODISCARD constexpr OverflowPositionKind GetOverflowPosition() const noexcept;

    uint8 _primary : 4 {static_cast<uint8>(PrimaryKind::Normal)};
    uint8 _secondary : 2 {
      0}; // unused, BaselineAlignmentPreferenceKind or OverflowPositionKind, depending on value of _primary
  };
  static_assert(sizeof(AlignContent) == 1);

  constexpr AlignContent::AlignContent(css::keywords::Normal) noexcept : AlignContent {PrimaryKind::Normal}
  {
  }

  constexpr AlignContent::AlignContent(css::keywords::Baseline,
                                       Maybe<BaselineAlignmentPreference> preference) noexcept
      : AlignContent {PrimaryKind::Baseline, preference}
  {
  }

  constexpr AlignContent::AlignContent(css::keywords::SpaceBetween) noexcept
      : AlignContent {PrimaryKind::SpaceBetween}
  {
  }

  constexpr AlignContent::AlignContent(css::keywords::SpaceAround) noexcept
      : AlignContent {PrimaryKind::SpaceAround}
  {
  }
  constexpr AlignContent::AlignContent(css::keywords::SpaceEvenly) noexcept
      : AlignContent {PrimaryKind::SpaceEvenly}
  {
  }

  constexpr AlignContent::AlignContent(css::keywords::Stretch) noexcept : AlignContent {PrimaryKind::Stretch}
  {
  }

  constexpr AlignContent::AlignContent(css::keywords::Center, Maybe<OverflowPosition> overflow) noexcept
      : AlignContent {PrimaryKind::Center, overflow}
  {
  }

  constexpr AlignContent::AlignContent(css::keywords::Start, Maybe<OverflowPosition> overflow) noexcept
      : AlignContent {PrimaryKind::Start, overflow}
  {
  }

  constexpr AlignContent::AlignContent(css::keywords::End, Maybe<OverflowPosition> overflow) noexcept
      : AlignContent {PrimaryKind::End, overflow}
  {
  }

  constexpr AlignContent::AlignContent(css::keywords::FlexStart, Maybe<OverflowPosition> overflow) noexcept
      : AlignContent {PrimaryKind::FlexStart, overflow}
  {
  }

  constexpr AlignContent::AlignContent(css::keywords::FlexEnd, Maybe<OverflowPosition> overflow) noexcept
      : AlignContent {PrimaryKind::FlexEnd, overflow}
  {
  }

  constexpr AlignContent::AlignContent(PrimaryKind primary) noexcept : _primary {static_cast<uint8>(primary)}
  {
    krys_debug_assert(!CanHaveBaselinePosition(primary));
    krys_debug_assert(!IsContentPosition(primary));
  }

  constexpr AlignContent::AlignContent(PrimaryKind primary,
                                       Maybe<BaselineAlignmentPreference> preference) noexcept
      : _primary {static_cast<uint8>(primary)}, _secondary {static_cast<uint8>(ComputeKind(preference))}
  {
    krys_debug_assert(CanHaveBaselinePosition(primary));
  }

  constexpr AlignContent::AlignContent(PrimaryKind primary, Maybe<OverflowPosition> overflow) noexcept
      : _primary {static_cast<uint8>(primary)}, _secondary {static_cast<uint8>(ComputeKind(overflow))}
  {
    krys_debug_assert(IsContentPosition(primary));
  }

  constexpr AlignContent::PrimaryKind AlignContent::Primary() const noexcept
  {
    return static_cast<PrimaryKind>(_primary);
  }

  constexpr BaselineAlignmentPreferenceKind AlignContent::GetBaselineAlignmentPreference() const noexcept
  {
    krys_assert(CanHaveBaselinePosition(Primary()));
    return static_cast<BaselineAlignmentPreferenceKind>(_secondary);
  }

  constexpr OverflowPositionKind AlignContent::GetOverflowPosition() const noexcept
  {
    krys_assert(IsContentPosition(Primary()));
    return static_cast<OverflowPositionKind>(_secondary);
  }

  constexpr bool AlignContent::CanHaveBaselinePosition(PrimaryKind primary) noexcept
  {
    return primary == PrimaryKind::Baseline;
  }

  constexpr bool AlignContent::IsContentPosition(PrimaryKind primary) noexcept
  {
    switch (primary)
    {
      case PrimaryKind::Normal:
      case PrimaryKind::Baseline:
      case PrimaryKind::SpaceBetween:
      case PrimaryKind::SpaceAround:
      case PrimaryKind::SpaceEvenly:
      case PrimaryKind::Stretch:      return false;
      case PrimaryKind::Center:
      case PrimaryKind::Start:
      case PrimaryKind::End:
      case PrimaryKind::FlexStart:
      case PrimaryKind::FlexEnd:      return true;
    }
    krys_unreachable();
  }

  template <typename... F>
  constexpr decltype(auto) AlignContent::SwitchOn(F &&...f) const
  {
    auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

    switch (Primary())
    {
      case PrimaryKind::Normal: return visitor(css::keywords::Normal {});
      case PrimaryKind::Baseline:
        return VisitBaselineAlignmentPreference(css::keywords::Baseline {}, GetBaselineAlignmentPreference(),
                                                visitor);
      case PrimaryKind::SpaceBetween: return visitor(css::keywords::SpaceBetween {});
      case PrimaryKind::SpaceAround:  return visitor(css::keywords::SpaceAround {});
      case PrimaryKind::SpaceEvenly:  return visitor(css::keywords::SpaceEvenly {});
      case PrimaryKind::Stretch:      return visitor(css::keywords::Stretch {});
      case PrimaryKind::Center:
        return VisitOverflowPosition(css::keywords::Center {}, GetOverflowPosition(), visitor);
      case PrimaryKind::Start:
        return VisitOverflowPosition(css::keywords::Start {}, GetOverflowPosition(), visitor);
      case PrimaryKind::End:
        return VisitOverflowPosition(css::keywords::End {}, GetOverflowPosition(), visitor);
      case PrimaryKind::FlexStart:
        return VisitOverflowPosition(css::keywords::FlexStart {}, GetOverflowPosition(), visitor);
      case PrimaryKind::FlexEnd:
        return VisitOverflowPosition(css::keywords::FlexEnd {}, GetOverflowPosition(), visitor);
    }
    krys_unreachable();
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<AlignContent>
  {
    KRYS_NODISCARD AlignContent operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::AlignContent)

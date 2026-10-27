#pragma once

#include "Krystal.Booey/Page/IntersectionObserverMarginBox.hpp"
#include "Krystal.Booey/Rendering/RepaintRectCalculation.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief How a rect is mapped up to a container: which clips and scroll positions apply.
  enum class VisibleRectContextOption : uint8
  {
    UseEdgeInclusiveIntersection,
    ApplyCompositedClips,
    ApplyCompositedContainerScrolls,
    ApplyContainerClip,
    CalculateAccurateRepaintRect
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::VisibleRectContextOption, 5uz);

namespace krys::boo
{
  /// @brief The state of mapping a rect up to a container, through `ComputeVisibleRectsInContainer`.
  struct VisibleRectContext
  {
    using Option = VisibleRectContextOption;

    bool HasPositionFixedDescendant {false};
    bool DirtyRectIsFlipped {false};
    bool DescendantNeedsEnclosingIntRect {false};
    EnumSet<Option> Options {};
    /// @brief An intersection observer's scroll margin, which grows the clips of the scroll containers on the
    /// way.
    Maybe<IntersectionObserverMarginBox> ScrollMargin {};

    KRYS_NODISCARD RepaintRectCalculation GetRepaintRectCalculation() const noexcept
    {
      return Options.Contains(Option::CalculateAccurateRepaintRect) ? RepaintRectCalculation::Accurate
                                                                    : RepaintRectCalculation::Fast;
    }
  };
}

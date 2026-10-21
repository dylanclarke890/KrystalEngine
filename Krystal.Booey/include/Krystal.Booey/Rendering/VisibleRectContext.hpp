#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

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
  // FIXME: The scroll margin, once there are intersection observers.
  struct VisibleRectContext
  {
    using Option = VisibleRectContextOption;

    bool HasPositionFixedDescendant {false};
    bool DirtyRectIsFlipped {false};
    bool DescendantNeedsEnclosingIntRect {false};
    EnumSet<Option> Options {};
  };
}

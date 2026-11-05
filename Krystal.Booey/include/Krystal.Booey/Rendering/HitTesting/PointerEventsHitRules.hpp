#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  class HitTestRequest;

  enum class PointerEventsHitTestingTargetType : uint8
  {
    SVGImage,
    SVGPath,
    SVGText
  };

  /// @brief What `pointer-events` lets SVG graphics be hit by: their fill, stroke or bounding box, and
  /// whether they must be visible or painted.
  class PointerEventsHitRules
  {
  public:
    using HitTestingTargetType = PointerEventsHitTestingTargetType;

    PointerEventsHitRules(HitTestingTargetType, const HitTestRequest &, PointerEvents) noexcept;

    bool RequireVisible {false};
    bool RequireFill {false};
    bool RequireStroke {false};
    bool CanHitStroke {false};
    bool CanHitFill {false};
    bool CanHitBoundingBox {false};
  };
}

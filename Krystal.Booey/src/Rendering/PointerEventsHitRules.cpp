#include "Krystal.Booey/Rendering/PointerEventsHitRules.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"

namespace krys::boo
{
  PointerEventsHitRules::PointerEventsHitRules(HitTestingTargetType hitTestingTargetType,
                                               const HitTestRequest &request,
                                               PointerEvents pointerEvents) noexcept
  {
    if (request.SVGClipContent())
    {
      pointerEvents = PointerEvents::Fill;
    }

    if (hitTestingTargetType == HitTestingTargetType::SVGPath)
    {
      switch (pointerEvents)
      {
        case PointerEvents::VisiblePainted:
        case PointerEvents::Auto: // "auto" is like "visiblePainted" when in SVG content
          RequireFill = true;
          RequireStroke = true;
          [[fallthrough]];
        case PointerEvents::Visible:
          RequireVisible = true;
          CanHitFill = true;
          CanHitStroke = true;
          break;
        case PointerEvents::VisibleFill:
          RequireVisible = true;
          CanHitFill = true;
          break;
        case PointerEvents::VisibleStroke:
          RequireVisible = true;
          CanHitStroke = true;
          break;
        case PointerEvents::Painted:
          RequireFill = true;
          RequireStroke = true;
          [[fallthrough]];
        case PointerEvents::All:
          CanHitFill = true;
          CanHitStroke = true;
          break;
        case PointerEvents::Fill:   CanHitFill = true; break;
        case PointerEvents::Stroke: CanHitStroke = true; break;
        case PointerEvents::BoundingBox:
          CanHitFill = true;
          CanHitBoundingBox = true;
          break;
        case PointerEvents::None:
          // nothing to do here, defaults are all false.
          break;
      }
    }
    else
    {
      switch (pointerEvents)
      {
        case PointerEvents::VisiblePainted:
        case PointerEvents::Auto: // "auto" is like "visiblePainted" when in SVG content
          RequireVisible = true;
          RequireFill = true;
          RequireStroke = true;
          CanHitFill = true;
          CanHitStroke = true;
          break;
        case PointerEvents::VisibleFill:
        case PointerEvents::VisibleStroke:
        case PointerEvents::Visible:
          RequireVisible = true;
          CanHitFill = true;
          CanHitStroke = true;
          break;
        case PointerEvents::Painted:
          RequireFill = true;
          RequireStroke = true;
          CanHitFill = true;
          CanHitStroke = true;
          break;
        case PointerEvents::Fill:
        case PointerEvents::Stroke:
        case PointerEvents::All:
          CanHitFill = true;
          CanHitStroke = true;
          break;
        case PointerEvents::BoundingBox:
          CanHitFill = true;
          CanHitBoundingBox = true;
          break;
        case PointerEvents::None:
          // nothing to do here, defaults are all false.
          break;
      }
    }
  }
}

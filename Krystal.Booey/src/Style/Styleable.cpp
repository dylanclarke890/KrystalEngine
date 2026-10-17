#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/Rendering/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::style
{
  RenderElement *Styleable::Renderer() const noexcept
  {
    if (!PseudoElementIdentifier)
    {
      return Element.Renderer();
    }

    switch (PseudoElementIdentifier->Type)
    {
      case PseudoElementType::After:
        if (auto *afterPseudoElement = Element.AfterPseudoElement())
        {
          return afterPseudoElement->Renderer();
        }
        break;
      case PseudoElementType::Before:
        if (auto *beforePseudoElement = Element.BeforePseudoElement())
        {
          return beforePseudoElement->Renderer();
        }
        break;
      case PseudoElementType::Marker:
        if (auto *renderListItem = DynamicDowncast<RenderListItem>(Element.Renderer()))
        {
          auto *markerRenderer = renderListItem->MarkerRenderer();
          if (markerRenderer && !markerRenderer->Style().HasUsedContentNone())
          {
            return markerRenderer;
          }
        }
        break;
      default:
        // FIXME: The ::backdrop renderer and the view transition pseudo-elements' renderers, once there are
        // those.
        break;
    }

    return nullptr;
  }
}

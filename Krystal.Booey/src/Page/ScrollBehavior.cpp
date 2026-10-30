#include "Krystal.Booey/Page/ScrollBehavior.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  bool UseSmoothScrolling(ScrollBehavior behavior, const dom::Element *associatedElement) noexcept
  {
    if (!associatedElement)
    {
      return false;
    }

    // FIXME: Should we use document()->scrollingElement()?
    // See https://bugs.webkit.org/show_bug.cgi?id=205059
    auto &document = associatedElement->NodeDocument();
    if (associatedElement == document.ScrollingElement().get())
    {
      associatedElement = document.DocumentElement().get();
    }

    if (!associatedElement || !associatedElement->Renderer())
    {
      return false;
    }

    switch (behavior)
    {
      case ScrollBehavior::Auto:
        return associatedElement->Renderer()->Style().GetScrollBehavior() == style::ScrollBehavior::Smooth;
      case ScrollBehavior::Instant: return false;
      case ScrollBehavior::Smooth:  return true;
    }
    return false;
  }
}

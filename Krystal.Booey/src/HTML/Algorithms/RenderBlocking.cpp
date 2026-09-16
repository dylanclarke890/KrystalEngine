#include "Krystal.Booey/HTML/Algorithms/RenderBlocking.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include <ranges>

namespace krys::boo::html
{
  bool RenderBlocking::AllowsAddingRenderBlockingElements(const dom::Document &document) noexcept
  {
    return document.ContentType() == u8"text/html" && document.Body() == nullptr;
  }

  bool RenderBlocking::IsRenderBlocked(const dom::Document &document) noexcept
  {
    return !document._renderBlockingElements.empty() || AllowsAddingRenderBlockingElements(document);
    // TODO(DOCUMENT, RENDER-BLOCKING): And if the current high resolution time given document's relevant
    // global object has not exceeded an implementation-defined timeout value. -
    // https://html.spec.whatwg.org/#render-blocked
  }

  bool RenderBlocking::IsRenderBlocking(const dom::Element &element, const dom::Document &document) noexcept
  {
    return std::ranges::any_of(document._renderBlockingElements, [&element](const auto &renderBlockingElement)
                               { return renderBlockingElement.get() == &element; });
  }

  void RenderBlocking::BlockRendering(dom::Element &element) noexcept
  {
    auto &document = element.NodeDocument();
    if (AllowsAddingRenderBlockingElements(document))
    {
      auto existing =
        std::ranges::find_if(document._renderBlockingElements, [&element](const auto &renderBlockingElement)
                             { return renderBlockingElement.get() == &element; });

      if (existing == std::ranges::end(document._renderBlockingElements))
      {
        document._renderBlockingElements.push_back(CreateWeakRef(element));
      }
    }
  }

  void RenderBlocking::UnblockRendering(dom::Element &element) noexcept
  {
    auto &document = element.NodeDocument();

    std::erase_if(document._renderBlockingElements, [&element](const auto &renderBlockingElement)
                  { return renderBlockingElement.get() == &element; });
  }
}
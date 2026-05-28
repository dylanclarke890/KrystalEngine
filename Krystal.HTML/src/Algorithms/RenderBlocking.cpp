#include "Krystal.HTML/Algorithms/RenderBlocking.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLBodyElement.hpp"
#include "Krystal.HTML/HTMLElement/HTMLHtmlElement.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include <ranges>

namespace Krys::HTML
{
  bool RenderBlocking::AllowsAddingRenderBlockingElements(const Document &document) noexcept
  {
    return document.ContentType() == u8"text/html" && document.Body() == nullptr;
  }

  bool RenderBlocking::IsRenderBlocked(const Document &document) noexcept
  {
    return !document._renderBlockingElements.empty() || AllowsAddingRenderBlockingElements(document);
    // TODO(impl): And if the current high resolution time given document's relevant global object has not
    // exceeded an implementation-defined timeout value.
  }

  bool RenderBlocking::IsRenderBlocking(const Element &element, const Document &document) noexcept
  {
    return std::ranges::any_of(document._renderBlockingElements, [&element](const auto &renderBlockingElement)
                               { return renderBlockingElement.get() == &element; });
  }

  void RenderBlocking::BlockRendering(Element &element) noexcept
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

  void RenderBlocking::UnblockRendering(Element &element) noexcept
  {
    auto &document = element.NodeDocument();

    std::erase_if(document._renderBlockingElements, [&element](const auto &renderBlockingElement)
                  { return renderBlockingElement.get() == &element; });
  }
}
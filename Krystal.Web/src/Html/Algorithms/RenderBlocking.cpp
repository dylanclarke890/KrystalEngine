#include "Krystal.Web/Html/Algorithms/RenderBlocking.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Html/Elements/HTMLBodyElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLHtmlElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLSlotElement.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/Html/Window.hpp"
#include "Krystal.Web/Infra/StringAlgorithms.hpp"
#include <ranges>

namespace krys::web::html
{
  bool RenderBlocking::HasRenderBlockingToken(const Element &element) noexcept
  {
    // 1. Let value be the value of el's blocking attribute, or the empty string if no such attribute exists.
    auto value = element.GetAttribute(u8"blocking").value_or(DOMString());

    // 2. Set value to value, converted to ASCII lowercase.
    // 3. Let rawTokens be the result of splitting value on ASCII whitespace.
    // 4. Return a set containing the elements of rawTokens that are possible blocking tokens.
    // The only possible blocking token is "render", so this is whether a token matches it ASCII
    // case-insensitively.
    for (auto &token : infra::StringAlgorithms::SplitOnWhitespace(value))
    {
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(token, u8"render"))
      {
        return true;
      }
    }
    return false;
  }

  bool RenderBlocking::AllowsAddingRenderBlockingElements(const Document &document) noexcept
  {
    return document.ContentType() == u8"text/html" && document.BodyOrFrameset() == nullptr;
  }

  bool RenderBlocking::IsRenderBlocked(const Document &document) noexcept
  {
    // A Document document is render-blocked if both of the following are true:
    // - document's render-blocking element set is not empty, or document allows adding render-blocking
    //   elements.
    // - The current high resolution time given document's relevant global object has not exceeded an
    //   implementation-defined timeout value.
    // (A document without a window has no time origin, so it doesn't time out.)
    if (auto *window = document.Window();
        window && EventLoop::Current().Now() - window->TimeOrigin() > Timeout)
    {
      return false;
    }
    return !document._renderBlockingElements.empty() || AllowsAddingRenderBlockingElements(document);
  }

  bool RenderBlocking::IsRenderBlocking(const Element &element) noexcept
  {
    // An element el is render-blocking if el's node document document is render-blocked, and el is in
    // document's render-blocking element set.
    auto &document = element.NodeDocument();
    return IsRenderBlocked(document)
           && std::ranges::any_of(document._renderBlockingElements,
                                  [&element](const auto &renderBlockingElement)
                                  { return renderBlockingElement.get() == &element; });
  }

  void RenderBlocking::BlockRendering(Element &element) noexcept
  {
    // 1. Let document be el's node document.
    auto &document = element.NodeDocument();

    // 2. If document allows adding render-blocking elements, then append el to document's render-blocking
    //    element set.
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
    // 1. Let document be el's node document.
    auto &document = element.NodeDocument();

    // 2. Remove el from document's render-blocking element set.
    std::erase_if(document._renderBlockingElements, [&element](const auto &renderBlockingElement)
                  { return renderBlockingElement.get() == &element; });
  }
}

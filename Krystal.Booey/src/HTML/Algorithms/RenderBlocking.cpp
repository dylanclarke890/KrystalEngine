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
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include <ranges>

namespace krys::boo::html
{
  bool RenderBlocking::HasRenderBlockingToken(const dom::Element &element) noexcept
  {
    // 1. Let value be the value of el's blocking attribute, or the empty string if no such attribute exists.
    auto value = element.GetAttribute(u8"blocking").value_or(dom::DOMString());

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

  bool RenderBlocking::AllowsAddingRenderBlockingElements(const dom::Document &document) noexcept
  {
    return document.ContentType() == u8"text/html" && document.BodyOrFrameset() == nullptr;
  }

  bool RenderBlocking::IsRenderBlocked(const dom::Document &document) noexcept
  {
    // A Document document is render-blocked if both of the following are true:
    // - document's render-blocking element set is not empty, or document allows adding render-blocking
    //   elements.
    // - The current high resolution time given document's relevant global object has not exceeded an
    //   implementation-defined timeout value.
    // FIXME: The timeout, once the rendering update skips render-blocked documents.
    return !document._renderBlockingElements.empty() || AllowsAddingRenderBlockingElements(document);
  }

  bool RenderBlocking::IsRenderBlocking(const dom::Element &element) noexcept
  {
    // An element el is render-blocking if el's node document document is render-blocked, and el is in
    // document's render-blocking element set.
    auto &document = element.NodeDocument();
    return IsRenderBlocked(document)
           && std::ranges::any_of(document._renderBlockingElements,
                                  [&element](const auto &renderBlockingElement)
                                  { return renderBlockingElement.get() == &element; });
  }

  void RenderBlocking::BlockRendering(dom::Element &element) noexcept
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

  void RenderBlocking::UnblockRendering(dom::Element &element) noexcept
  {
    // 1. Let document be el's node document.
    auto &document = element.NodeDocument();

    // 2. Remove el from document's render-blocking element set.
    std::erase_if(document._renderBlockingElements, [&element](const auto &renderBlockingElement)
                  { return renderBlockingElement.get() == &element; });
  }
}
#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Web/Dom/Algorithms/Iterators/SubtreeRanges.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Html/Elements/HTMLBodyElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLFrameSetElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLHeadElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLHtmlElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLTitleElement.hpp"

namespace krys::web::html
{
  /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-tree-accessors
  class DOMTreeAccessors
  {
  public:
    /// @brief The document element, if it's an html element.
    /// @see https://html.spec.whatwg.org/multipage/dom.html#the-html-element-2
    KRYS_NODISCARD static HTMLHtmlElement *GetHtmlElement(const Document &document) noexcept
    {
      auto documentElement = document.DocumentElement();
      if (!Is<HTMLHtmlElement>(documentElement))
      {
        return nullptr;
      }

      return const_cast<HTMLHtmlElement *>(&Downcast<HTMLHtmlElement>(*documentElement));
    }

    /// @brief The first head element that is a child of the html element.
    /// @see https://html.spec.whatwg.org/multipage/dom.html#the-head-element-2
    KRYS_NODISCARD static HTMLHeadElement *GetHeadElement(const Document &document) noexcept
    {
      auto *html = GetHtmlElement(document);
      if (html == nullptr)
      {
        return nullptr;
      }

      auto children = dom::ChildNodeRange(*html);
      auto head = dom::FirstOfType<HTMLHeadElement>(children);
      return head == std::ranges::end(children) ? nullptr : &Downcast<HTMLHeadElement>(*head);
    }

    /// @brief The first title element in the document, in tree order.
    /// @see https://html.spec.whatwg.org/multipage/dom.html#the-title-element-2
    KRYS_NODISCARD static HTMLTitleElement *GetTitleElement(const Document &document) noexcept
    {
      auto descendants = dom::DescendantRange(const_cast<Document &>(document));
      auto title = dom::FirstOfType<HTMLTitleElement>(descendants);
      return title == std::ranges::end(descendants) ? nullptr : &Downcast<HTMLTitleElement>(*title);
    }

    /// @brief The first of the html element's children that is a body or frameset element.
    /// @see https://html.spec.whatwg.org/multipage/dom.html#the-body-element-2
    KRYS_NODISCARD static HTMLElement *GetBodyElement(const Document &document) noexcept
    {
      auto *html = GetHtmlElement(document);
      if (html == nullptr)
      {
        return nullptr;
      }

      for (auto &child : dom::ChildNodeRange(*html))
      {
        auto *element = DynamicDowncast<HTMLElement>(child);
        if (element && (Is<HTMLBodyElement>(*element) || Is<HTMLFrameSetElement>(*element)))
        {
          return element;
        }
      }
      return nullptr;
    }
  };
}

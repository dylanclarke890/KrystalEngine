#pragma once

#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLFrameSetElement.hpp"
#include "Krystal.Booey/HTML/HTMLHeadElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/HTMLTitleElement.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#dom-tree-accessors
  class DOMTreeAccessors
  {
  public:
    /// @brief The document element, if it's an html element.
    /// @see https://html.spec.whatwg.org/#the-html-element-2
    KRYS_NODISCARD static HTMLHtmlElement *GetHtmlElement(const dom::Document &document) noexcept
    {
      auto documentElement = document.DocumentElement();
      if (!Is<HTMLHtmlElement>(documentElement))
      {
        return nullptr;
      }

      return const_cast<HTMLHtmlElement *>(&Downcast<HTMLHtmlElement>(*documentElement));
    }

    /// @brief The first head element that is a child of the html element.
    /// @see https://html.spec.whatwg.org/#the-head-element-2
    KRYS_NODISCARD static HTMLHeadElement *GetHeadElement(const dom::Document &document) noexcept
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
    /// @see https://html.spec.whatwg.org/#the-title-element-2
    KRYS_NODISCARD static HTMLTitleElement *GetTitleElement(const dom::Document &document) noexcept
    {
      auto descendants = dom::DescendantRange(const_cast<dom::Document &>(document));
      auto title = dom::FirstOfType<HTMLTitleElement>(descendants);
      return title == std::ranges::end(descendants) ? nullptr : &Downcast<HTMLTitleElement>(*title);
    }

    /// @brief The first of the html element's children that is a body or frameset element.
    /// @see https://html.spec.whatwg.org/#the-body-element-2
    KRYS_NODISCARD static HTMLElement *GetBodyElement(const dom::Document &document) noexcept
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

#pragma once

#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/HTML/HTMLBodyElement.hpp"
#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/HTML/HTMLHeadElement.hpp"
#include "Krystal.HTML/HTML/HTMLHtmlElement.hpp"
#include "Krystal.HTML/HTML/HTMLTitleElement.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#dom-tree-accessors
  class DOMTreeAccessors
  {
  public:
    /// @see https://html.spec.whatwg.org/#the-html-element-2
    KRYS_NODISCARD static RefPtr<HTML::HTMLHtmlElement> HTMLHtmlElement(Document &document) noexcept
    {
      auto documentElement = document.DocumentElement();

      if (!Is<HTMLElement>(documentElement))
      {
        return nullptr;
      }

      auto *htmlElement = Downcast<HTMLElement>(documentElement.get());
      if (!Is<HTML::HTMLHtmlElement>(htmlElement))
      {
        return nullptr;
      }

      auto *html = Downcast<HTML::HTMLHtmlElement>(htmlElement);
      return ShareRefPtr(html);
    }

    /// @see https://html.spec.whatwg.org/#the-html-element-2
    KRYS_NODISCARD static RefPtr<const HTML::HTMLHtmlElement>
      HTMLHtmlElement(const Document &document) noexcept
    {
      return HTMLHtmlElement(const_cast<Document &>(document));
    }

    /// @see https://html.spec.whatwg.org/#the-head-element-2
    KRYS_NODISCARD static RefPtr<HTML::HTMLHeadElement> HTMLHeadElement(Document &document) noexcept
    {
      auto HTML = HTMLHtmlElement(document);
      if (HTML == nullptr)
      {
        return nullptr;
      }

      auto children = ChildHTMLElementRange(*HTML);
      auto body = FirstOfType<HTML::HTMLHeadElement>(children);
      return body == std::ranges::end(children) ? nullptr
                                                : ShareRefPtr(Downcast<HTML::HTMLHeadElement>(&*body));
    }

    /// @see https://html.spec.whatwg.org/#the-head-element-2
    KRYS_NODISCARD static RefPtr<const HTML::HTMLHeadElement>
      HTMLHeadElement(const Document &document) noexcept
    {
      return HTMLHeadElement(const_cast<Document &>(document));
    }

    /// @see https://html.spec.whatwg.org/#the-title-element-2
    KRYS_NODISCARD static RefPtr<HTML::HTMLTitleElement> HTMLTitleElement(Document &document) noexcept
    {
      auto head = HTMLHeadElement(document);
      if (head == nullptr)
      {
        return nullptr;
      }

      for (auto &child : ChildElementRange(*head))
      {
        if (!Is<HTMLElement>(child))
        {
          continue;
        }

        auto &htmlElement = Downcast<HTMLElement>(child);
        if (!Is<HTML::HTMLTitleElement>(htmlElement))
        {
          continue;
        }

        return ShareRefPtr(&Downcast<HTML::HTMLTitleElement>(htmlElement));
      }

      return nullptr;
    }

    /// @see https://html.spec.whatwg.org/#the-title-element-2
    KRYS_NODISCARD static RefPtr<const HTML::HTMLTitleElement>
      HTMLTitleElement(const Document &document) noexcept
    {
      return HTMLTitleElement(const_cast<Document &>(document));
    }

    /// @see https://html.spec.whatwg.org/#the-body-element-2
    KRYS_NODISCARD static RefPtr<HTML::HTMLBodyElement> HTMLBodyElement(Document &document) noexcept
    {
      auto html = HTMLHtmlElement(document);
      if (html == nullptr)
      {
        return nullptr;
      }

      auto children = ChildHTMLElementRange(*html);
      auto body = FirstOfType<HTML::HTMLBodyElement>(children);
      return body == std::ranges::end(children) ? nullptr
                                                : ShareRefPtr(Downcast<HTML::HTMLBodyElement>(&*body));
    }

    /// @see https://html.spec.whatwg.org/#the-body-element-2
    KRYS_NODISCARD static RefPtr<const HTML::HTMLBodyElement>
      HTMLBodyElement(const Document &document) noexcept
    {
      return HTMLBodyElement(const_cast<Document &>(document));
    }
  };
}
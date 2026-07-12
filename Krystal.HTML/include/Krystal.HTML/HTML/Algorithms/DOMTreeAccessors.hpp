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
    KRYS_NODISCARD static RefPtr<HTMLHtmlElement> GetHtmlElement(Document &document) noexcept
    {
      auto documentElement = document.DocumentElement();
      if (!Is<HTMLHtmlElement>(documentElement))
      {
        return nullptr;
      }

      return ShareRefPtr(&Downcast<HTMLHtmlElement>(*documentElement));
    }

    /// @see https://html.spec.whatwg.org/#the-html-element-2
    KRYS_NODISCARD static RefPtr<const HTMLHtmlElement> GetHtmlElement(const Document &document) noexcept
    {
      auto documentElement = document.DocumentElement();
      if (!Is<HTMLHtmlElement>(documentElement))
      {
        return nullptr;
      }

      return ShareRefPtr(&Downcast<HTMLHtmlElement>(*documentElement));
    }

    /// @see https://html.spec.whatwg.org/#the-head-element-2
    KRYS_NODISCARD static RefPtr<HTMLHeadElement> GetHeadElement(Document &document) noexcept
    {
      auto html = GetHtmlElement(document);
      if (html == nullptr)
      {
        return nullptr;
      }

      auto children = ChildHTMLElementRange(*html);
      auto body = FirstOfType<HTMLHeadElement>(children);
      return body == std::ranges::end(children) ? nullptr : ShareRefPtr(Downcast<HTMLHeadElement>(&*body));
    }

    /// @see https://html.spec.whatwg.org/#the-head-element-2
    KRYS_NODISCARD static RefPtr<const HTMLHeadElement> GetHeadElement(const Document &document) noexcept
    {
      auto html = GetHtmlElement(document);
      if (html == nullptr)
      {
        return nullptr;
      }

      auto children = ConstChildNodeRange(*html);
      auto body = FirstOfType<HTMLHeadElement>(children);
      return body == std::ranges::end(children) ? nullptr : ShareRefPtr(Downcast<HTMLHeadElement>(&*body));
    }

    /// @see https://html.spec.whatwg.org/#the-title-element-2
    KRYS_NODISCARD static RefPtr<HTMLTitleElement> GetTitleElement(Document &document) noexcept
    {
      auto head = GetHeadElement(document);
      if (head == nullptr)
      {
        return nullptr;
      }

      auto children = ChildHTMLElementRange(*head);
      auto body = FirstOfType<HTMLTitleElement>(children);
      return body == std::ranges::end(children) ? nullptr : ShareRefPtr(Downcast<HTMLTitleElement>(&*body));
    }

    /// @see https://html.spec.whatwg.org/#the-title-element-2
    KRYS_NODISCARD static RefPtr<const HTMLTitleElement> GetTitleElement(const Document &document) noexcept
    {
      auto head = GetHeadElement(document);
      if (head == nullptr)
      {
        return nullptr;
      }

      auto children = ConstChildNodeRange(*head);
      auto body = FirstOfType<HTMLTitleElement>(children);
      return body == std::ranges::end(children) ? nullptr : ShareRefPtr(Downcast<HTMLTitleElement>(&*body));
    }

    /// @see https://html.spec.whatwg.org/#the-body-element-2
    KRYS_NODISCARD static RefPtr<HTMLBodyElement> GetBodyElement(Document &document) noexcept
    {
      auto html = GetHtmlElement(document);
      if (html == nullptr)
      {
        return nullptr;
      }

      auto children = ChildNodeRange(*html);
      auto body = FirstOfType<HTMLBodyElement>(children);
      return body == std::ranges::end(children) ? nullptr : ShareRefPtr(Downcast<HTMLBodyElement>(&*body));
    }

    /// @see https://html.spec.whatwg.org/#the-body-element-2
    KRYS_NODISCARD static RefPtr<const HTMLBodyElement> GetBodyElement(const Document &document) noexcept
    {
      auto html = GetHtmlElement(document);
      if (html == nullptr)
      {
        return nullptr;
      }

      auto children = ConstChildNodeRange(*html);
      auto body = FirstOfType<HTMLBodyElement>(children);
      return body == std::ranges::end(children) ? nullptr : ShareRefPtr(Downcast<HTMLBodyElement>(&*body));
    }
  };
}
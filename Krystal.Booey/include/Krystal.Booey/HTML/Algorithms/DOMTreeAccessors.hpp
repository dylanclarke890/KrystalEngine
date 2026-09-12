#pragma once

#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
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
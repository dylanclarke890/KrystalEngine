#pragma once

#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/HTML/Dicts/ShowPopoverOptions.hpp"
#include "Krystal.HTML/HTML/Dicts/TogglePopoverOptions.hpp"
#include "Krystal.HTML/HTML/HTMLTagName.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class Document;

  class HTMLElement : public Element
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLElement);

  protected:
    HTMLTagName _tagName {HTMLTagName::Unknown};

    HTMLElement(Document &document, HTMLTagName tag, NodeFlags flags = NodeFlags::None) noexcept;

  public:
    HTMLElement(Document &document) noexcept;

    // TODO(impl) POPOVER - HTMLElement methods
    // The popover API
    // void ShowPopover(const ShowPopoverOptions &options = {}) noexcept;
    // void HidePopover() noexcept;
    // bool TogglePopover(const TogglePopoverOptionsOrBool &options = {}) noexcept;
    // attribute DOMString? popover;

  protected:
#pragma region Type Checks

    KRYS_NODISCARD bool IsHTMLBaseElement() const noexcept
    {
      return _tagName == HTMLTagName::Base;
    }

    KRYS_NODISCARD bool IsHTMLBodyElement() const noexcept
    {
      return _tagName == HTMLTagName::Body;
    }

    KRYS_NODISCARD bool IsHTMLHeadElement() const noexcept
    {
      return _tagName == HTMLTagName::Head;
    }

    KRYS_NODISCARD bool IsHTMLHtmlElement() const noexcept
    {
      return _tagName == HTMLTagName::Html;
    }

    KRYS_NODISCARD bool IsHTMLLinkElement() const noexcept
    {
      return _tagName == HTMLTagName::Link;
    }

    KRYS_NODISCARD bool IsHTMLMetaElement() const noexcept
    {
      return _tagName == HTMLTagName::Meta;
    }

    KRYS_NODISCARD bool IsHTMLPreElement() const noexcept
    {
      return _tagName == HTMLTagName::Pre;
    }

    // NOTE: HTMLSlotElement type check is not needed here as Node already has it.

    KRYS_NODISCARD bool IsHTMLStyleElement() const noexcept
    {
      return _tagName == HTMLTagName::Style;
    }

    KRYS_NODISCARD bool IsHTMLScriptElement() const noexcept
    {
      return _tagName == HTMLTagName::Script;
    }

    KRYS_NODISCARD bool IsHTMLTitleElement() const noexcept
    {
      return _tagName == HTMLTagName::Title;
    }

    // NOTE: HTMLUnknownElement type check is not needed here as Node already has it.

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return target.IsHTMLElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
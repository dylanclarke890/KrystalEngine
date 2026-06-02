#pragma once

#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/HTML/Dicts/ShowPopoverOptions.hpp"
#include "Krystal.HTML/HTML/Dicts/TogglePopoverOptions.hpp"
#include "Krystal.HTML/HTML/HTMLTagName.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class Document;
  class ElementInternals;

  /// @see https://html.spec.whatwg.org/#htmlelement
  class HTMLElement : public Element
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLElement);

  protected:
    HTMLTagName _tagName {HTMLTagName::Unknown};

    HTMLElement(Document &document, HTMLTagName tag, NodeFlags flags = NodeFlags::None) noexcept;

  public:
    HTMLElement(Document &document) noexcept;

#pragma region HTMLElement - https://html.spec.whatwg.org/#htmlelement

    /// @see https://html.spec.whatwg.org/#dom-title
    KRYS_NODISCARD DOMString Title() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-title
    void Title(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-lang
    KRYS_NODISCARD DOMString Lang() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-lang
    void Lang(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-translate
    KRYS_NODISCARD bool Translate() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-translate
    void Translate(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dir
    KRYS_NODISCARD DOMString Dir() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dir
    void Dir(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hidden
    KRYS_NODISCARD BoolOr<DOMString> Hidden() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hidden
    void Hidden(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hidden
    void Hidden(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hidden
    void Hidden(double value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-inert
    KRYS_NODISCARD bool Inert() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-inert
    void Inert(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-click
    void Click() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-accesskey
    KRYS_NODISCARD DOMString AccessKey() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-accesskey
    void AccessKey(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-accesskeylabel
    KRYS_NODISCARD DOMString AccessKeyLabel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-draggable
    KRYS_NODISCARD bool Draggable() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-draggable
    void Draggable(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-spellcheck
    KRYS_NODISCARD bool SpellCheck() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-spellcheck
    void SpellCheck(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-writingsuggestions
    KRYS_NODISCARD DOMString WritingSuggestions() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-writingsuggestions
    void WritingSuggestions(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-autocapitalize
    KRYS_NODISCARD DOMString Autocapitalize() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-autocapitalize
    void Autocapitalize(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-autocorrect
    KRYS_NODISCARD bool Autocorrect() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-autocorrect
    void Autocorrect(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-innertext
    KRYS_NODISCARD DOMString InnerText() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-innertext
    void InnerText(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-outertext
    KRYS_NODISCARD DOMString OuterText() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-outertext
    void OuterText(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-attachinternals
    KRYS_NODISCARD ExceptionOr<Ref<ElementInternals>> AttachInternals() noexcept;

    // TODO(HTMLELEMENT, POPOVER, HTML): Implement the popover API.
    // The popover API - https://html.spec.whatwg.org/#dom-showpopover
    // undefined showPopover(optional ShowPopoverOptions options = {});
    // undefined hidePopover();
    // boolean togglePopover(optional (TogglePopoverOptions or boolean) options = {});
    // [CEReactions] attribute DOMString? popover;

    /// @see https://html.spec.whatwg.org/#dom-headingoffset
    KRYS_NODISCARD uint32 HeadingOffset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-headingoffset
    void HeadingOffset(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-headingreset
    KRYS_NODISCARD bool HeadingReset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-headingreset
    void HeadingReset(bool value) noexcept;

#pragma endregion

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
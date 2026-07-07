#pragma once

#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/HTML/Dicts/ShowPopoverOptions.hpp"
#include "Krystal.HTML/HTML/Dicts/TogglePopoverOptions.hpp"
#include "Krystal.HTML/HTML/Enums/HTMLElementInterface.hpp"
#include "Krystal.HTML/HTML/Enums/HTMLElementFlags.hpp"
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

  private:
    HTMLElementFlags _flags : BitCount<HTMLElementFlags>() {HTMLElementFlags::None};
    HTMLElementInterface _interface : BitCount<HTMLElementInterface>() {HTMLElementInterface::None};

  protected:
    HTMLElement(Document &document, HTMLElementInterface interface, HTMLElementFlags flags = HTMLElementFlags::None,
                NodeFlags nodeFlags = NodeFlags::None) noexcept;

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

    // TODO(HTMLELEMENT, HTML): HTMLElement mixins:
    // HTMLElement includes GlobalEventHandlers;
    // HTMLElement includes ElementContentEditable;
    // HTMLElement includes HTMLOrSVGOrMathMLElement;

  protected:
#pragma region HTMLElement Flags

    void SetHTMLElement(HTMLElementFlags flag) noexcept
    {
      _flags = _flags | flag;
    }

    void ClearHTMLElement(HTMLElementFlags flag) noexcept
    {
      _flags = _flags & ~flag;
    }

    KRYS_NODISCARD bool HasHTMLElement(HTMLElementFlags flag) const noexcept
    {
      return HasFlag(_flags, flag);
    }

#pragma endregion

#pragma region Type Checks

    KRYS_NODISCARD bool IsHTMLAreaElement() const noexcept
    {
      return _interface == HTMLElementInterface::Area;
    }

    KRYS_NODISCARD bool IsHTMLAudioElement() const noexcept
    {
      return _interface == HTMLElementInterface::Audio;
    }

    KRYS_NODISCARD bool IsHTMLAnchorElement() const noexcept
    {
      return _interface == HTMLElementInterface::Anchor;
    }

    KRYS_NODISCARD bool IsHTMLBaseElement() const noexcept
    {
      return _interface == HTMLElementInterface::Base;
    }

    KRYS_NODISCARD bool IsHTMLBodyElement() const noexcept
    {
      return _interface == HTMLElementInterface::Body;
    }

    KRYS_NODISCARD bool IsHTMLBRElement() const noexcept
    {
      return _interface == HTMLElementInterface::BR;
    }

    KRYS_NODISCARD bool IsHTMLButtonElement() const noexcept
    {
      return _interface == HTMLElementInterface::Button;
    }

    KRYS_NODISCARD bool IsHTMLCanvasElement() const noexcept
    {
      return _interface == HTMLElementInterface::Canvas;
    }

    KRYS_NODISCARD bool IsHTMLDataElement() const noexcept
    {
      return _interface == HTMLElementInterface::Data;
    }

    KRYS_NODISCARD bool IsHTMLDataListElement() const noexcept
    {
      return _interface == HTMLElementInterface::DataList;
    }

    KRYS_NODISCARD bool IsHTMLDetailsElement() const noexcept
    {
      return _interface == HTMLElementInterface::Details;
    }

    KRYS_NODISCARD bool IsHTMLDialogElement() const noexcept
    {
      return _interface == HTMLElementInterface::Dialog;
    }

    KRYS_NODISCARD bool IsHTMLDivElement() const noexcept
    {
      return _interface == HTMLElementInterface::Div;
    }

    KRYS_NODISCARD bool IsHTMLDListElement() const noexcept
    {
      return _interface == HTMLElementInterface::DList;
    }

    KRYS_NODISCARD bool IsHTMLEmbedElement() const noexcept
    {
      return _interface == HTMLElementInterface::Embed;
    }

    KRYS_NODISCARD bool IsHTMLFieldSetElement() const noexcept
    {
      return _interface == HTMLElementInterface::FieldSet;
    }

    KRYS_NODISCARD bool IsHTMLFormElement() const noexcept
    {
      return _interface == HTMLElementInterface::Form;
    }

    KRYS_NODISCARD bool IsHTMLHeadElement() const noexcept
    {
      return _interface == HTMLElementInterface::Head;
    }

    KRYS_NODISCARD bool IsHTMLHeadingElement() const noexcept
    {
      return _interface == HTMLElementInterface::Heading;
    }

    KRYS_NODISCARD bool IsHTMLHRElement() const noexcept
    {
      return _interface == HTMLElementInterface::HR;
    }

    KRYS_NODISCARD bool IsHTMLHtmlElement() const noexcept
    {
      return _interface == HTMLElementInterface::Html;
    }

    KRYS_NODISCARD bool IsHTMLIFrameElement() const noexcept
    {
      return _interface == HTMLElementInterface::IFrame;
    }

    KRYS_NODISCARD bool IsHTMLImageElement() const noexcept
    {
      return _interface == HTMLElementInterface::Image;
    }

    KRYS_NODISCARD bool IsHTMLInputElement() const noexcept
    {
      return _interface == HTMLElementInterface::Input;
    }

    KRYS_NODISCARD bool IsHTMLLabelElement() const noexcept
    {
      return _interface == HTMLElementInterface::Label;
    }

    KRYS_NODISCARD bool IsHTMLLegendElement() const noexcept
    {
      return _interface == HTMLElementInterface::Legend;
    }

    KRYS_NODISCARD bool IsHTMLLIElement() const noexcept
    {
      return _interface == HTMLElementInterface::LI;
    }

    KRYS_NODISCARD bool IsHTMLLinkElement() const noexcept
    {
      return _interface == HTMLElementInterface::Link;
    }

    KRYS_NODISCARD bool IsHTMLMapElement() const noexcept
    {
      return _interface == HTMLElementInterface::Map;
    }

    KRYS_NODISCARD bool IsHTMLMediaElement() const noexcept
    {
      return HasFlag(_flags, HTMLElementFlags::IsMediaElement);
    }

    KRYS_NODISCARD bool IsHTMLMenuElement() const noexcept
    {
      return _interface == HTMLElementInterface::Menu;
    }

    KRYS_NODISCARD bool IsHTMLMetaElement() const noexcept
    {
      return _interface == HTMLElementInterface::Meta;
    }

    KRYS_NODISCARD bool IsHTMLMeterElement() const noexcept
    {
      return _interface == HTMLElementInterface::Meter;
    }

    KRYS_NODISCARD bool IsHTMLModElement() const noexcept
    {
      return _interface == HTMLElementInterface::Mod;
    }

    KRYS_NODISCARD bool IsHTMLObjectElement() const noexcept
    {
      return _interface == HTMLElementInterface::Object;
    }

    KRYS_NODISCARD bool IsHTMLOListElement() const noexcept
    {
      return _interface == HTMLElementInterface::OList;
    }

    KRYS_NODISCARD bool IsHTMLOptGroupElement() const noexcept
    {
      return _interface == HTMLElementInterface::OptGroup;
    }

    KRYS_NODISCARD bool IsHTMLOptionElement() const noexcept
    {
      return _interface == HTMLElementInterface::Option;
    }

    KRYS_NODISCARD bool IsHTMLOutputElement() const noexcept
    {
      return _interface == HTMLElementInterface::Output;
    }

    KRYS_NODISCARD bool IsHTMLParagraphElement() const noexcept
    {
      return _interface == HTMLElementInterface::Paragraph;
    }

    KRYS_NODISCARD bool IsHTMLPictureElement() const noexcept
    {
      return _interface == HTMLElementInterface::Pre;
    }

    KRYS_NODISCARD bool IsHTMLPreElement() const noexcept
    {
      return _interface == HTMLElementInterface::Pre;
    }

    KRYS_NODISCARD bool IsHTMLProgressElement() const noexcept
    {
      return _interface == HTMLElementInterface::Progress;
    }

    KRYS_NODISCARD bool IsHTMLQuoteElement() const noexcept
    {
      return _interface == HTMLElementInterface::Quote;
    }

    KRYS_NODISCARD bool IsHTMLScriptElement() const noexcept
    {
      return _interface == HTMLElementInterface::Script;
    }

    KRYS_NODISCARD bool IsHTMLSelectElement() const noexcept
    {
      return _interface == HTMLElementInterface::Select;
    }

    KRYS_NODISCARD bool IsHTMLSelectedContentElement() const noexcept
    {
      return _interface == HTMLElementInterface::SelectedContent;
    }

    // NOTE: HTMLSlotElement type check is not needed here as Node already has it.

    KRYS_NODISCARD bool IsHTMLSourceElement() const noexcept
    {
      return _interface == HTMLElementInterface::Source;
    }

    KRYS_NODISCARD bool IsHTMLSpanElement() const noexcept
    {
      return _interface == HTMLElementInterface::Span;
    }

    KRYS_NODISCARD bool IsHTMLStyleElement() const noexcept
    {
      return _interface == HTMLElementInterface::Style;
    }

    KRYS_NODISCARD bool IsHTMLTableElement() const noexcept
    {
      return _interface == HTMLElementInterface::Table;
    }

    KRYS_NODISCARD bool IsHTMLTableCaptionElement() const noexcept
    {
      return _interface == HTMLElementInterface::TableCaption;
    }

    KRYS_NODISCARD bool IsHTMLTableCellElement() const noexcept
    {
      return _interface == HTMLElementInterface::TableCell;
    }

    KRYS_NODISCARD bool IsHTMLTableColElement() const noexcept
    {
      return _interface == HTMLElementInterface::TableCol;
    }

    KRYS_NODISCARD bool IsHTMLTableRowElement() const noexcept
    {
      return _interface == HTMLElementInterface::TableRow;
    }

    KRYS_NODISCARD bool IsHTMLTableSectionElement() const noexcept
    {
      return _interface == HTMLElementInterface::TableSection;
    }

    KRYS_NODISCARD bool IsHTMLTemplateElement() const noexcept
    {
      return _interface == HTMLElementInterface::Template;
    }

    KRYS_NODISCARD bool IsHTMLTextAreaElement() const noexcept
    {
      return _interface == HTMLElementInterface::TextArea;
    }

    KRYS_NODISCARD bool IsHTMLTitleElement() const noexcept
    {
      return _interface == HTMLElementInterface::Title;
    }

    KRYS_NODISCARD bool IsHTMLTimeElement() const noexcept
    {
      return _interface == HTMLElementInterface::Time;
    }

    KRYS_NODISCARD bool IsHTMLTrackElement() const noexcept
    {
      return _interface == HTMLElementInterface::Track;
    }

    KRYS_NODISCARD bool IsHTMLUListElement() const noexcept
    {
      return _interface == HTMLElementInterface::UList;
    }

    // NOTE: HTMLUnknownElement type check is not needed here as Node already has it.

    KRYS_NODISCARD bool IsHTMLVideoElement() const noexcept
    {
      return _interface == HTMLElementInterface::Video;
    }

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return target.IsHTMLElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
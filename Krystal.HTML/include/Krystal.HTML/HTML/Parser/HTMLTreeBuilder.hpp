#pragma once

#include "Krystal.HTML/Constants/ElementNames.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/DocumentFragment.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/HTML/Enums/InsertionMode.hpp"
#include "Krystal.HTML/HTML/HTMLFormElement.hpp"
#include "Krystal.HTML/HTML/HTMLHeadElement.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLElementStack.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLFormattingElementList.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLStackItem.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenAtom.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  struct FragmentParsingContext
  {
  private:
    RefPtr<DocumentFragment> _fragment;
    RefPtr<Element> _contextElement;

  public:
    FragmentParsingContext() noexcept = default;

    FragmentParsingContext(DocumentFragment &fragment, Element &contextElement) noexcept
        : _fragment(ShareRefPtr(&fragment)), _contextElement(ShareRefPtr(&contextElement))
    {
    }

    KRYS_NODISCARD DocumentFragment &Fragment() const noexcept
    {
      assert(_fragment);
      return *_fragment;
    }

    KRYS_NODISCARD Element &ContextElement() const noexcept
    {
      assert(_contextElement);
      return *_contextElement;
    }
  };

  struct AdjustedInsertionLocation
  {
    /// @brief The parent into which the new node should be inserted.
    RawPtr<ContainerNode> Parent;

    /// @brief The child before which the new node should be inserted, or null if the new node should be
    /// appended to `Parent`.
    RawPtr<Node> BeforeSibling {nullptr};
  };

  class HTMLTreeBuilder
  {
  private:
    Document &_document;

    /// @see https://html.spec.whatwg.org/multipage/syntax.html#insertion-mode
    InsertionMode _insertionMode : BitCount<InsertionMode>() {InsertionMode::Initial};

    /// @see https://html.spec.whatwg.org/multipage/syntax.html#original-insertion-mode
    InsertionMode _originalInsertionMode : BitCount<InsertionMode>() {InsertionMode::Initial};

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#frameset-ok-flag
    bool _framesetOk : 1 {true};

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#foster-parent
    bool _fosterParenting : 1 {false};

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#stack-of-template-insertion-modes
    List<InsertionMode> _templateInsertionModes;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#stack-of-open-elements
    HTMLElementStack _openElementStack;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#the-list-of-active-formatting-elements
    HTMLFormattingElementList _activeFormattingElements;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#head-element-pointer
    RefPtr<HTMLHeadElement> _head;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#form-element-pointer
    RefPtr<HTMLFormElement> _form;

    Maybe<FragmentParsingContext> _fragmentParsingContext;

  public:
    HTMLTreeBuilder(Document &document) noexcept;

    HTMLTreeBuilder(DocumentFragment &fragment, Element &contextElement) noexcept;

    void ProcessToken(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#current-template-insertion-mode
    KRYS_NODISCARD InsertionMode CurrentTemplateInsertionMode() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#reset-the-insertion-mode-appropriately
    void ResetInsertionModeAppropriately() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#current-node
    KRYS_NODISCARD ContainerNode &CurrentNode() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#adjusted-current-node
    KRYS_NODISCARD ContainerNode &AdjustedCurrentNode() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#special
    KRYS_NODISCARD static bool IsSpecialElement(ElementName name) noexcept
    {
      // TODO(HTMLTREEBUILDER, HTML): These are also special elements:
      // MathML mi, MathML mo, MathML mn, MathML ms, MathML mtext, and MathML annotation-xml; and SVG
      // foreignObject, SVG desc, and SVG title.

      switch (name)
      {
        case ElementName::address:
        case ElementName::applet:
        case ElementName::area:
        case ElementName::article:
        case ElementName::aside:
        case ElementName::base:
        case ElementName::basefont:
        case ElementName::bgsound:
        case ElementName::blockquote:
        case ElementName::body:
        case ElementName::br:
        case ElementName::button:
        case ElementName::caption:
        case ElementName::center:
        case ElementName::col:
        case ElementName::colgroup:
        case ElementName::dd:
        case ElementName::details:
        case ElementName::dir:
        case ElementName::div:
        case ElementName::dl:
        case ElementName::dt:
        case ElementName::embed:
        case ElementName::fieldset:
        case ElementName::figcaption:
        case ElementName::figure:
        case ElementName::footer:
        case ElementName::form:
        case ElementName::frame:
        case ElementName::frameset:
        case ElementName::h1:
        case ElementName::h2:
        case ElementName::h3:
        case ElementName::h4:
        case ElementName::h5:
        case ElementName::h6:
        case ElementName::head:
        case ElementName::header:
        case ElementName::hgroup:
        case ElementName::hr:
        case ElementName::html:
        case ElementName::iframe:
        case ElementName::img:
        case ElementName::input:
        case ElementName::keygen:
        case ElementName::li:
        case ElementName::link:
        case ElementName::listing:
        case ElementName::main:
        case ElementName::marquee:
        case ElementName::menu:
        case ElementName::meta:
        case ElementName::nav:
        case ElementName::noembed:
        case ElementName::noframes:
        case ElementName::noscript:
        case ElementName::object:
        case ElementName::ol:
        case ElementName::p:
        case ElementName::param:
        case ElementName::plaintext:
        case ElementName::pre:
        case ElementName::script:
        case ElementName::search:
        case ElementName::section:
        case ElementName::select:
        case ElementName::source:
        case ElementName::style:
        case ElementName::summary:
        case ElementName::table:
        case ElementName::tbody:
        case ElementName::td:
        case ElementName::template_:
        case ElementName::textarea:
        case ElementName::tfoot:
        case ElementName::th:
        case ElementName::thead:
        case ElementName::title:
        case ElementName::tr:
        case ElementName::track:
        case ElementName::ul:
        case ElementName::wbr:
        case ElementName::xmp:
        {
          return true;
        }
        default:
        {
          return false;
        }
      }
    }

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#formatting
    KRYS_NODISCARD static bool IsFormattingElement(ElementName name) noexcept
    {
      switch (name)
      {
        case ElementName::a:
        case ElementName::b:
        case ElementName::big:
        case ElementName::code:
        case ElementName::em:
        case ElementName::font:
        case ElementName::i:
        case ElementName::nobr:
        case ElementName::s:
        case ElementName::small:
        case ElementName::strike:
        case ElementName::strong:
        case ElementName::tt:
        case ElementName::u:
        {
          return true;
        }
        default:
        {
          return false;
        }
      }
    }

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#has-an-element-in-scope
    KRYS_NODISCARD bool HasElementInScope(ElementName targetNode) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#has-an-element-in-list-item-scope
    KRYS_NODISCARD bool HasElementInListItemScope(ElementName targetNode) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#has-an-element-in-button-scope
    KRYS_NODISCARD bool HasElementInButtonScope(ElementName targetNode) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#has-an-element-in-table-scope
    KRYS_NODISCARD bool HasElementInTableScope(ElementName targetNode) const noexcept;

    KRYS_NODISCARD AdjustedInsertionLocation
      AppropriateInsertionLocation(RawPtr<ContainerNode> targetOverride = nullptr) noexcept;
  };
}
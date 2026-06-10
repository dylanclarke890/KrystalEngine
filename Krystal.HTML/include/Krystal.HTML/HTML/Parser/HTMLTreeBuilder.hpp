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
    KRYS_NODISCARD static bool IsSpecialElement(TagName name) noexcept
    {
      // TODO(HTMLTREEBUILDER, HTML): These are also special elements:
      // MathML mi, MathML mo, MathML mn, MathML ms, MathML mtext, and MathML annotation-xml; and SVG
      // foreignObject, SVG desc, and SVG title.

      switch (name)
      {
        case TagName::address:
        case TagName::applet:
        case TagName::area:
        case TagName::article:
        case TagName::aside:
        case TagName::base:
        case TagName::basefont:
        case TagName::bgsound:
        case TagName::blockquote:
        case TagName::body:
        case TagName::br:
        case TagName::button:
        case TagName::caption:
        case TagName::center:
        case TagName::col:
        case TagName::colgroup:
        case TagName::dd:
        case TagName::details:
        case TagName::dir:
        case TagName::div:
        case TagName::dl:
        case TagName::dt:
        case TagName::embed:
        case TagName::fieldset:
        case TagName::figcaption:
        case TagName::figure:
        case TagName::footer:
        case TagName::form:
        case TagName::frame:
        case TagName::frameset:
        case TagName::h1:
        case TagName::h2:
        case TagName::h3:
        case TagName::h4:
        case TagName::h5:
        case TagName::h6:
        case TagName::head:
        case TagName::header:
        case TagName::hgroup:
        case TagName::hr:
        case TagName::html:
        case TagName::iframe:
        case TagName::img:
        case TagName::input:
        case TagName::keygen:
        case TagName::li:
        case TagName::link:
        case TagName::listing:
        case TagName::main:
        case TagName::marquee:
        case TagName::menu:
        case TagName::meta:
        case TagName::nav:
        case TagName::noembed:
        case TagName::noframes:
        case TagName::noscript:
        case TagName::object:
        case TagName::ol:
        case TagName::p:
        case TagName::param:
        case TagName::plaintext:
        case TagName::pre:
        case TagName::script:
        case TagName::search:
        case TagName::section:
        case TagName::select:
        case TagName::source:
        case TagName::style:
        case TagName::summary:
        case TagName::table:
        case TagName::tbody:
        case TagName::td:
        case TagName::template_:
        case TagName::textarea:
        case TagName::tfoot:
        case TagName::th:
        case TagName::thead:
        case TagName::title:
        case TagName::tr:
        case TagName::track:
        case TagName::ul:
        case TagName::wbr:
        case TagName::xmp:
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
    KRYS_NODISCARD static bool IsFormattingElement(TagName name) noexcept
    {
      switch (name)
      {
        case TagName::a:
        case TagName::b:
        case TagName::big:
        case TagName::code:
        case TagName::em:
        case TagName::font:
        case TagName::i:
        case TagName::nobr:
        case TagName::s:
        case TagName::small:
        case TagName::strike:
        case TagName::strong:
        case TagName::tt:
        case TagName::u:
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
    KRYS_NODISCARD bool HasElementInScope(TagName targetNode) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#has-an-element-in-list-item-scope
    KRYS_NODISCARD bool HasElementInListItemScope(TagName targetNode) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#has-an-element-in-button-scope
    KRYS_NODISCARD bool HasElementInButtonScope(TagName targetNode) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#has-an-element-in-table-scope
    KRYS_NODISCARD bool HasElementInTableScope(TagName targetNode) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#appropriate-place-for-inserting-a-node
    KRYS_NODISCARD AdjustedInsertionLocation
      AppropriateInsertionLocation(RawPtr<ContainerNode> targetOverride = nullptr) noexcept;
  };
}
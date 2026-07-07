#pragma once

#include "Krystal.HTML/Constants/TagNames.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/DocumentFragment.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/HTML/Enums/InsertionMode.hpp"
#include "Krystal.HTML/HTML/Enums/ParserScriptingMode.hpp"
#include "Krystal.HTML/HTML/HTMLFormElement.hpp"
#include "Krystal.HTML/HTML/HTMLHeadElement.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLElementStack.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLFormattingElementList.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLStackItem.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenAtom.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenizer.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
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

    HTMLTokenizer &_tokenizer;

    /// @see https://html.spec.whatwg.org/multipage/syntax.html#insertion-mode
    InsertionMode _insertionMode : BitCount<InsertionMode>() {InsertionMode::Initial};

    /// @see https://html.spec.whatwg.org/multipage/syntax.html#original-insertion-mode
    InsertionMode _originalInsertionMode : BitCount<InsertionMode>() {InsertionMode::Initial};

    /// @see https://html.spec.whatwg.org/#scripting-mode
    ParserScriptingMode _scriptingMode : BitCount<ParserScriptingMode>() {ParserScriptingMode::Normal};

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

    RawPtr<Element> _contextElement;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#pending-table-character-tokens
    List<DOMString> _pendingTableCharacterTokens;

  public:
    HTMLTreeBuilder(Document &document, HTMLTokenizer &tokenizer,
                    RawPtr<Element> contextElement = nullptr) noexcept;

    void ProcessToken(HTMLTokenAtom &&token) noexcept;

    void SetScriptingMode(ParserScriptingMode mode) noexcept
    {
      _scriptingMode = mode;
    }

  private:
    bool ShouldProcessAccordingToRulesForHTMLContent(const HTMLTokenAtom &token) noexcept;

    void ProcessAccordingToRulesForHTMLContent(HTMLTokenAtom &&token) noexcept;

    void ProcessAccordingToRulesForForeignContent(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#current-node
    KRYS_NODISCARD ContainerNode &CurrentNode() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#adjusted-current-node
    KRYS_NODISCARD ContainerNode &AdjustedCurrentNode() noexcept;

    void ParseError(const HTMLTokenAtom &token) noexcept;

#pragma region InsertionMode Algorithms

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#current-template-insertion-mode
    KRYS_NODISCARD InsertionMode CurrentTemplateInsertionMode() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#reset-the-insertion-mode-appropriately
    void ResetInsertionModeAppropriately() noexcept;

    /// @see https://html.spec.whatwg.org/#the-initial-insertion-mode
    void InitialMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-before-html-insertion-mode
    void BeforeHTMLMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-before-head-insertion-mode
    void BeforeHeadMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#parsing-main-inhead
    void InHeadMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-head-noscript-insertion-mode
    void InHeadNoscriptMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-after-head-insertion-mode
    void AfterHeadMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-body-insertion-mode
    void InBodyMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-text-insertion-mode
    void TextMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-table-insertion-mode
    void InTableMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-table-text-insertion-mode
    void InTableTextMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-caption-insertion-mode
    void InCaptionMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-column-group-insertion-mode
    void InColumnGroupMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-table-body-insertion-mode
    void InTableBodyMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-row-insertion-mode
    void InRowMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-cell-insertion-mode
    void InCellMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-template-insertion-mode
    void InTemplateMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-after-body-insertion-mode
    void AfterBodyMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-frameset-insertion-mode
    void InFramesetMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-after-frameset-insertion-mode
    void AfterFramesetMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-after-after-body-insertion-mode
    void AfterAfterBodyMode(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-after-after-frameset-insertion-mode
    void AfterAfterFramesetMode(HTMLTokenAtom &&token) noexcept;

    void InBodyGenericEndTag(const HTMLTokenAtom &token, TagName tagName);

    void StopParsing() noexcept;

#pragma endregion

#pragma region Insertion Algorithms

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#appropriate-place-for-inserting-a-node
    KRYS_NODISCARD AdjustedInsertionLocation
      AppropriateInsertionLocation(RawPtr<HTMLStackItem> targetOverride = nullptr) noexcept;

    /// @see https://html.spec.whatwg.org/#insert-an-element-at-the-adjusted-insertion-location
    void InsertElementAtAdjustedInsertionLocation(Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/#insert-a-foreign-element
    Ref<Element> InsertForeignElement(HTMLTokenAtom &&token, DOMStringAtom namespaceURI,
                                      bool onlyAddToElementStack) noexcept;

    /// @see https://html.spec.whatwg.org/#insert-an-html-element
    Ref<Element> InsertHTMLElement(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#insert-a-character
    void InsertCharacter(DOMString &&data) noexcept;

    /// @see https://html.spec.whatwg.org/#insert-a-character
    void AppendCommentToDocument(DOMString &&data) noexcept;

    /// @brief Skips whitespace in a character token and returns whether any non-whitespace characters remain.
    bool SkipCharacterTokenWhitespace(HTMLTokenAtom &token) noexcept;

    /// @brief Inserts leading whitespace in a character token and returns whether any non-whitespace
    /// characters remain.
    bool InsertCharacterTokenWhitespace(HTMLTokenAtom &token) noexcept;

    /// @see https://html.spec.whatwg.org/#insert-a-comment
    void InsertComment(DOMString &&data, Maybe<AdjustedInsertionLocation> position = Null) noexcept;

    /// @see https://html.spec.whatwg.org/#generic-raw-text-element-parsing-algorithm
    void ParseGenericRawTextElement(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#generic-rcdata-text-element-parsing-algorithm
    void ParseGenericRCDATATextElement(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#create-an-element-for-the-token
    KRYS_NODISCARD Ref<Element> CreateElement(DOMStringAtom name, DOMStringAtom namespaceURI,
                                              const ParsedAttributeList &attributes,
                                              ContainerNode &intendedParent) noexcept;

    KRYS_NODISCARD Ref<Element> CreateElement(const HTMLStackItem &item,
                                              RawPtr<ContainerNode> intendedParent = nullptr) noexcept;

    void ReconstructActiveFormattingElements() noexcept;

    /// @see https://html.spec.whatwg.org/#close-a-p-element
    void ClosePElement(const HTMLTokenAtom &token) noexcept;

#pragma endregion

#pragma region Adjust Attribute Algorithms

    /// @see https://html.spec.whatwg.org/#adjust-foreign-attributes
    void AdjustForeignAttributes(HTMLTokenAtom &token) noexcept;

    /// @see https://html.spec.whatwg.org/#adjust-svg-attributes
    void AdjustSVGAttributes(HTMLTokenAtom &token) noexcept;

    /// @see https://html.spec.whatwg.org/#adjust-math-ml-attributes
    void AdjustMathMLAttributes(HTMLTokenAtom &token) noexcept;

#pragma endregion

#pragma region Quirks Mode Algorithms

    bool IsQuirksModeDOCTYPE(const HTMLTokenAtom &token) const noexcept;

    bool IsLimitedQuirksModeDOCTYPE(const HTMLTokenAtom &token) const noexcept;

#pragma endregion

#pragma region Element Category Algorithms

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
      }

      return false;
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

#pragma endregion

#pragma region Adoption Agency Algorithms

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#adoption-agency-algorithm
    void RunAdoptionAgency(HTMLTokenAtom &token) noexcept;

    RawPtr<HTMLStackItem> FurthestSpecialElementBlock(const Element &formattingElement) noexcept;

#pragma endregion

#pragma region IntegrationPoint Algorithms

    KRYS_NODISCARD bool IsMathMLTextIntegrationPoint(const Element &element) const noexcept;

    KRYS_NODISCARD bool IsHTMLIntegrationPoint(const Element &element) const noexcept;

#pragma endregion
  };
}
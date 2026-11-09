/*
 * Copyright (C) 2010 Google, Inc. All rights reserved.
 * Copyright (C) 2011-2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY GOOGLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL GOOGLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHeadElement.hpp"
#include "Krystal.Booey/HTML/Parser/FormattingElementList.hpp"
#include "Krystal.Booey/HTML/Parser/OpenElementStack.hpp"
#include "Krystal.Booey/HTML/Parser/StackItem.hpp"
#include "Krystal.Booey/HTML/Parser/TokenAtom.hpp"
#include "Krystal.Booey/HTML/Parser/Tokenizer.hpp"
#include "Krystal.Booey/HTML/Sanitizer/SanitizerAlgorithms.hpp"
#include "Krystal.Booey/HTML/Sanitizer/SanitizerConfig.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::html
{
  enum class InsertionMode : uint8
  {
    Initial,
    BeforeHTML,
    BeforeHead,
    InHead,
    InHeadNoscript,
    AfterHead,
    InBody,
    Text,
    InTable,
    InTableText,
    InCaption,
    InColumnGroup,
    InTableBody,
    InRow,
    InCell,
    InTemplate,
    AfterBody,
    InFrameset,
    AfterFrameset,
    AfterAfterBody,
    AfterAfterFrameset
  };

  /// @see https://html.spec.whatwg.org/multipage/parsing.html#parser-scripting-mode
  enum class ParserScriptingMode : uint8
  {
    Normal,
    Disabled,
    Inert,
    Fragment
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::InsertionMode, 21u)
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::ParserScriptingMode, 4u)

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/multipage/parsing.html#special
  KRYS_NODISCARD constexpr bool IsSpecialElement(TagName tagName, Namespace tagNamespace) noexcept
  {
    switch (tagNamespace)
    {
      case Namespace::HTML:
      {
        switch (tagName)
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
      case Namespace::MathML:
      {
        switch (tagName)
        {
          case TagName::mi:
          case TagName::mo:
          case TagName::mn:
          case TagName::ms:
          case TagName::mtext:
          case TagName::annotation_xml:
          {
            return true;
          }
        }

        return false;
      }
      case Namespace::SVG:
      {
        switch (tagName)
        {
          case TagName::foreignObject:
          case TagName::desc:
          case TagName::title:
          {
            return true;
          }
        }

        return false;
      }
    }

    return false;
  }

  /// @see https://html.spec.whatwg.org/multipage/parsing.html#formatting
  KRYS_NODISCARD constexpr bool IsFormattingElement(TagName tagName, Namespace tagNamespace) noexcept
  {
    if (tagNamespace != Namespace::HTML)
    {
      return false;
    }

    switch (tagName)
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
    }

    return false;
  }

  struct InsertionLocation
  {
    /// @brief The parent into which the new node should be inserted.
    dom::ContainerNode *Parent;

    /// @brief The child before which the new node should be inserted, or null if the new node should be
    /// appended to `Parent`.
    dom::Node *BeforeSibling {nullptr};
  };

  class TreeBuilder
  {
  private:
    dom::Document &_document;

    Tokenizer &_tokenizer;

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
    OpenElementStack _openElementStack;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#the-list-of-active-formatting-elements
    FormattingElementList _activeFormattingElements;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#head-element-pointer
    RefPtr<HTMLHeadElement> _head;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#form-element-pointer
    RefPtr<HTMLFormElement> _form;

    Maybe<StackItem> &_context;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#pending-table-character-tokens
    List<dom::DOMString> _pendingTableCharacterTokens;
    /// @brief Redirects insertions into the fragment case's root `html` element to the fragment, and
    /// insertions into elements replaced with their children by the sanitizer to where those elements would
    /// have been.
    /// @see https://html.spec.whatwg.org/multipage/parsing.html#insertion-target-redirection-map
    HashMap<const dom::Node *, InsertionLocation> _insertionTargetRedirectionMap;
    /// @brief Keeps the redirection map's keys alive.
    List<Ref<dom::Element>> _redirectedElements;
    /// @brief When there is one, the parser sanitizes the markup as it's parsed: elements, attributes and
    /// comments that it doesn't allow aren't inserted.
    /// @see https://html.spec.whatwg.org/multipage/parsing.html#parser-sanitizer-configuration
    Maybe<SanitizerConfig> _sanitizerConfig;

    bool _ignoreNextNewline : 1 {false};

  public:
    TreeBuilder(dom::Document &document, Tokenizer &tokenizer, Maybe<StackItem> &context) noexcept;

    void ProcessToken(TokenAtom &&token) noexcept;

    void ScriptingMode(ParserScriptingMode mode) noexcept
    {
      _scriptingMode = mode;
    }

    ParserScriptingMode ScriptingMode() const noexcept
    {
      return _scriptingMode;
    }

    /// @brief Sets the parser's insertion target redirection map[@p element] to @p location.
    void RedirectInsertions(dom::Element &element, InsertionLocation location) noexcept;

    /// @brief Sets the parser sanitizer configuration; null for none.
    void SanitizerConfiguration(Maybe<SanitizerConfig> &&configuration) noexcept
    {
      _sanitizerConfig = krys::move(configuration);
    }

    void FormElement(HTMLFormElement &form) noexcept
    {
      _form = ShareRefPtr(&form);
    }

    bool IsCDATASectionAllowedInCurrentContext() noexcept
    {
      if (_openElementStack.IsEmpty())
      {
        return false;
      }

      auto &adjustedCurrentNode = AdjustedCurrentNode();
      return adjustedCurrentNode.NamespaceURI() != infra::Namespaces::HTML;
    }

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#reset-the-insertion-mode-appropriately
    void ResetInsertionModeAppropriately() noexcept;

    KRYS_NODISCARD OpenElementStack &GetOpenElementStack() noexcept
    {
      return _openElementStack;
    }

    void PushTemplateInsertionMode(InsertionMode mode) noexcept;

  private:
    bool ShouldProcessAccordingToRulesForHTMLContent(const TokenAtom &token) noexcept;

    void ProcessAccordingToRulesForHTMLContent(TokenAtom &&token) noexcept;

    void ProcessAccordingToRulesForForeignContent(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#current-node
    KRYS_NODISCARD dom::Element &CurrentNode() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#adjusted-current-node
    KRYS_NODISCARD dom::Element &AdjustedCurrentNode() noexcept;

    void ParseError(const TokenAtom &token) noexcept;

#pragma region InsertionMode Algorithms

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#current-template-insertion-mode
    KRYS_NODISCARD InsertionMode CurrentTemplateInsertionMode() const noexcept;

    /// @see https://html.spec.whatwg.org/#the-initial-insertion-mode
    void InitialMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-before-html-insertion-mode
    void BeforeHTMLMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-before-head-insertion-mode
    void BeforeHeadMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#parsing-main-inhead
    void InHeadMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-head-noscript-insertion-mode
    void InHeadNoscriptMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-after-head-insertion-mode
    void AfterHeadMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-body-insertion-mode
    void InBodyMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-text-insertion-mode
    void TextMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-table-insertion-mode
    void InTableMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-table-text-insertion-mode
    void InTableTextMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-caption-insertion-mode
    void InCaptionMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-column-group-insertion-mode
    void InColumnGroupMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-table-body-insertion-mode
    void InTableBodyMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-row-insertion-mode
    void InRowMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-cell-insertion-mode
    void InCellMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-template-insertion-mode
    void InTemplateMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-after-body-insertion-mode
    void AfterBodyMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-in-frameset-insertion-mode
    void InFramesetMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-after-frameset-insertion-mode
    void AfterFramesetMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-after-after-body-insertion-mode
    void AfterAfterBodyMode(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#the-after-after-frameset-insertion-mode
    void AfterAfterFramesetMode(TokenAtom &&token) noexcept;

    void InBodyGenericEndTag(const TokenAtom &token, TagName tagName);

    void StopParsing() noexcept;

#pragma endregion

#pragma region Character Insertion

    /// @see https://html.spec.whatwg.org/#insert-a-character
    void InsertCharacters(dom::DOMString data) noexcept;

    /// @brief Removes null characters in a character token and returns whether any characters remain.
    bool RemoveNulls(TokenAtom &token) noexcept;

    /// @brief Removes non whitespace characters in a character token and returns whether any characters
    /// remain.
    bool RemoveNonWhitespace(TokenAtom &token) noexcept;

    /// @brief Skips leading whitespace in a character token and returns whether any characters remain.
    bool SkipLeadingWhitespace(TokenAtom &token) noexcept;

    /// @brief Inserts leading whitespace in a character token and returns whether any characters remain.
    bool InsertLeadingWhitespace(TokenAtom &token) noexcept;

    /// @brief Inserts leading whitespace and null characters (replacing them with U+FFFD beforehand) in a
    /// character token and returns whether any characters remain. Will also replace any other nulls in the
    /// string so that InsertCharacters can be called safely afterwards.
    bool InsertLeadingWhitespaceAndNulls(TokenAtom &token) noexcept;

#pragma endregion

#pragma region Insertion Algorithms

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#appropriate-place-for-inserting-a-node
    KRYS_NODISCARD InsertionLocation
      AppropriatePlaceToInsertNode(dom::ContainerNode *targetOverride = nullptr) noexcept;

    /// @see https://html.spec.whatwg.org/#adjusted-insertion-location
    KRYS_NODISCARD InsertionLocation
      AdjustedInsertionLocation(Maybe<InsertionLocation> insertionLocation = null) noexcept;

    /// @see https://html.spec.whatwg.org/#insert-an-element-at-the-adjusted-insertion-location
    void InsertElementAtAdjustedInsertionLocation(dom::Element &element,
                                                  Maybe<InsertionLocation> location = null,
                                                  bool alreadySanitized = false) noexcept;

    /// @see https://html.spec.whatwg.org/#insert-a-foreign-element
    Ref<dom::Element> InsertForeignElement(TokenAtom &&token, dom::DOMStringAtom namespaceURI,
                                           bool onlyAddToElementStack) noexcept;

    /// @brief The sanitizer action for an element named @p localName in @p namespaceURI: "Keep" when the
    /// parser isn't sanitizing.
    /// @see https://html.spec.whatwg.org/#get-the-sanitizer-action-for-an-element-name
    KRYS_NODISCARD SanitizerAction SanitizerActionFor(dom::DOMStringView localName,
                                                      dom::DOMStringAtom namespaceURI) const noexcept;

    /// @brief Adds the attributes of an `html` or `body` start tag that @p element doesn't have and the
    /// parser's sanitizer configuration allows.
    void MergeAttributes(dom::Element &element, const ParsedAttributeList &attributes) noexcept;

    /// @brief The "in head" insertion mode's steps for a template start tag, which may attach a declarative
    /// shadow root.
    /// @see https://html.spec.whatwg.org/multipage/parsing.html#parsing-main-inhead
    void InsertTemplateElement(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#insert-an-html-element
    Ref<dom::Element> InsertHTMLElement(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#insert-a-comment
    void InsertComment(dom::DOMStringView data, Maybe<InsertionLocation> location = null) noexcept;

    /// @see https://html.spec.whatwg.org/#insert-a-processing-instruction
    void InsertProcessingInstruction(const TokenAtom &token,
                                     Maybe<InsertionLocation> location = null) noexcept;

    /// @see https://html.spec.whatwg.org/#insert-a-character
    void AppendCommentToDocument(dom::DOMStringView data) noexcept;

    /// @see https://html.spec.whatwg.org/#generic-raw-text-element-parsing-algorithm
    void ParseGenericRawTextElement(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#generic-rcdata-text-element-parsing-algorithm
    void ParseGenericRCDATATextElement(TokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/#create-an-element-for-the-token
    KRYS_NODISCARD Ref<dom::Element> CreateElement(dom::DOMStringAtom name, dom::DOMStringAtom namespaceURI,
                                                   const ParsedAttributeList &attributes,
                                                   dom::ContainerNode &intendedParent) noexcept;

    KRYS_NODISCARD Ref<dom::Element> CreateElement(const StackItem &item,
                                                   dom::ContainerNode *intendedParent = nullptr) noexcept;

    void ReconstructActiveFormattingElements() noexcept;

    /// @see https://html.spec.whatwg.org/#close-a-p-element
    void ClosePElement(const TokenAtom &token) noexcept;

#pragma endregion

#pragma region Adjust Attribute Algorithms

    /// @see https://html.spec.whatwg.org/#adjust-foreign-attributes
    void AdjustForeignAttributes(TokenAtom &token) noexcept;

    /// @see https://html.spec.whatwg.org/#adjust-svg-attributes
    void AdjustSVGAttributes(TokenAtom &token) noexcept;

    /// @see https://html.spec.whatwg.org/#adjust-math-ml-attributes
    void AdjustMathMLAttributes(TokenAtom &token) noexcept;

#pragma endregion

#pragma region Quirks Mode Algorithms

    bool IsQuirksModeDOCTYPE(const TokenAtom &token) const noexcept;

    bool IsLimitedQuirksModeDOCTYPE(const TokenAtom &token) const noexcept;

#pragma endregion

#pragma region Adoption Agency Algorithms

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#adoption-agency-algorithm
    void RunAdoptionAgency(TokenAtom &token) noexcept;

    StackItem *FurthestSpecialElementBlock(const dom::Element &formattingElement) noexcept;

#pragma endregion

#pragma region IntegrationPoint Algorithms

    KRYS_NODISCARD bool IsMathMLTextIntegrationPoint(const dom::Element &element) const noexcept;

    KRYS_NODISCARD bool IsHTMLIntegrationPoint(const dom::Element &element) const noexcept;

#pragma endregion
  };
}
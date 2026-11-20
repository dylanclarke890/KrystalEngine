/*
 * Copyright (C) 2004-2024 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Parsing/ParserContentPolicy.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Editing/Markup/HTMLInterchange.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class ContainerNode;
  class Document;
  class DocumentFragment;
  class Element;
  class Node;
  class ShadowRoot;
}

namespace krys::boo::dom
{
  struct SimpleRange;
  enum class TreeType : uint8;
}

namespace krys::boo
{
  class Page;
  class VisibleSelection;

  enum class ResolveUrls : uint8
  {
    No,
    NoExcludingUrlsForPrivacy,
    Yes,
    YesExcludingUrlsForPrivacy,
  };

  /// @brief Whether nodes, and everything under them, are `user-select: none`, remembered as nodes are asked
  /// about.
  class UserSelectNoneStateCache
  {
  private:
    enum class State : uint8
    {
      NotUserSelectNone,
      Mixed,
      OnlyUserSelectNone,
    };

    HashMap<const Node *, State> _cache;
    bool _useComposedTree;

  public:
    explicit UserSelectNoneStateCache(dom::TreeType treeType) noexcept;

    KRYS_NODISCARD bool NodeOnlyContainsUserSelectNone(Node &node) noexcept
    {
      return ComputeState(node) == State::OnlyUserSelectNone;
    }

  private:
    KRYS_NODISCARD ContainerNode *ParentNode(Node &node) const noexcept;

    KRYS_NODISCARD Node *FirstChild(Node &node) const noexcept;

    KRYS_NODISCARD Node *NextSibling(Node &node) const noexcept;

    KRYS_NODISCARD State ComputeState(Node &targetNode) noexcept;
  };

  /// @brief The text, as a fragment of `p`s (or clones of the context's block), `br`s, text nodes and tab
  /// spans, that it would be pasted as at @p context.
  KRYS_NODISCARD Ref<DocumentFragment> CreateFragmentFromText(const dom::SimpleRange &context,
                                                              utf8_stringview text) noexcept;

  /// @brief The markup, parsed as if it were in a `body`.
  KRYS_NODISCARD Ref<DocumentFragment> CreateFragmentFromMarkup(Document &document, utf8_stringview markup,
                                                                utf8_stringview baseUrl) noexcept;

  /// @brief The markup, parsed as the children of @p contextElement, for `innerHTML`, `outerHTML`,
  /// `insertAdjacentHTML()` and `setHTMLUnsafe()`: as HTML in an HTML document or when the policy says to,
  /// and as XML otherwise, which throws a `SyntaxError` if it isn't well formed.
  /// @see https://html.spec.whatwg.org/#fragment-parsing-algorithm-steps
  KRYS_NODISCARD ExceptionOr<Ref<DocumentFragment>>
    CreateFragmentForInnerOuterHTML(Element &contextElement, utf8_stringview markup,
                                    EnumSet<dom::ParserContentPolicy> parserContentPolicy = {}) noexcept;

  /// @see https://w3c.github.io/DOM-Parsing/#dom-range-createcontextualfragment
  KRYS_NODISCARD ExceptionOr<Ref<DocumentFragment>> CreateContextualFragment(Element &element,
                                                                             utf8_stringview markup) noexcept;

  KRYS_NODISCARD bool IsPlainTextMarkup(Node *node) noexcept;

  // These methods are used by HTMLElement & ShadowRoot to replace the children with respected fragment/text.
  ExceptionOr<void> ReplaceChildrenWithFragment(ContainerNode &container,
                                                const Ref<DocumentFragment> &fragment) noexcept;

  enum class ConvertBlocksToInlines : bool
  {
    No,
    Yes,
  };

  enum class SerializeComposedTree : bool
  {
    No,
    Yes,
  };

  enum class IgnoreUserSelectNone : bool
  {
    No,
    Yes,
  };

  enum class PreserveBaseElement : bool
  {
    No,
    Yes,
  };

  enum class PreserveDirectionForInlineText : bool
  {
    No,
    Yes,
  };

  /// @brief The range's markup, with the styles that make it look as it does inlined, so it looks the same
  /// wherever it's pasted.
  KRYS_NODISCARD utf8_string SerializePreservingVisualAppearance(
    const dom::SimpleRange &range, List<Ref<Node>> *nodes = nullptr,
    AnnotateForInterchange annotate = AnnotateForInterchange::No,
    ConvertBlocksToInlines convertBlocksToInlines = ConvertBlocksToInlines::No,
    ResolveUrls resolveUrls = ResolveUrls::No) noexcept;

  KRYS_NODISCARD utf8_string SerializePreservingVisualAppearance(
    const VisibleSelection &selection, ResolveUrls resolveUrls = ResolveUrls::No,
    SerializeComposedTree serializeComposedTree = SerializeComposedTree::No,
    IgnoreUserSelectNone ignoreUserSelectNone = IgnoreUserSelectNone::Yes,
    PreserveBaseElement preserveBaseElement = PreserveBaseElement::No,
    PreserveDirectionForInlineText preserveDirectionForInlineText = PreserveDirectionForInlineText::No,
    List<Ref<Node>> *nodes = nullptr) noexcept;

  enum class SerializedNodes : uint8
  {
    SubtreeIncludingNode,
    SubtreesOfChildren,
  };

  enum class SerializationSyntax : uint8
  {
    HTML,
    XML,
    HTMLLegacyAttributeValue,
  };

  enum class SerializeShadowRoots : uint8
  {
    Explicit,
    Serializable,
    AllForInterchange,
  };

  enum class RequireWellFormed : bool
  {
    No,
    Yes,
  };

  /// @see https://html.spec.whatwg.org/#serialising-html-fragments
  KRYS_NODISCARD utf8_string SerializeFragment(
    const Node &node, SerializedNodes root, List<Ref<Node>> *nodes = nullptr,
    ResolveUrls resolveUrls = ResolveUrls::No, Maybe<SerializationSyntax> serializationSyntax = null,
    SerializeShadowRoots serializeShadowRoots = SerializeShadowRoots::Explicit,
    List<Ref<ShadowRoot>> &&explicitShadowRoots = {}) noexcept;

  /// @brief Serializes @p node as the markup getters (`innerHTML`, `outerHTML`) do: as HTML in an HTML
  /// document, and otherwise as XML that must be well-formed.
  /// @return An `InvalidStateError` if the XML serialization meets one of its error cases.
  /// @see https://html.spec.whatwg.org/#fragment-serializing-algorithm-steps
  KRYS_NODISCARD ExceptionOr<utf8_string> SerializeFragmentRequiringWellFormed(const Node &node,
                                                                               SerializedNodes root) noexcept;

  KRYS_NODISCARD utf8_string UrlToMarkup(utf8_stringview url, utf8_stringview title) noexcept;

  /// @brief A page with empty clients, whose main frame shows an empty document, to parse markup in apart
  /// from @p destinationDocument.
  KRYS_NODISCARD Ref<Page> CreatePageForSanitizingWebContent(Document *destinationDocument) noexcept;

  enum class MSOListQuirks : bool
  {
    CheckIfNeeded,
    Disabled,
  };

  /// @brief @p rawHTML parsed in a staging page and serialised again with its styles inlined, which leaves
  /// out what the parser and serialiser don't keep (scripts, comments, unknown markup).
  /// @param fragmentSanitizer Changes the parsed fragment before it's serialised, if given.
  /// @return The markup, or none if the staging page has no document.
  KRYS_NODISCARD Maybe<utf8_string>
    SanitizeMarkup(utf8_stringview rawHTML, Document *destinationDocument,
                   MSOListQuirks msoListQuirks = MSOListQuirks::Disabled,
                   const Func<void(DocumentFragment &)> &fragmentSanitizer = {}) noexcept;

  KRYS_NODISCARD utf8_string SanitizedMarkupForFragmentInDocument(const Ref<DocumentFragment> &fragment,
                                                                  Document &document,
                                                                  MSOListQuirks msoListQuirks,
                                                                  utf8_stringview originalMarkup) noexcept;

  KRYS_NODISCARD utf8_string DocumentTypeString(const Document &document) noexcept;
}

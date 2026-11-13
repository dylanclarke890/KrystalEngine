/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 * Copyright (C) 2015-2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetList.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Editing/Markup/Markup.hpp"
#include "Krystal.Booey/HTML/Algorithms/HTMLInsertionAlgorithms.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::dom
{
  ShadowRoot::ShadowRoot(Document &document, RefPtr<html::CustomElementRegistry> &&registry,
                         NodeFlags flags) noexcept
      : DocumentFragment(document, flags | NodeFlags::IsShadowRoot),
        DocumentOrShadowRoot(*this, krys::move(registry))
  {
    SetNodeFlag(NodeFlags::IsInShadowTree);
    _containingShadowRoot = this;
    _styleScope = CreateUnique<style::Scope>(*this);
  }

  ShadowRoot::~ShadowRoot() noexcept
  {
    if (_styleSheetList)
    {
      _styleSheetList->Detach();
    }

    // Children are detached without the removing steps, and something else may hold one.
    for (auto &node : DescendantRange(*this))
    {
      node._containingShadowRoot = nullptr;
    }
  }

#pragma region DocumentOrShadowRoot Mixin (CSSOM)

  css::StyleSheetList &ShadowRoot::StyleSheets() noexcept
  {
    if (!_styleSheetList)
    {
      _styleSheetList = css::StyleSheetList::Create(*this);
    }
    return *_styleSheetList;
  }

  List<RefPtr<WebAnimation>> ShadowRoot::GetAnimations() noexcept
  {
    return NodeDocument().MatchingAnimations([&](Element &target) -> bool
                                             { return TreeQueries::ContainingShadowRoot(target) == this; });
  }

#pragma endregion

  void ShadowRoot::SetReferenceTarget(Maybe<DOMString> referenceTarget) noexcept
  {
    // The setter steps are to set this's reference target to the given value.
    _referenceTarget = krys::move(referenceTarget);
  }

  Element *ShadowRoot::Host() const noexcept
  {
    if (auto host = _host.lock())
    {
      return host.get();
    }

    return nullptr;
  }

#pragma region ShadowRoot (DOM Parsing)

  ExceptionOr<void> ShadowRoot::SetHTML(DOMStringView html, html::SetHTMLOptions &&options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // Filter and set HTML given this, html, options, and Safe.
    return html::HTMLInsertionAlgorithms::FilterAndSetHTML(*this, html, &options.Sanitizer,
                                                           html::FragmentParserMode::Safe);
  }

  ExceptionOr<void> ShadowRoot::AppendHTML(DOMStringView html, html::SetHTMLOptions &&options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // Filter and pre-insert HTML given this, null, html, options, and Safe.
    return html::HTMLInsertionAlgorithms::FilterAndPreInsertHTML(*this, nullptr, html, &options.Sanitizer,
                                                                 html::FragmentParserMode::Safe);
  }

  ExceptionOr<void> ShadowRoot::PrependHTML(DOMStringView html, html::SetHTMLOptions &&options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // Filter and pre-insert HTML given this, this's first child, html, options, and Safe.
    return html::HTMLInsertionAlgorithms::FilterAndPreInsertHTML(
      *this, FirstChild(), html, &options.Sanitizer, html::FragmentParserMode::Safe);
  }

  ExceptionOr<void> ShadowRoot::SetHTMLUnsafe(DOMStringView html,
                                              html::SetHTMLUnsafeOptions &&options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input
    //    algorithm with this's relevant global object, html, options, and "ShadowRoot setHTMLUnsafe".
    // (There are no Trusted Types.)
    // 2. Filter and set HTML given this, html, options, and Unsafe.
    return html::HTMLInsertionAlgorithms::FilterAndSetHTML(
      *this, html, options.Sanitizer ? &*options.Sanitizer : nullptr, html::FragmentParserMode::Unsafe);
  }

  ExceptionOr<void> ShadowRoot::AppendHTMLUnsafe(DOMStringView html,
                                                 html::SetHTMLUnsafeOptions &&options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input
    //    algorithm with this's relevant global object, html, options, and "ShadowRoot appendHTMLUnsafe".
    // (There are no Trusted Types.)
    // 2. Filter and pre-insert HTML given this, null, html, options, and Unsafe.
    return html::HTMLInsertionAlgorithms::FilterAndPreInsertHTML(
      *this, nullptr, html, options.Sanitizer ? &*options.Sanitizer : nullptr,
      html::FragmentParserMode::Unsafe);
  }

  ExceptionOr<void> ShadowRoot::PrependHTMLUnsafe(DOMStringView html,
                                                  html::SetHTMLUnsafeOptions &&options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input
    //    algorithm with this's relevant global object, html, options, and "ShadowRoot prependHTMLUnsafe".
    // (There are no Trusted Types.)
    // 2. Filter and pre-insert HTML given this, this's first child, html, options, and Unsafe.
    return html::HTMLInsertionAlgorithms::FilterAndPreInsertHTML(
      *this, FirstChild(), html, options.Sanitizer ? &*options.Sanitizer : nullptr,
      html::FragmentParserMode::Unsafe);
  }

  DOMString ShadowRoot::GetHTML(GetHTMLOptions &&options) const noexcept
  {
    return SerializeFragment(*this, SerializedNodes::SubtreesOfChildren, nullptr,
                             ResolveURLs::NoExcludingURLsForPrivacy, SerializationSyntax::HTML,
                             options.SerializableShadowRoots ? SerializeShadowRoots::Serializable
                                                             : SerializeShadowRoots::Explicit,
                             krys::move(options.ShadowRoots));
  }

  ExceptionOr<DOMString> ShadowRoot::InnerHTML() const noexcept
  {
    // The innerHTML getter steps are to return the result of running fragment serializing algorithm steps
    // with this and true.
    return SerializeFragmentRequiringWellFormed(*this, SerializedNodes::SubtreesOfChildren);
  }

  ExceptionOr<void> ShadowRoot::InnerHTML(DOMStringView markup) noexcept
  {
    html::CEReactionsScope ceReactions;
    return ReplaceChildrenWithMarkup(markup, {});
  }

#pragma endregion

  ExceptionOr<void>
    ShadowRoot::ReplaceChildrenWithMarkup(DOMStringView markup,
                                          EnumSet<ParserContentPolicy> parserContentPolicy) noexcept
  {
    if (markup.empty())
    {
      return MutationAlgorithms::ReplaceAll(nullptr, *this);
    }

    RefPtr<Element> host = ShareRefPtr(Host());
    krys_assert(host);
    auto fragment = CreateFragmentForInnerOuterHTML(*host, markup, parserContentPolicy);
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }
    return ReplaceChildrenWithFragment(*this, fragment.ReleaseValue());
  }

#pragma region Extension Hooks

  void ShadowRoot::OnInsert() noexcept
  {
    DocumentFragment::OnInsert();
    if (IsConnected())
    {
      NodeDocument().DidInsertInDocumentShadowRoot(*this);
    }
    if (!GetAdoptedStyleSheets().empty())
    {
      StyleScope().DidChangeActiveStyleSheetCandidates();
    }
  }

  void ShadowRoot::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    DocumentFragment::OnRemove(isSubtreeRoot, oldAncestor);
    NodeDocument().DidRemoveInDocumentShadowRoot(*this);
  }

  void ShadowRoot::OnAdopt(Document &oldDocument) noexcept
  {
    DocumentFragment::OnAdopt(oldDocument);

    // Adopted sheets were constructed for the old document.
    (void)SetAdoptedStyleSheets({});

    // Style scopes are document specific.
    _styleScope = CreateUnique<style::Scope>(*this);
  }

#pragma endregion

  EventTarget *ShadowRoot::GetParent(Event &event) const noexcept
  {
    if (!event.Composed())
    {
      if (event._path.size() >= 1)
      {
        auto &firstItem = event._path[0];
        if (auto *invocationTargetNode = DynamicDowncast<Node>(firstItem.InvocationTarget()))
        {
          if (&TreeQueries::Root(*invocationTargetNode) == this)
          {
            return nullptr;
          }
        }
      }
    }

    if (auto host = _host.lock())
    {
      return host.get();
    }

    return nullptr;
  }

  namespace
  {
    /// @brief One mapping of an `exportparts` list: `name` or `name : exported`.
    KRYS_NODISCARD Maybe<Pair<DOMStringAtom, DOMStringAtom>> ParsePartMapping(DOMStringView mapping) noexcept
    {
      auto skipWhitespace = [&](size_t position)
      {
        while (position < mapping.size() && krys::text::IsASCIIWhitespace(mapping[position]))
        {
          ++position;
        }
        return position;
      };
      auto collectValue = [&](size_t position)
      {
        while (position < mapping.size() && !krys::text::IsASCIIWhitespace(mapping[position])
               && mapping[position] != u8':')
        {
          ++position;
        }
        return position;
      };

      auto begin = skipWhitespace(0uz);
      auto firstPartEnd = collectValue(begin);
      if (firstPartEnd == begin)
      {
        return null;
      }
      auto firstPart = DOMStringAtom(mapping.substr(begin, firstPartEnd - begin));

      begin = skipWhitespace(firstPartEnd);
      if (begin == mapping.size())
      {
        return Pair {firstPart, firstPart};
      }
      if (mapping[begin] != u8':')
      {
        return null;
      }

      begin = skipWhitespace(begin + 1uz);
      auto secondPartEnd = collectValue(begin);
      if (secondPartEnd == begin)
      {
        return null;
      }
      auto secondPart = DOMStringAtom(mapping.substr(begin, secondPartEnd - begin));

      if (skipWhitespace(secondPartEnd) != mapping.size())
      {
        return null;
      }
      return Pair {firstPart, secondPart};
    }
  }

  const ShadowRoot::PartMappings &ShadowRoot::GetPartMappings() const noexcept
  {
    if (!_partMappings)
    {
      PartMappings mappings;
      if (auto exportParts = Host()->GetAttribute(u8"exportparts"))
      {
        DOMStringView remaining = *exportParts;
        while (true)
        {
          auto comma = remaining.find(u8',');
          if (auto mapping = ParsePartMapping(remaining.substr(0uz, comma)))
          {
            mappings[mapping->first].push_back(mapping->second);
          }
          if (comma == DOMStringView::npos)
          {
            break;
          }
          remaining.remove_prefix(comma + 1uz);
        }
      }
      _partMappings = krys::move(mappings);
    }
    return *_partMappings;
  }

  List<Ref<ShadowRoot>> AssignedShadowRootsIfSlotted(const Node &node) noexcept
  {
    List<Ref<ShadowRoot>> result;
    for (auto *slot = SlotAlgorithms::GetAssignedSlot(node); slot;
         slot = SlotAlgorithms::GetAssignedSlot(*slot))
    {
      auto *shadowRoot = TreeQueries::ContainingShadowRoot(*slot);
      krys_debug_assert(shadowRoot);
      result.push_back(ShareRef(*shadowRoot));
    }
    return result;
  }
}

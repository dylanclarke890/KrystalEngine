/*
 * Copyright (C) 2011-2018 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::css
{
  class Selector;
}

namespace krys::boo::dom
{
  class ContainerNode;
  class Document;
  class Element;
  class NodeList;

  /// @brief Runs the selectors of a list against elements, choosing a fast path for the common
  /// single-selector shapes.
  class SelectorDataList
  {
    enum class MatchType : uint8
    {
      SingleSelector,
      TagNameMatch,
      ClassNameMatch,
      AttributeExactMatch,
      MultipleSelectorMatch,
    };

    struct SelectorData
    {
      const css::Selector *Selector;
    };

  private:
    List<SelectorData> _selectors;
    /// @brief The ID the single selector's rightmost compound selector requires, if any.
    const css::Selector *_rightmostIdSelector {nullptr};
    MatchType _matchType;

  public:
    explicit SelectorDataList(const css::SelectorList &selectorList) noexcept;

    KRYS_NODISCARD bool Matches(const Element &element) const noexcept;
    KRYS_NODISCARD Element *Closest(Element &element) const noexcept;
    KRYS_NODISCARD Ref<NodeList> QueryAll(ContainerNode &rootNode) const noexcept;
    KRYS_NODISCARD Element *QueryFirst(ContainerNode &rootNode) const noexcept;

  private:
    KRYS_NODISCARD bool SelectorMatches(const SelectorData &selectorData, const Element &element,
                                        const ContainerNode &rootNode) const noexcept;

    template <typename TOutput>
    void Execute(ContainerNode &rootNode, TOutput &output) const noexcept;
    template <typename TOutput>
    void ExecuteSingleTagNameSelectorData(const ContainerNode &rootNode, const SelectorData &selectorData,
                                          TOutput &output) const noexcept;
    template <typename TOutput>
    void ExecuteSingleClassNameSelectorData(const ContainerNode &rootNode, const SelectorData &selectorData,
                                            TOutput &output) const noexcept;
    template <typename TOutput>
    void ExecuteSingleAttributeExactSelectorData(const ContainerNode &rootNode,
                                                 const SelectorData &selectorData,
                                                 TOutput &output) const noexcept;
    /// @brief Finds what the selector matches through the ID in its rightmost compound selector, if there's
    /// one, and at most one element has it.
    /// @return Whether it did.
    template <typename TOutput>
    KRYS_NODISCARD bool ExecuteRightmostIdSelectorData(const ContainerNode &rootNode,
                                                       const SelectorData &selectorData,
                                                       TOutput &output) const noexcept;
    /// @brief The narrowest subtree of @p rootNode that holds what the selector can match, going by an ID in
    /// it that only one element has.
    KRYS_NODISCARD const ContainerNode &
      SearchRootForAncestorId(const ContainerNode &rootNode, const SelectorData &selectorData) const noexcept;
    template <typename TOutput>
    void ExecuteSingleSelectorData(const ContainerNode &rootNode, const ContainerNode &searchRootNode,
                                   const SelectorData &selectorData, TOutput &output) const noexcept;
    template <typename TOutput>
    void ExecuteSingleMultiSelectorData(const ContainerNode &rootNode, TOutput &output) const noexcept;
  };

  /// @brief A parsed selector list for `querySelector()`, `querySelectorAll()`, `matches()` and `closest()`.
  class SelectorQuery
  {
    KRYS_NON_COPY_MOVABLE(SelectorQuery);

  private:
    css::SelectorList _selectorList;
    SelectorDataList _selectors;

  public:
    explicit SelectorQuery(css::SelectorList &&selectorList) noexcept;

    KRYS_NODISCARD bool Matches(const Element &element) const noexcept
    {
      return _selectors.Matches(element);
    }

    KRYS_NODISCARD Element *Closest(Element &element) const noexcept
    {
      return _selectors.Closest(element);
    }

    /// @see https://dom.spec.whatwg.org/#scope-match-a-selectors-string (step 3)
    KRYS_NODISCARD Ref<NodeList> QueryAll(ContainerNode &rootNode) const noexcept
    {
      return _selectors.QueryAll(rootNode);
    }

    /// @brief The first element QueryAll() would return.
    /// @see https://dom.spec.whatwg.org/#scope-match-a-selectors-string (step 3)
    KRYS_NODISCARD Element *QueryFirst(ContainerNode &rootNode) const noexcept
    {
      return _selectors.QueryFirst(rootNode);
    }
  };

  /// @brief The selector queries parsed so far, by selector text and parser context.
  class SelectorQueryCache
  {
    using Key = Pair<DOMString, css::SelectorParserContext>;

    struct KeyHash
    {
      KRYS_NODISCARD size_t operator()(const Key &key) const noexcept;
    };

  private:
    HashMap<Key, UniquePtr<SelectorQuery>, KeyHash> _entries;

  public:
    KRYS_NODISCARD static SelectorQueryCache &Singleton() noexcept;

    /// @return The query for @p selectors in @p document, or null if @p selectors isn't a valid selector
    /// list.
    KRYS_NODISCARD SelectorQuery *Add(DOMStringView selectors, const Document &document) noexcept;

    void Clear() noexcept;
  };
}

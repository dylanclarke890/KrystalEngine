// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016-2020 Apple Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "Krystal.Booey/CSS/Parser/ParserEnum.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Selectors/MutableSelector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/StronglyTypedValue.hpp"

namespace krys::boo::css
{
  struct DisallowPseudoElements : public StronglyTypedBool<DisallowPseudoElements>
  {
    using Base::Base;
  };

  struct IsForgivingSelectorList : public StronglyTypedBool<IsForgivingSelectorList>
  {
    using Base::Base;
  };

  /// @brief Parses selectors into MutableSelectors.
  /// @see https://drafts.csswg.org/selectors/
  class SelectorParser
  {
    const SelectorParserContext _context;
    const RefPtr<StyleSheetContents> _styleSheet;
    NestedContext _nestedContext;

    // FIXME: This _failedParsing is ugly and confusing, we should look into removing it (the return value of
    // each function already convey this information).
    bool _failedParsing {false};

    bool _disallowPseudoElements {false};
    bool _disallowHasPseudoClass {false};
    // Inside an argument that is a compound selector (`:host()`, `:host-context()`, `::slotted()`), where
    // nested selector lists can hold only compound selectors.
    bool _disallowComplexSelectors {false};
    bool _resistDefaultNamespace {false};
    bool _ignoreDefaultNamespace {false};
    bool _disableForgivingParsing {false};
    const MutableSelector *_precedingPseudoElement {nullptr};

  public:
    SelectorParser(const SelectorParserContext &context, StyleSheetContents *styleSheet,
                   NestedContext nestedContext = {},
                   DisallowPseudoElements disallowPseudoElements = DisallowPseudoElements(false)) noexcept;

    KRYS_NODISCARD MutableSelectorList ConsumeComplexSelectorList(TokenRange &tokens) noexcept;
    KRYS_NODISCARD MutableSelectorList ConsumeNestedSelectorList(TokenRange &tokens) noexcept;
    KRYS_NODISCARD MutableSelectorList ConsumeComplexForgivingSelectorList(TokenRange &tokens) noexcept;
    KRYS_NODISCARD MutableSelectorList ConsumeNestedComplexForgivingSelectorList(TokenRange &tokens) noexcept;

    KRYS_NODISCARD static Maybe<SelectorList> ParseSelectorList(CSSOMStringView string,
                                                                const SelectorParserContext &context,
                                                                StyleSheetContents *styleSheet = nullptr,
                                                                NestedContext nestedContext = {}) noexcept;

    /// @brief Whether @p tokens are a complex selector `@supports selector()` accepts.
    KRYS_NODISCARD static bool SupportsComplexSelector(TokenRange tokens,
                                                       const SelectorParserContext &context) noexcept;

    /// @brief Replaces each `&` in @p nestedSelectorList with the parent rule's selectors.
    KRYS_NODISCARD static SelectorList ResolveNestingParent(const SelectorList &nestedSelectorList,
                                                            const SelectorList *parentResolvedSelectorList,
                                                            bool parentRuleIsScope = false) noexcept;

    /// @brief Parses a pseudo-element string such as `::before` or `::highlight(name)`, as passed to
    /// `getComputedStyle()`.
    /// @return Whether the string is a valid pseudo-element, and its identifier if it has its own style.
    KRYS_NODISCARD static Pair<bool, Maybe<style::PseudoElementIdentifier>>
      ParsePseudoElement(CSSOMStringView input, const SelectorParserContext &context) noexcept;

  private:
    template <typename TConsumeSelector>
    KRYS_NODISCARD MutableSelectorList ConsumeSelectorList(TokenRange &tokens,
                                                           TConsumeSelector &&consumeSelector) noexcept;
    template <typename TConsumeSelector>
    KRYS_NODISCARD MutableSelectorList
      ConsumeForgivingSelectorList(TokenRange &tokens, TConsumeSelector &&consumeSelector) noexcept;

    KRYS_NODISCARD MutableSelectorList ConsumeCompoundSelectorList(TokenRange &tokens) noexcept;
    KRYS_NODISCARD MutableSelectorList ConsumeRelativeSelectorList(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeComplexSelector(TokenRange &tokens) noexcept;
    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeNestedComplexSelector(TokenRange &tokens) noexcept;
    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeCompoundSelector(TokenRange &tokens) noexcept;
    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeRelativeScopeSelector(TokenRange &tokens) noexcept;
    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeRelativeNestedSelector(TokenRange &tokens) noexcept;

    /// @note This doesn't include element names, since they're handled specially.
    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeSimpleSelector(TokenRange &tokens) noexcept;

    KRYS_NODISCARD bool ConsumeName(TokenRange &tokens, CSSOMStringAtom &name,
                                    CSSOMStringAtom &namespacePrefix) noexcept;

    // These will return nullptr when the selector is invalid.
    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeId(TokenRange &tokens) noexcept;
    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeClass(TokenRange &tokens) noexcept;
    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumePseudo(TokenRange &tokens) noexcept;
    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeAttribute(TokenRange &tokens) noexcept;
    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeNesting(TokenRange &tokens) noexcept;

    KRYS_NODISCARD SelectorRelation ConsumeCombinator(TokenRange &tokens) noexcept;
    KRYS_NODISCARD SelectorMatch ConsumeAttributeMatch(TokenRange &tokens) noexcept;
    KRYS_NODISCARD AttributeCaseFlag ConsumeAttributeFlags(TokenRange &tokens) noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &DefaultNamespace() const noexcept;
    KRYS_NODISCARD const CSSOMStringAtom &DetermineNamespace(const CSSOMStringAtom &prefix) const noexcept;
    void PrependTypeSelectorIfNeeded(const CSSOMStringAtom &namespacePrefix,
                                     const CSSOMStringAtom &elementName,
                                     MutableSelector &compoundSelector) noexcept;
    KRYS_NODISCARD static UniquePtr<MutableSelector>
      SplitCompoundAtImplicitShadowCrossingCombinator(UniquePtr<MutableSelector> compoundSelector,
                                                      const SelectorParserContext &context) noexcept;
  };

  KRYS_NODISCARD Maybe<SelectorList> ParseSelectorList(TokenRange tokens,
                                                       const SelectorParserContext &context,
                                                       StyleSheetContents *styleSheet = nullptr,
                                                       NestedContext nestedContext = {}) noexcept;

  KRYS_NODISCARD MutableSelectorList ParseMutableSelectorList(
    TokenRange &tokens, const SelectorParserContext &context, StyleSheetContents *styleSheet,
    NestedContext nestedContext, IsForgivingSelectorList isForgiving,
    DisallowPseudoElements disallowPseudoElements) noexcept;
}

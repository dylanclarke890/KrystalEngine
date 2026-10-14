#pragma once

#include "Krystal.Booey/CSS/Parser/ParserEnum.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Selectors/MutableSelector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/DisallowPseudoElements.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/IsCaseSensitive.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/IsForgivingSelectorList.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
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
    KRYS_NODISCARD IsCaseSensitive ConsumeAttributeFlags(TokenRange &tokens) noexcept;

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

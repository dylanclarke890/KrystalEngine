#pragma once

#include "Krystal.Booey/CSS/Parser/NestedParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/DisallowPseudoElements.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/IsCaseSensitive.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/IsForgivingSelectorList.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class StyleSheetContents;

  class SelectorParser
  {
  private:
    const SelectorParserContext _context;
    const RefPtr<StyleSheetContents> _stylesheet;
    const Maybe<NestedParserContextType> _nestedContext;
    const MutableSelector *_precedingPseudoElement {nullptr};
    bool _ignoreDefaultNamespace {false};
    bool _resistDefaultNamespace {false};
    bool _disallowPseudoElements {false};

  public:
    SelectorParser(const SelectorParserContext &context, StyleSheetContents *stylesheet,
                   Maybe<NestedParserContextType> nestedContext,
                   DisallowPseudoElements disallowPseudoElements) noexcept
        : _context(context), _stylesheet(ShareRefPtr(stylesheet)), _nestedContext(nestedContext),
          _disallowPseudoElements(disallowPseudoElements)
    {
    }

    KRYS_NODISCARD static Maybe<SelectorList>
      ParseSelector(CSSOMString &&input, const SelectorParserContext &context, StyleSheetContents *stylesheet,
                    Maybe<NestedParserContextType> nestedContext) noexcept;

    KRYS_NODISCARD static Maybe<SelectorList>
      ParseSelector(TokenRange tokens, const SelectorParserContext &context, StyleSheetContents *stylesheet,
                    Maybe<NestedParserContextType> nestedContext) noexcept;

    KRYS_NODISCARD MutableSelectorList static ParseMutableSelectorList(
      TokenRange &tokens, const SelectorParserContext &context, StyleSheetContents *stylesheet,
      Maybe<NestedParserContextType> nestedContext, IsForgivingSelectorList isForgiving,
      DisallowPseudoElements disallowPseudoElements) noexcept;

  private:
#pragma region Selector List Parsing

    template <typename ConsumeSelector>
    KRYS_NODISCARD MutableSelectorList ConsumeSelectorList(TokenRange &tokens,
                                                              ConsumeSelector &&consumeSelector) noexcept;

    KRYS_NODISCARD MutableSelectorList ConsumeComplexSelectorList(TokenRange &tokens) noexcept;

    MutableSelectorList ConsumeRelativeSelectorList(TokenRange &tokens) noexcept;

    MutableSelectorList ConsumeNestedSelectorList(TokenRange &tokens) noexcept;

    template <typename ConsumeSelector>
    KRYS_NODISCARD MutableSelectorList
      ConsumeForgivingSelectorList(TokenRange &tokens, ConsumeSelector &&consumeSelector) noexcept;

    KRYS_NODISCARD MutableSelectorList ConsumeComplexForgivingSelectorList(TokenRange &tokens) noexcept;

    KRYS_NODISCARD MutableSelectorList
      ConsumeNestedComplexForgivingSelectorList(TokenRange &tokens) noexcept;

#pragma endregion

#pragma region Simple Selector Parsing

    KRYS_NODISCARD bool ConsumeName(TokenRange &tokens, CSSOMStringAtom &name,
                                    CSSOMStringAtom &namespacePrefix) noexcept;

    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeId(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeClass(TokenRange &tokens) noexcept;

    KRYS_NODISCARD Maybe<SelectorMatch> ConsumeAttributeMatch(TokenRange &tokens) noexcept;

    KRYS_NODISCARD Maybe<IsCaseSensitive> ConsumeAttributeCaseSensitiveFlag(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeAttribute(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeNesting(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumePseudo(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeSimpleSelector(TokenRange &tokens) noexcept;

    KRYS_NODISCARD bool
      IsSimpleSelectorValidAfterPseudoElement(const MutableSelector &simpleSelector,
                                              const MutableSelector &compoundPseudoElement) const noexcept;

#pragma endregion

    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeCompoundSelector(TokenRange &tokens) noexcept;

    KRYS_NODISCARD SelectorRelation ConsumeCombinator(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeComplexSelector(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeNestedComplexSelector(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeRelativeScopeSelector(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableSelector> ConsumeRelativeNestedSelector(TokenRange &tokens) noexcept;

    /// @brief Prepends a type selector to the compound selector if needed, based on the namespace prefix and
    /// element name.
    /// @returns true if a type selector was prepended, false if not, or null if an error occurred.
    KRYS_NODISCARD Maybe<bool> PrependTypeSelectorIfNeeded(const CSSOMStringAtom &namespacePrefix,
                                                           const CSSOMStringAtom &elementName,
                                                           MutableSelector &compoundSelector) noexcept;

#pragma region Namespaces

    KRYS_NODISCARD const CSSOMStringAtom &DetermineNamespace(const CSSOMStringAtom &prefix) const noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &DefaultNamespace() const noexcept;

#pragma endregion

    KRYS_NODISCARD bool IsAtEndIgnoringWhitespace(TokenRange tokens) const noexcept;
  };
}
#pragma once

#include "Krystal.Booey/CSS/Parser/Context/NestedParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Selectors/CSSSelectorList.hpp"
#include "Krystal.Booey/CSS/Selectors/CSSSelectorParserContext.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/DisallowPseudoElements.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/IsCaseSensitive.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/IsForgivingSelectorList.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class StyleSheetContents;

  class CSSSelectorParser
  {
  private:
    const CSSSelectorParserContext _context;
    const RefPtr<StyleSheetContents> _stylesheet;
    const Maybe<NestedParserContextType> _nestedContext;
    const MutableCSSSelector *_precedingPseudoElement {nullptr};
    bool _ignoreDefaultNamespace {false};
    bool _resistDefaultNamespace {false};
    bool _disallowPseudoElements {false};

  public:
    CSSSelectorParser(const CSSSelectorParserContext &context, StyleSheetContents *stylesheet,
                      Maybe<NestedParserContextType> nestedContext,
                      DisallowPseudoElements disallowPseudoElements) noexcept
        : _context(context), _stylesheet(ShareRefPtr(stylesheet)), _nestedContext(nestedContext),
          _disallowPseudoElements(disallowPseudoElements)
    {
    }

    KRYS_NODISCARD static Maybe<CSSSelectorList>
      ParseSelector(CSSOMString &&input, const CSSSelectorParserContext &context,
                    StyleSheetContents *stylesheet, Maybe<NestedParserContextType> nestedContext) noexcept;

    KRYS_NODISCARD static Maybe<CSSSelectorList>
      ParseSelector(TokenRange tokens, const CSSSelectorParserContext &context,
                    StyleSheetContents *stylesheet, Maybe<NestedParserContextType> nestedContext) noexcept;

    KRYS_NODISCARD MutableCSSSelectorList static ParseMutableSelectorList(
      TokenRange &tokens, const CSSSelectorParserContext &context, StyleSheetContents *stylesheet,
      Maybe<NestedParserContextType> nestedContext, IsForgivingSelectorList isForgiving,
      DisallowPseudoElements disallowPseudoElements) noexcept;

  private:
#pragma region Selector List Parsing

    template <typename ConsumeSelector>
    KRYS_NODISCARD MutableCSSSelectorList ConsumeSelectorList(TokenRange &tokens,
                                                              ConsumeSelector &&consumeSelector) noexcept;

    KRYS_NODISCARD MutableCSSSelectorList ConsumeComplexSelectorList(TokenRange &tokens) noexcept;

    MutableCSSSelectorList ConsumeRelativeSelectorList(TokenRange &tokens) noexcept;

    MutableCSSSelectorList ConsumeNestedSelectorList(TokenRange &tokens) noexcept;

    template <typename ConsumeSelector>
    KRYS_NODISCARD MutableCSSSelectorList
      ConsumeForgivingSelectorList(TokenRange &tokens, ConsumeSelector &&consumeSelector) noexcept;

    KRYS_NODISCARD MutableCSSSelectorList ConsumeComplexForgivingSelectorList(TokenRange &tokens) noexcept;

    KRYS_NODISCARD MutableCSSSelectorList
      ConsumeNestedComplexForgivingSelectorList(TokenRange &tokens) noexcept;

#pragma endregion

#pragma region Simple Selector Parsing

    KRYS_NODISCARD bool ConsumeName(TokenRange &tokens, CSSOMStringAtom &name,
                                    CSSOMStringAtom &namespacePrefix) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeId(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeClass(TokenRange &tokens) noexcept;

    KRYS_NODISCARD Maybe<SelectorMatch> ConsumeAttributeMatch(TokenRange &tokens) noexcept;

    KRYS_NODISCARD Maybe<IsCaseSensitive> ConsumeAttributeCaseSensitiveFlag(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeAttribute(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeNesting(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumePseudo(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeSimpleSelector(TokenRange &tokens) noexcept;

    KRYS_NODISCARD bool
      IsSimpleSelectorValidAfterPseudoElement(const MutableCSSSelector &simpleSelector,
                                              const MutableCSSSelector &compoundPseudoElement) const noexcept;

#pragma endregion

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeCompoundSelector(TokenRange &tokens) noexcept;

    KRYS_NODISCARD SelectorRelation ConsumeCombinator(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeComplexSelector(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeNestedComplexSelector(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeRelativeScopeSelector(TokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeRelativeNestedSelector(TokenRange &tokens) noexcept;

    /// @brief Prepends a type selector to the compound selector if needed, based on the namespace prefix and
    /// element name.
    /// @returns true if a type selector was prepended, false if not, or null if an error occurred.
    KRYS_NODISCARD Maybe<bool> PrependTypeSelectorIfNeeded(const CSSOMStringAtom &namespacePrefix,
                                                           const CSSOMStringAtom &elementName,
                                                           MutableCSSSelector &compoundSelector) noexcept;

#pragma region Namespaces

    KRYS_NODISCARD const CSSOMStringAtom &DetermineNamespace(const CSSOMStringAtom &prefix) const noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &DefaultNamespace() const noexcept;

#pragma endregion

    KRYS_NODISCARD bool IsAtEndIgnoringWhitespace(TokenRange tokens) const noexcept;
  };
}
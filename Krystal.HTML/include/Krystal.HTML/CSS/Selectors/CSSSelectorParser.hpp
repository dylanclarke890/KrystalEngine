#pragma once

#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Parser/Enums/NestedContextType.hpp"
#include "Krystal.HTML/CSS/Selectors/CSSSelectorList.hpp"
#include "Krystal.HTML/CSS/Selectors/CSSSelectorParserContext.hpp"
#include "Krystal.HTML/CSS/Selectors/Types/DisallowPseudoElements.hpp"
#include "Krystal.HTML/CSS/Selectors/Types/IsCaseSensitive.hpp"
#include "Krystal.HTML/CSS/Selectors/Types/IsForgivingSelectorList.hpp"
#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class StyleSheetContents;

  class CSSSelectorParser
  {
  private:
    const CSSSelectorParserContext _context;
    const RefPtr<StyleSheetContents> _stylesheet;
    const Maybe<NestedContextType> _nestedContext;
    RawPtr<const MutableCSSSelector> _precedingPseudoElement {nullptr};
    bool _ignoreDefaultNamespace {false};
    bool _resistDefaultNamespace {false};
    bool _disallowPseudoElements {false};

  public:
    CSSSelectorParser(const CSSSelectorParserContext &context, RawPtr<StyleSheetContents> stylesheet,
                      Maybe<NestedContextType> nestedContext,
                      DisallowPseudoElements disallowPseudoElements) noexcept
        : _context(context), _stylesheet(ShareRefPtr(stylesheet)), _nestedContext(nestedContext),
          _disallowPseudoElements(disallowPseudoElements)
    {
    }

    KRYS_NODISCARD static Maybe<CSSSelectorList>
      ParseSelector(CSSOMString &&input, const CSSSelectorParserContext &context,
                    RawPtr<StyleSheetContents> stylesheet, Maybe<NestedContextType> nestedContext) noexcept;

    KRYS_NODISCARD static Maybe<CSSSelectorList>
      ParseSelector(CSSTokenRange tokens, const CSSSelectorParserContext &context,
                    RawPtr<StyleSheetContents> stylesheet, Maybe<NestedContextType> nestedContext) noexcept;

    KRYS_NODISCARD MutableCSSSelectorList static ParseMutableSelectorList(
      CSSTokenRange &tokens, const CSSSelectorParserContext &context, RawPtr<StyleSheetContents> stylesheet,
      Maybe<NestedContextType> nestedContext, IsForgivingSelectorList isForgiving,
      DisallowPseudoElements disallowPseudoElements) noexcept;

  private:
#pragma region Selector List Parsing

    template <typename ConsumeSelector>
    KRYS_NODISCARD MutableCSSSelectorList ConsumeSelectorList(CSSTokenRange &tokens,
                                                              ConsumeSelector &&consumeSelector) noexcept;

    KRYS_NODISCARD MutableCSSSelectorList ConsumeComplexSelectorList(CSSTokenRange &tokens) noexcept;

    MutableCSSSelectorList ConsumeRelativeSelectorList(CSSTokenRange &tokens) noexcept;

    MutableCSSSelectorList ConsumeNestedSelectorList(CSSTokenRange &tokens) noexcept;

    template <typename ConsumeSelector>
    KRYS_NODISCARD MutableCSSSelectorList
      ConsumeForgivingSelectorList(CSSTokenRange &tokens, ConsumeSelector &&consumeSelector) noexcept;

    KRYS_NODISCARD MutableCSSSelectorList ConsumeComplexForgivingSelectorList(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD MutableCSSSelectorList
      ConsumeNestedComplexForgivingSelectorList(CSSTokenRange &tokens) noexcept;

#pragma endregion

#pragma region Simple Selector Parsing

    KRYS_NODISCARD bool ConsumeName(CSSTokenRange &tokens, CSSOMStringAtom &name,
                                    CSSOMStringAtom &namespacePrefix) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeId(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeClass(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD Maybe<SelectorMatch> ConsumeAttributeMatch(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD Maybe<IsCaseSensitive> ConsumeAttributeCaseSensitiveFlag(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeAttribute(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeNesting(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumePseudo(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeSimpleSelector(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD bool
      IsSimpleSelectorValidAfterPseudoElement(const MutableCSSSelector &simpleSelector,
                                              const MutableCSSSelector &compoundPseudoElement) const noexcept;

#pragma endregion

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeCompoundSelector(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD SelectorRelation ConsumeCombinator(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeComplexSelector(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeNestedComplexSelector(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeRelativeScopeSelector(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD UniquePtr<MutableCSSSelector> ConsumeRelativeNestedSelector(CSSTokenRange &tokens) noexcept;

    /// @brief Prepends a type selector to the compound selector if needed, based on the namespace prefix and
    /// element name.
    /// @returns true if a type selector was prepended, false if not, or Null if an error occurred.
    KRYS_NODISCARD Maybe<bool> PrependTypeSelectorIfNeeded(const CSSOMStringAtom &namespacePrefix,
                                                           const CSSOMStringAtom &elementName,
                                                           MutableCSSSelector &compoundSelector) noexcept;

#pragma region Namespaces

    KRYS_NODISCARD const CSSOMStringAtom &DetermineNamespace(const CSSOMStringAtom &prefix) const noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &DefaultNamespace() const noexcept;

#pragma endregion

    KRYS_NODISCARD bool IsAtEndIgnoringWhitespace(CSSTokenRange tokens) const noexcept;
  };
}
#pragma once

#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/IsCaseSensitive.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/MatchLowercase.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::css
{
  struct SelectorParserContext;

  class MutableSelector;
  using MutableSelectorList = List<UniquePtr<MutableSelector>>;

  /// @brief A simple selector and the chain of simple selectors preceding it, as built by the parser. A
  /// SelectorList flattens these into its array.
  class MutableSelector
  {
  private:
    UniquePtr<Selector> _selector;
    UniquePtr<MutableSelector> _precedingComplexSelectorComponent;

  public:
    struct SimpleSelectorTag
    {
    };

    MutableSelector() noexcept;

    /// @brief Recursively copies the selector chain.
    explicit MutableSelector(const Selector &selector) noexcept;

    /// @brief Only copies the simple selector.
    MutableSelector(const Selector &selector, SimpleSelectorTag) noexcept;

    explicit MutableSelector(const dom::QualifiedName &tagQualifiedName) noexcept;

    ~MutableSelector() noexcept;

    KRYS_NODISCARD static UniquePtr<MutableSelector>
      ParsePseudoClassSelector(CSSOMStringView name, const SelectorParserContext &context) noexcept;
    KRYS_NODISCARD static UniquePtr<MutableSelector>
      ParsePseudoElementSelector(CSSOMStringView name, const SelectorParserContext &context) noexcept;
    KRYS_NODISCARD static UniquePtr<MutableSelector> ParsePagePseudoSelector(CSSOMStringView name) noexcept;

    KRYS_NODISCARD UniquePtr<Selector> ReleaseSelector() noexcept
    {
      return krys::move(_selector);
    }

    KRYS_NODISCARD const Selector *GetSelector() const noexcept
    {
      return _selector.get();
    }

    KRYS_NODISCARD Selector *GetSelector() noexcept
    {
      return _selector.get();
    }

    void SetValue(const CSSOMStringAtom &value,
                  MatchLowercase matchLowercase = MatchLowercase(false)) noexcept
    {
      _selector->SetValue(value, matchLowercase);
    }

    KRYS_NODISCARD const CSSOMStringAtom &Value() const noexcept
    {
      return _selector->Value();
    }

    void SetAttribute(const dom::QualifiedName &name, IsCaseSensitive caseSensitive) noexcept
    {
      _selector->SetAttribute(name, caseSensitive);
    }

    void SetArgument(const CSSOMStringAtom &value) noexcept
    {
      _selector->SetArgument(value);
    }

    void SetNth(int32 a, int32 b) noexcept
    {
      _selector->SetNth(a, b);
    }

    void SetMatch(SelectorMatch match) noexcept
    {
      _selector->SetMatch(match);
    }

    void SetRelation(SelectorRelation relation) noexcept
    {
      _selector->SetRelation(relation);
    }

    void SetForPage() noexcept
    {
      _selector->SetForPage();
    }

    KRYS_NODISCARD SelectorMatch Match() const noexcept
    {
      return _selector->Match();
    }

    KRYS_NODISCARD SelectorPseudoElement PseudoElement() const noexcept
    {
      return _selector->PseudoElement();
    }

    KRYS_NODISCARD const SelectorList *SubSelectors() const noexcept
    {
      return _selector->SubSelectors();
    }

    void SetPseudoElement(SelectorPseudoElement type) noexcept
    {
      _selector->SetPseudoElement(type);
    }

    void SetPseudoClass(SelectorPseudoClass type) noexcept
    {
      _selector->SetPseudoClass(type);
    }

    void AdoptSelectorVector(MutableSelectorList &&selectorVector) noexcept;
    void SetArgumentList(FixedVector<CSSOMStringAtom> list) noexcept;
    void SetLangList(FixedVector<PossiblyQuotedIdentifier> list) noexcept;
    void SetSubSelectors(UniquePtr<SelectorList> selectorList) noexcept;

    void SetImplicit() noexcept
    {
      _selector->SetImplicit();
    }

    KRYS_NODISCARD SelectorPseudoClass PseudoClass() const noexcept
    {
      return _selector->PseudoClass();
    }

    KRYS_NODISCARD bool MatchesPseudoElement() const noexcept;

    KRYS_NODISCARD bool IsHostPseudoClass() const noexcept
    {
      return _selector->IsHostPseudoClass();
    }

    KRYS_NODISCARD bool HasExplicitNestingParent() const noexcept;
    KRYS_NODISCARD bool HasExplicitPseudoClassScope() const noexcept;

    // FIXME: "slotted" was removed here for now, since it leads to a combinator connection of
    // ShadowDescendant, and the current shadow DOM code doesn't expect this. When we do fix this issue, make
    // sure to patch the namespace prependTag code to remove the slotted special case, since it will be
    // covered by this function once again.
    KRYS_NODISCARD bool NeedsImplicitShadowCombinatorForMatching() const noexcept
    {
      return Match() == SelectorMatch::PseudoElement
             && (PseudoElement() == SelectorPseudoElement::UserAgentPart
                 || PseudoElement() == SelectorPseudoElement::Cue
                 || PseudoElement() == SelectorPseudoElement::Part
                 || PseudoElement() == SelectorPseudoElement::Slotted);
    }

    KRYS_NODISCARD MutableSelector *PrecedingComplexSelectorComponent() const noexcept
    {
      return _precedingComplexSelectorComponent.get();
    }

    KRYS_NODISCARD MutableSelector *LeftmostSimpleSelector() noexcept;
    KRYS_NODISCARD const MutableSelector *LeftmostSimpleSelector() const noexcept;
    KRYS_NODISCARD bool StartsWithExplicitCombinator() const noexcept;

    void SetPrecedingComplexSelectorComponent(UniquePtr<MutableSelector> selector) noexcept
    {
      _precedingComplexSelectorComponent = krys::move(selector);
    }

    void PrependComplexSelectorComponent(SelectorRelation relation,
                                         UniquePtr<MutableSelector> selector) noexcept;
    void PrependComplexSelectorComponentAsRelative(UniquePtr<MutableSelector> selector) noexcept;
    void AppendTagInComplexSelector(const dom::QualifiedName &tagQualifiedName,
                                    bool tagIsForNamespaceRule = false) noexcept;
    KRYS_NODISCARD UniquePtr<MutableSelector> ReleaseFromComplexSelector() noexcept;
  };
}

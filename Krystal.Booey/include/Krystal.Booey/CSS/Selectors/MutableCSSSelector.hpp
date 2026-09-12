#pragma once

#include "Krystal.Booey/CSS/Selectors/CSSSelector.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/IsCaseSensitive.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/MatchLowercase.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::css
{
  class MutableCSSSelector
  {
  private:
    UniquePtr<CSSSelector> _selector;
    UniquePtr<MutableCSSSelector> _precedingComplexSelectorComponent;

  public:
    MutableCSSSelector() noexcept;

    ~MutableCSSSelector() noexcept;

    explicit MutableCSSSelector(const dom::QualifiedName &name) noexcept;

    KRYS_NODISCARD const CSSSelector *get() const noexcept
    {
      return _selector.get();
    }

    KRYS_NODISCARD CSSSelector *get() noexcept
    {
      return _selector.get();
    }

    KRYS_NODISCARD UniquePtr<CSSSelector> release() noexcept
    {
      return krys::move(_selector);
    }

    void PrependComplexSelectorComponent(SelectorRelation relation,
                                         UniquePtr<MutableCSSSelector> selector) noexcept;

    void AppendTagInComplexSelector(const dom::QualifiedName &tagQualifiedName,
                                    bool tagIsForNamespaceRule) noexcept;

#pragma region Basic setters and getters

    KRYS_NODISCARD MutableCSSSelector *PrecedingComplexSelectorComponent() const noexcept
    {
      return _precedingComplexSelectorComponent.get();
    }

    void SetPrecedingComplexSelectorComponent(UniquePtr<MutableCSSSelector> selector) noexcept
    {
      _precedingComplexSelectorComponent = krys::move(selector);
    }

    KRYS_NODISCARD SelectorRelation Relation() const noexcept
    {
      return _selector->Relation();
    }

    void SetRelation(SelectorRelation relation) noexcept
    {
      _selector->SetRelation(relation);
    }

    KRYS_NODISCARD SelectorMatch Match() const noexcept
    {
      return _selector->Match();
    }

    void SetMatch(SelectorMatch match) noexcept
    {
      _selector->SetMatch(match);
    }

    KRYS_NODISCARD PseudoElementId PseudoElement() const noexcept
    {
      return _selector->PseudoElement();
    }

    void SetPseudoElement(PseudoElementId type) noexcept
    {
      _selector->SetPseudoElement(type);
    }

    KRYS_NODISCARD PseudoClassId PseudoClass() const noexcept
    {
      return _selector->PseudoClass();
    }

    void SetPseudoClass(PseudoClassId type) noexcept
    {
      _selector->SetPseudoClass(type);
    }

    void SetValue(const CSSOMStringAtom &value, MatchLowercase matchLowercase) noexcept
    {
      _selector->SetValue(value, matchLowercase);
    }

    void SetAttribute(const dom::QualifiedName &name, IsCaseSensitive caseSensitive) noexcept
    {
      _selector->SetAttribute(name, caseSensitive);
    }

#pragma endregion

    KRYS_NODISCARD MutableCSSSelector *LeftmostSimpleSelector() noexcept
    {
      auto *selector = this;
      while (auto *next = selector->PrecedingComplexSelectorComponent())
      {
        selector = next;
      }

      return selector;
    }

    KRYS_NODISCARD bool NeedsImplicitShadowCombinatorForMatching() const noexcept
    {
      // return Match() == SelectorMatch::PseudoElement;
      return false; // TODO
    }
  };

  using MutableCSSSelectorList = SmallList<UniquePtr<MutableCSSSelector>>;
}
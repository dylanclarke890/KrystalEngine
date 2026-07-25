#pragma once

#include "Krystal.HTML/CSS/Selectors/CSSSelector.hpp"
#include "Krystal.HTML/CSS/Selectors/Types/IsCaseSensitive.hpp"
#include "Krystal.HTML/CSS/Selectors/Types/MatchLowercase.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  class MutableCSSSelector
  {
  private:
    UniquePtr<CSSSelector> _selector;
    UniquePtr<MutableCSSSelector> _precedingComplexSelectorComponent;

  public:
    MutableCSSSelector() noexcept;

    ~MutableCSSSelector() noexcept;

    explicit MutableCSSSelector(const QualifiedName &name) noexcept;

    KRYS_NODISCARD RawPtr<const CSSSelector> get() const noexcept
    {
      return _selector.get();
    }

    KRYS_NODISCARD RawPtr<CSSSelector> get() noexcept
    {
      return _selector.get();
    }

    KRYS_NODISCARD UniquePtr<CSSSelector> release() noexcept
    {
      return Krys::Move(_selector);
    }

    void PrependComplexSelectorComponent(SelectorRelation relation,
                                         UniquePtr<MutableCSSSelector> selector) noexcept;

#pragma region Basic setters and getters

    KRYS_NODISCARD RawPtr<MutableCSSSelector> PrecedingComplexSelectorComponent() const noexcept
    {
      return _precedingComplexSelectorComponent.get();
    }

    void SetPrecedingComplexSelectorComponent(UniquePtr<MutableCSSSelector> selector) noexcept
    {
      _precedingComplexSelectorComponent = Krys::Move(selector);
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

    void SetAttribute(const QualifiedName &name, IsCaseSensitive caseSensitive) noexcept
    {
      _selector->SetAttribute(name, caseSensitive);
    }

#pragma endregion

    KRYS_NODISCARD RawPtr<MutableCSSSelector> LeftmostSimpleSelector() noexcept
    {
      auto *selector = this;
      while (auto *next = selector->PrecedingComplexSelectorComponent())
      {
        selector = next;
      }

      return selector;
    }
  };

  using MutableCSSSelectorList = SmallList<UniquePtr<MutableCSSSelector>>;
}
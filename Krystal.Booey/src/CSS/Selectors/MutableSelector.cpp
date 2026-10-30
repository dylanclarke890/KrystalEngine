#include "Krystal.Booey/CSS/Selectors/MutableSelector.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorInlines.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorPseudoTypeMap.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  UniquePtr<MutableSelector> MutableSelector::ParsePagePseudoSelector(CSSOMStringView name) noexcept
  {
    SelectorPagePseudoClass pseudoType;
    if (text::IsASCIICaselessEqual(name, CSSOMStringView {u8"first"}))
    {
      pseudoType = SelectorPagePseudoClass::First;
    }
    else if (text::IsASCIICaselessEqual(name, CSSOMStringView {u8"left"}))
    {
      pseudoType = SelectorPagePseudoClass::Left;
    }
    else if (text::IsASCIICaselessEqual(name, CSSOMStringView {u8"right"}))
    {
      pseudoType = SelectorPagePseudoClass::Right;
    }
    else
    {
      return nullptr;
    }

    auto selector = CreateUnique<MutableSelector>();
    selector->_selector->SetMatch(SelectorMatch::PagePseudoClass);
    selector->_selector->SetPagePseudoClass(pseudoType);
    return selector;
  }

  UniquePtr<MutableSelector>
    MutableSelector::ParsePseudoElementSelector(CSSOMStringView name,
                                                const SelectorParserContext &context) noexcept
  {
    auto pseudoType = Selector::ParsePseudoElementName(name, context);
    if (!pseudoType)
    {
      return nullptr;
    }

    auto selector = CreateUnique<MutableSelector>();
    selector->_selector->SetMatch(SelectorMatch::PseudoElement);
    selector->_selector->SetPseudoElement(*pseudoType);
    selector->_selector->SetValue(CSSOMStringAtom(text::ToASCIILower(name)));
    return selector;
  }

  UniquePtr<MutableSelector>
    MutableSelector::ParsePseudoClassSelector(CSSOMStringView name,
                                              const SelectorParserContext &context) noexcept
  {
    auto pseudoType = FindPseudoClassAndCompatibilityElementName(name);
    if (pseudoType.PseudoClass)
    {
      if (!Selector::IsPseudoClassEnabled(*pseudoType.PseudoClass, context))
      {
        return nullptr;
      }
      auto selector = CreateUnique<MutableSelector>();
      selector->_selector->SetMatch(SelectorMatch::PseudoClass);
      selector->_selector->SetPseudoClass(*pseudoType.PseudoClass);
      return selector;
    }
    if (pseudoType.CompatibilityPseudoElement)
    {
      krys_debug_assert(
        Selector::IsPseudoElementEnabled(*pseudoType.CompatibilityPseudoElement, name, context));
      auto selector = CreateUnique<MutableSelector>();
      selector->_selector->SetMatch(SelectorMatch::PseudoElement);
      selector->_selector->SetPseudoElement(*pseudoType.CompatibilityPseudoElement);
      selector->_selector->SetValue(CSSOMStringAtom(text::ToASCIILower(name)));
      return selector;
    }
    return nullptr;
  }

  MutableSelector::MutableSelector() noexcept : _selector(CreateUnique<Selector>())
  {
  }

  MutableSelector::MutableSelector(const dom::QualifiedName &tagQualifiedName) noexcept
      : _selector(CreateUnique<Selector>(tagQualifiedName))
  {
  }

  MutableSelector::MutableSelector(const Selector &selector) noexcept
      : _selector(CreateUnique<Selector>(selector, Selector::MutableSelectorCopyTag {}))
  {
    if (const auto *preceding = selector.PrecedingComplexSelectorComponent())
    {
      _precedingComplexSelectorComponent = CreateUnique<MutableSelector>(*preceding);
    }
  }

  MutableSelector::MutableSelector(const Selector &selector, SimpleSelectorTag) noexcept
      : _selector(CreateUnique<Selector>(selector, Selector::MutableSelectorCopyTag {}))
  {
  }

  MutableSelector::~MutableSelector() noexcept
  {
    if (!_precedingComplexSelectorComponent)
    {
      return;
    }

    // Unlink the chain iteratively so a long chain doesn't overflow the stack.
    SmallList<UniquePtr<MutableSelector>, 16uz> toDelete;
    UniquePtr<MutableSelector> selector = krys::move(_precedingComplexSelectorComponent);
    while (true)
    {
      UniquePtr<MutableSelector> next = krys::move(selector->_precedingComplexSelectorComponent);
      toDelete.push_back(krys::move(selector));
      if (!next)
      {
        break;
      }
      selector = krys::move(next);
    }
  }

  void MutableSelector::AdoptSelectorVector(MutableSelectorList &&selectorVector) noexcept
  {
    _selector->SetSubSelectors(CreateUnique<SelectorList>(krys::move(selectorVector)));
  }

  void MutableSelector::SetArgumentList(FixedVector<CSSOMStringAtom> list) noexcept
  {
    krys_debug_assert(!list.isEmpty());
    _selector->SetArgumentList(krys::move(list));
  }

  void MutableSelector::SetLangList(FixedVector<PossiblyQuotedIdentifier> list) noexcept
  {
    krys_debug_assert(!list.isEmpty());
    _selector->SetLangList(krys::move(list));
  }

  void MutableSelector::SetSubSelectors(UniquePtr<SelectorList> selectorList) noexcept
  {
    _selector->SetSubSelectors(krys::move(selectorList));
  }

  const MutableSelector *MutableSelector::LeftmostSimpleSelector() const noexcept
  {
    const auto *selector = this;
    while (const auto *next = selector->PrecedingComplexSelectorComponent())
    {
      selector = next;
    }
    return selector;
  }

  MutableSelector *MutableSelector::LeftmostSimpleSelector() noexcept
  {
    auto *selector = this;
    while (auto *next = selector->PrecedingComplexSelectorComponent())
    {
      selector = next;
    }
    return selector;
  }

  bool MutableSelector::HasExplicitNestingParent() const noexcept
  {
    const auto *selector = this;
    while (selector)
    {
      if (selector->GetSelector()->HasExplicitNestingParent())
      {
        return true;
      }

      selector = selector->PrecedingComplexSelectorComponent();
    }
    return false;
  }

  bool MutableSelector::HasExplicitPseudoClassScope() const noexcept
  {
    const auto *selector = this;
    while (selector)
    {
      if (selector->GetSelector()->HasExplicitPseudoClassScope())
      {
        return true;
      }

      selector = selector->PrecedingComplexSelectorComponent();
    }
    return false;
  }

  namespace
  {
    KRYS_NODISCARD bool SelectorListMatchesPseudoElement(const SelectorList *selectorList) noexcept
    {
      if (!selectorList)
      {
        return false;
      }

      for (const auto &subSelector : *selectorList)
      {
        for (const auto *selector = &subSelector; selector;
             selector = selector->PrecedingComplexSelectorComponent())
        {
          if (selector->MatchesPseudoElement())
          {
            return true;
          }
          if (SelectorListMatchesPseudoElement(selector->SubSelectors()))
          {
            return true;
          }
        }
      }
      return false;
    }
  }

  bool MutableSelector::MatchesPseudoElement() const noexcept
  {
    return _selector->MatchesPseudoElement() || SelectorListMatchesPseudoElement(_selector->SubSelectors());
  }

  void MutableSelector::PrependComplexSelectorComponent(SelectorRelation relation,
                                                        UniquePtr<MutableSelector> selector) noexcept
  {
    auto *first = this;
    while (first->PrecedingComplexSelectorComponent())
    {
      first = first->PrecedingComplexSelectorComponent();
    }

    first->SetRelation(relation);
    first->SetPrecedingComplexSelectorComponent(krys::move(selector));
  }

  void
    MutableSelector::PrependComplexSelectorComponentAsRelative(UniquePtr<MutableSelector> selector) noexcept
  {
    auto *firstSelector = LeftmostSimpleSelector()->GetSelector();
    krys_debug_assert(firstSelector);

    // Relation is Descendant by default.
    auto relation = firstSelector->Relation();
    if (relation == SelectorRelation::Compounding)
    {
      relation = SelectorRelation::Descendant;
    }

    PrependComplexSelectorComponent(relation, krys::move(selector));
  }

  void MutableSelector::AppendTagInComplexSelector(const dom::QualifiedName &tagQualifiedName,
                                                   bool tagIsForNamespaceRule) noexcept
  {
    // Make the current last selector the second to last.
    auto currentLast = CreateUnique<MutableSelector>();
    currentLast->_selector = krys::move(_selector);
    currentLast->_precedingComplexSelectorComponent = krys::move(_precedingComplexSelectorComponent);
    _precedingComplexSelectorComponent = krys::move(currentLast);

    // Change the last selector to be the tag selector.
    _selector = CreateUnique<Selector>(tagQualifiedName, tagIsForNamespaceRule);
    _selector->SetRelation(SelectorRelation::Compounding);
  }

  UniquePtr<MutableSelector> MutableSelector::ReleaseFromComplexSelector() noexcept
  {
    SetRelation(SelectorRelation::Compounding);
    return krys::move(_precedingComplexSelectorComponent);
  }

  bool MutableSelector::StartsWithExplicitCombinator() const noexcept
  {
    auto relation = LeftmostSimpleSelector()->GetSelector()->Relation();
    return relation != SelectorRelation::Compounding && relation != SelectorRelation::Descendant;
  }
}

#include "Krystal.Booey/CSS/Selectors/MutableSelector.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorInlines.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorPseudoTypeMap.hpp"
#include "Krystal.Core/Containers/SmallList.hpp"
#include <utility>

namespace krys::boo::css
{
  namespace
  {
    /// @brief Whether a simple selector in @p selectorList, or nested in their arguments, matches a
    /// pseudo-element.
    KRYS_NODISCARD bool SelectorListMatchesPseudoElement(const SelectorList *selectorList) noexcept
    {
      if (!selectorList)
      {
        return false;
      }
      for (const auto &complexSelector : *selectorList)
      {
        for (const auto *selector = &complexSelector; selector;
             selector = selector->PrecedingComplexSelectorComponent())
        {
          if (selector->MatchesPseudoElement() || SelectorListMatchesPseudoElement(selector->SubSelectors()))
          {
            return true;
          }
        }
      }
      return false;
    }

    KRYS_NODISCARD UniquePtr<MutableSelector> PseudoElementSelector(SelectorPseudoElement pseudoElement,
                                                                    CSSOMStringView name) noexcept
    {
      auto selector = CreateUnique<MutableSelector>();
      selector->SetMatch(SelectorMatch::PseudoElement);
      selector->SetPseudoElement(pseudoElement);
      selector->SetValue(CSSOMStringAtom(text::ToASCIILower(name)));
      return selector;
    }
  }

  MutableSelector::MutableSelector() noexcept : _selector(CreateUnique<Selector>())
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

  MutableSelector::MutableSelector(const QualifiedName &tagQualifiedName) noexcept
      : _selector(CreateUnique<Selector>(tagQualifiedName))
  {
  }

  MutableSelector::~MutableSelector() noexcept
  {
    // Each component owns the next, so destroying them recursively could overflow the stack on a long chain.
    // Detach them first; they're destroyed with `chain`, each with nothing left to own.
    SmallList<UniquePtr<MutableSelector>, 16uz> chain;
    for (auto next = krys::move(_precedingComplexSelectorComponent); next;)
    {
      auto after = krys::move(next->_precedingComplexSelectorComponent);
      chain.push_back(krys::move(next));
      next = krys::move(after);
    }
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
      selector->SetMatch(SelectorMatch::PseudoClass);
      selector->SetPseudoClass(*pseudoType.PseudoClass);
      return selector;
    }
    // ::before, ::after, ::first-line and ::first-letter can be written with one colon.
    if (pseudoType.CompatibilityPseudoElement)
    {
      krys_debug_assert(
        Selector::IsPseudoElementEnabled(*pseudoType.CompatibilityPseudoElement, name, context));
      return PseudoElementSelector(*pseudoType.CompatibilityPseudoElement, name);
    }
    return nullptr;
  }

  UniquePtr<MutableSelector>
    MutableSelector::ParsePseudoElementSelector(CSSOMStringView name,
                                                const SelectorParserContext &context) noexcept
  {
    auto pseudoElement = Selector::ParsePseudoElementName(name, context);
    return pseudoElement ? PseudoElementSelector(*pseudoElement, name) : nullptr;
  }

  void MutableSelector::AdoptSelectorVector(MutableSelectorList &&selectorVector) noexcept
  {
    _selector->SetSubSelectors(CreateUnique<SelectorList>(krys::move(selectorVector)));
  }

  void MutableSelector::SetArgumentList(FixedList<CSSOMStringAtom> list) noexcept
  {
    krys_debug_assert(!list.empty());
    _selector->SetArgumentList(krys::move(list));
  }

  void MutableSelector::SetLangList(FixedList<PossiblyQuotedIdentifier> list) noexcept
  {
    krys_debug_assert(!list.empty());
    _selector->SetLangList(krys::move(list));
  }

  void MutableSelector::SetIntegerList(FixedList<int32> list) noexcept
  {
    krys_debug_assert(!list.empty());
    _selector->SetIntegerList(krys::move(list));
  }

  void MutableSelector::SetSubSelectors(UniquePtr<SelectorList> selectorList) noexcept
  {
    _selector->SetSubSelectors(krys::move(selectorList));
  }

  bool MutableSelector::MatchesPseudoElement() const noexcept
  {
    return _selector->MatchesPseudoElement() || SelectorListMatchesPseudoElement(_selector->SubSelectors());
  }

  bool MutableSelector::HasExplicitNestingParent() const noexcept
  {
    return AnyInChain([](const Selector &selector) { return selector.HasExplicitNestingParent(); });
  }

  bool MutableSelector::HasExplicitPseudoClassScope() const noexcept
  {
    return AnyInChain([](const Selector &selector) { return selector.HasExplicitPseudoClassScope(); });
  }

  const MutableSelector *MutableSelector::LeftmostSimpleSelector() const noexcept
  {
    const auto *selector = this;
    while (const auto *preceding = selector->PrecedingComplexSelectorComponent())
    {
      selector = preceding;
    }
    return selector;
  }

  MutableSelector *MutableSelector::LeftmostSimpleSelector() noexcept
  {
    return const_cast<MutableSelector *>(std::as_const(*this).LeftmostSimpleSelector());
  }

  bool MutableSelector::StartsWithExplicitCombinator() const noexcept
  {
    auto relation = LeftmostSimpleSelector()->_selector->Relation();
    return relation != SelectorRelation::Compounding && relation != SelectorRelation::Descendant;
  }

  void MutableSelector::PrependComplexSelectorComponent(SelectorRelation relation,
                                                        UniquePtr<MutableSelector> selector) noexcept
  {
    auto *leftmost = LeftmostSimpleSelector();
    leftmost->SetRelation(relation);
    leftmost->SetPrecedingComplexSelectorComponent(krys::move(selector));
  }

  void
    MutableSelector::PrependComplexSelectorComponentAsRelative(UniquePtr<MutableSelector> selector) noexcept
  {
    // A relative selector's leading combinator is kept on its leftmost simple selector; without one, it's
    // still compounding there, and a descendant combinator is implied.
    auto relation = LeftmostSimpleSelector()->_selector->Relation();
    PrependComplexSelectorComponent(relation == SelectorRelation::Compounding ? SelectorRelation::Descendant
                                                                              : relation,
                                    krys::move(selector));
  }

  void MutableSelector::AppendTagInComplexSelector(const QualifiedName &tagQualifiedName,
                                                   bool tagIsForNamespaceRule) noexcept
  {
    auto previous = CreateUnique<MutableSelector>();
    previous->_selector = krys::move(_selector);
    previous->_precedingComplexSelectorComponent = krys::move(_precedingComplexSelectorComponent);
    _precedingComplexSelectorComponent = krys::move(previous);

    _selector = CreateUnique<Selector>(tagQualifiedName, tagIsForNamespaceRule);
    _selector->SetRelation(SelectorRelation::Compounding);
  }

  UniquePtr<MutableSelector> MutableSelector::ReleaseFromComplexSelector() noexcept
  {
    SetRelation(SelectorRelation::Compounding);
    return krys::move(_precedingComplexSelectorComponent);
  }
}

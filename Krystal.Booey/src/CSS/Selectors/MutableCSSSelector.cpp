#include "Krystal.Booey/CSS/Selectors/MutableCSSSelector.hpp"
#include "Krystal.Booey/CSS/Selectors/CSSSelector.hpp"

namespace krys::boo::css
{
  MutableCSSSelector::MutableCSSSelector() noexcept
      : _selector(CreateUnique<CSSSelector>()), _precedingComplexSelectorComponent(nullptr)
  {
  }

  MutableCSSSelector::~MutableCSSSelector() noexcept
  {
    if (_precedingComplexSelectorComponent == nullptr)
    {
      return;
    }

    SmallList<UniquePtr<MutableCSSSelector>, 16uz> toDelete;
    UniquePtr<MutableCSSSelector> selector = krys::move(_precedingComplexSelectorComponent);
    while (true)
    {
      UniquePtr<MutableCSSSelector> next = krys::move(selector->_precedingComplexSelectorComponent);
      toDelete.push_back(krys::move(selector));

      if (next == nullptr)
      {
        break;
      }

      selector = krys::move(next);
    }
  }

  MutableCSSSelector::MutableCSSSelector(const QualifiedName &name) noexcept
      : _selector(CreateUnique<CSSSelector>(name)), _precedingComplexSelectorComponent(nullptr)
  {
  }

  void MutableCSSSelector::PrependComplexSelectorComponent(SelectorRelation relation,
                                                           UniquePtr<MutableCSSSelector> selector) noexcept
  {
    auto *first = this;
    while (first->PrecedingComplexSelectorComponent())
    {
      first = first->PrecedingComplexSelectorComponent();
    }

    first->SetRelation(relation);
    first->SetPrecedingComplexSelectorComponent(krys::move(selector));
  }

  void MutableCSSSelector::AppendTagInComplexSelector(const QualifiedName &tagQualifiedName,
                                                      bool tagIsForNamespaceRule) noexcept
  {
    // Make the current last selector the second to last.
    auto currentLast = CreateUnique<MutableCSSSelector>();
    currentLast->_selector = krys::move(_selector);
    currentLast->_precedingComplexSelectorComponent = krys::move(_precedingComplexSelectorComponent);
    _precedingComplexSelectorComponent = krys::move(currentLast);

    // Change the last selector to be the tag selector.
    _selector = CreateUnique<CSSSelector>(tagQualifiedName, tagIsForNamespaceRule);
    _selector->SetRelation(SelectorRelation::Compounding);
  }
}
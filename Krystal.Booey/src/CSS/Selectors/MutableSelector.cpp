#include "Krystal.Booey/CSS/Selectors/MutableSelector.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"

namespace krys::boo::css
{
  MutableSelector::MutableSelector() noexcept
      : _selector(CreateUnique<Selector>()), _precedingComplexSelectorComponent(nullptr)
  {
  }

  MutableSelector::~MutableSelector() noexcept
  {
    if (_precedingComplexSelectorComponent == nullptr)
    {
      return;
    }

    SmallList<UniquePtr<MutableSelector>, 16uz> toDelete;
    UniquePtr<MutableSelector> selector = krys::move(_precedingComplexSelectorComponent);
    while (true)
    {
      UniquePtr<MutableSelector> next = krys::move(selector->_precedingComplexSelectorComponent);
      toDelete.push_back(krys::move(selector));

      if (next == nullptr)
      {
        break;
      }

      selector = krys::move(next);
    }
  }

  MutableSelector::MutableSelector(const dom::QualifiedName &name) noexcept
      : _selector(CreateUnique<Selector>(name)), _precedingComplexSelectorComponent(nullptr)
  {
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
}
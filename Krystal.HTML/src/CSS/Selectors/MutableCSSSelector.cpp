#include "Krystal.HTML/CSS/Selectors/MutableCSSSelector.hpp"
#include "Krystal.HTML/CSS/Selectors/CSSSelector.hpp"

namespace Krys::HTML
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
    UniquePtr<MutableCSSSelector> selector = Krys::Move(_precedingComplexSelectorComponent);
    while (true)
    {
      UniquePtr<MutableCSSSelector> next = Krys::Move(selector->_precedingComplexSelectorComponent);
      toDelete.push_back(Krys::Move(selector));

      if (next == nullptr)
      {
        break;
      }

      selector = Krys::Move(next);
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
    first->SetPrecedingComplexSelectorComponent(Krys::Move(selector));
  }
}
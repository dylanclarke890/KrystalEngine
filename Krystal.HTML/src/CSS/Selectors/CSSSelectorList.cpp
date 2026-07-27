#include "Krystal.HTML/CSS/Selectors/CSSSelectorList.hpp"
#include "Krystal.HTML/CSS/Selectors/MutableCSSSelector.hpp"

namespace Krys::HTML
{
#pragma region Constructors

  CSSSelectorList::CSSSelectorList(const CSSSelectorList &other) noexcept
  {
    size_t otherComponentCount = other.ComponentCount();
    if (!otherComponentCount)
      return;

    _selectors = CreateUniqueArray<CSSSelector>(otherComponentCount);
    for (size_t i = 0uz; i < otherComponentCount; ++i)
    {
      new (NotNullTag {}, &_selectors[i]) CSSSelector(other._selectors[i]);
    }
  }

  CSSSelectorList::CSSSelectorList(MutableCSSSelectorList &&selectors) noexcept
  {
    assert(!selectors.empty());

    size_t flattenedSize = 0uz;
    for (size_t i = 0uz; i < selectors.size(); ++i)
    {
      for (auto *selector = selectors[i].get(); selector;
           selector = selector->PrecedingComplexSelectorComponent())
      {
        ++flattenedSize;
      }
    }

    assert(flattenedSize);

    _selectors = CreateUniqueArray<CSSSelector>(flattenedSize);
    size_t arrayIndex = 0uz;

    for (size_t i = 0uz; i < selectors.size(); ++i)
    {
      auto *last = selectors[i].get();
      auto *current = last;

      while (current)
      {
        {
          // Move item from the parser selector vector into _selectors without invoking destructor (Ugh.)
          auto *currentSelector = current->release().release();
          std::memcpy(static_cast<void *>(&_selectors[arrayIndex]), static_cast<void *>(currentSelector),
                      sizeof(CSSSelector));

          // Free the underlying memory without invoking the destructor.
          operator delete(currentSelector);
        }

        if (current != last)
        {
          _selectors[arrayIndex].SetIsLastInComplexSelector(false);
        }

        current = current->PrecedingComplexSelectorComponent();
        assert(!_selectors[arrayIndex].IsLastInSelectorList() || (flattenedSize == arrayIndex + 1uz));

        if (current)
        {
          _selectors[arrayIndex].SetIsFirstInComplexSelector(false);
        }

        ++arrayIndex;
      }

      assert(_selectors[arrayIndex - 1uz].IsFirstInComplexSelector());
    }

    assert(flattenedSize == arrayIndex);
    _selectors[arrayIndex - 1].SetIsLastInSelectorList(true);
  }

  CSSSelectorList::CSSSelectorList(UniqueArray<CSSSelector> &&array) noexcept : _selectors(Krys::Move(array))
  {
  }

  bool CSSSelectorList::operator==(const CSSSelectorList &other) const noexcept
  {
    for (auto a = begin(), b = other.begin(); a != end() || b != other.end(); ++a, ++b)
    {
      if (a == end() || b == other.end())
      {
        return false;
      }
      if (!ComplexSelectorsEqual(*a, *b))
      {
        return false;
      }
    }
    return true;
  }

#pragma endregion

  size_t CSSSelectorList::ComponentCount() const noexcept
  {
    if (_selectors == nullptr)
    {
      return 0uz;
    }

    auto current = _selectors.get();
    while (!current->IsLastInSelectorList())
    {
      ++current;
    }

    return std::distance(_selectors.get(), current) + 1uz;
  }

  size_t CSSSelectorList::SelectorCount() const noexcept
  {
    if (_selectors == nullptr)
    {
      return 0uz;
    }

    size_t size = 1uz;
    auto current = _selectors.get();
    while (!current->IsLastInSelectorList())
    {
      if (current->IsFirstInComplexSelector())
      {
        ++size;
      }

      ++current;
    }

    return size;
  }

  bool CSSSelectorList::HasExplicitNestingParent() const noexcept
  {
    auto functor = [](RawPtr<const CSSSelector> selector) noexcept -> bool
    {
      return selector->HasExplicitNestingParent();
    };

    return ForEachSelector(functor);
  }

  bool CSSSelectorList::HasOnlyNestingSelector() const noexcept
  {
    if (ComponentCount() != 1uz)
    {
      return false;
    }

    auto singleSelector = _selectors.get();
    // Selector should be a single selector
    if (singleSelector->PrecedingComplexSelectorComponent())
    {
      return false;
    }

    return singleSelector->Match() == SelectorMatch::NestingParent;
  }

#pragma region Static utility functions

  CSSSelectorList CSSSelectorList::Join(const CSSSelectorList &a, const CSSSelectorList &b) noexcept
  {
    if (a.IsEmpty())
    {
      return b;
    }

    if (b.IsEmpty())
    {
      return a;
    }

    auto aComponentCount = a.ComponentCount();
    auto bComponentCount = b.ComponentCount();

    auto selectors = CreateUniqueArray<CSSSelector>(aComponentCount + bComponentCount);

    for (size_t i = 0; i < aComponentCount; ++i)
    {
      new (NotNullTag {}, &selectors[i]) CSSSelector(a._selectors[i]);
    }

    for (size_t i = 0; i < bComponentCount; ++i)
    {
      new (NotNullTag {}, &selectors[aComponentCount + i]) CSSSelector(b._selectors[i]);
    }

    selectors[aComponentCount - 1uz].SetIsLastInSelectorList(false);
    selectors[aComponentCount + bComponentCount - 1uz].SetIsLastInSelectorList(true);

    return CSSSelectorList {Krys::Move(selectors)};
  }

  CSSSelectorList CSSSelectorList::Join(const SmallList<RawPtr<const CSSSelectorList>> &lists) noexcept
  {
    size_t totalComponentCount = 0uz;
    for (auto list : lists)
    {
      totalComponentCount += list->ComponentCount();
    }

    if (totalComponentCount == 0uz)
    {
      return {};
    }

    auto selectors = CreateUniqueArray<CSSSelector>(totalComponentCount);

    size_t componentIndex = 0uz;
    for (auto list : lists)
    {
      auto count = list->ComponentCount();
      for (size_t i = 0uz; i < count; ++i)
      {
        new (NotNullTag {}, &selectors[componentIndex++]) CSSSelector(list->_selectors[i]);
      }

      selectors[componentIndex - 1uz].SetIsLastInSelectorList(false);
    }

    assert(componentIndex == totalComponentCount);
    selectors[componentIndex - 1uz].SetIsLastInSelectorList(true);

    return CSSSelectorList {Krys::Move(selectors)};
  }

  CSSSelectorList CSSSelectorList::CopySimple(const CSSSelector &simpleSelector) noexcept
  {
    auto selectors = CreateUniqueArray<CSSSelector>(1uz);

    new (NotNullTag {}, &selectors[0uz]) CSSSelector(simpleSelector);
    selectors[0].SetIsFirstInComplexSelector(true);
    selectors[0].SetIsLastInComplexSelector(true);
    selectors[0].SetIsLastInSelectorList(true);

    return CSSSelectorList {Krys::Move(selectors)};
  }

  CSSSelectorList CSSSelectorList::CopyComplex(const CSSSelector &complexSelector) noexcept
  {
    size_t length = 0uz;
    for (auto *selector = &complexSelector; selector;
         selector = selector->PrecedingComplexSelectorComponent())
    {
      ++length;
    }

    auto selectors = CreateUniqueArray<CSSSelector>(length);

    size_t i = 0uz;
    for (auto *selector = &complexSelector; selector;
         selector = selector->PrecedingComplexSelectorComponent(), ++i)
    {
      new (NotNullTag {}, &selectors[i]) CSSSelector(*selector);
    }
    selectors[length - 1uz].SetIsLastInSelectorList(true);

    return CSSSelectorList {Krys::Move(selectors)};
  }

#pragma endregion
}
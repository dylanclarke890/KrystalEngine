#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/Selectors/MutableSelector.hpp"

namespace krys::boo::css
{
#pragma region Constructors

  SelectorList::SelectorList(const SelectorList &other) noexcept
  {
    size_t otherComponentCount = other.ComponentCount();
    if (!otherComponentCount)
      return;

    _selectors = CreateUniqueArray<Selector>(otherComponentCount);
    for (size_t i = 0uz; i < otherComponentCount; ++i)
    {
      new (NotNullTag {}, &_selectors[i]) Selector(other._selectors[i]);
    }
  }

  SelectorList::SelectorList(MutableSelectorList &&selectors) noexcept
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

    _selectors = CreateUniqueArray<Selector>(flattenedSize);
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
                      sizeof(Selector));

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
    _selectors[arrayIndex - 1uz].SetIsLastInSelectorList(true);
  }

  SelectorList::SelectorList(UniqueArray<Selector> &&array) noexcept : _selectors(krys::move(array))
  {
  }

  bool SelectorList::operator==(const SelectorList &other) const noexcept
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

  size_t SelectorList::ComponentCount() const noexcept
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

  size_t SelectorList::SelectorCount() const noexcept
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

  bool SelectorList::HasExplicitNestingParent() const noexcept
  {
    return ForEachSelector([](const Selector *selector) noexcept -> bool
                           { return selector->HasExplicitNestingParent(); });
  }

  bool SelectorList::HasOnlyNestingSelector() const noexcept
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

  CSSOMString SelectorList::SelectorText() const noexcept
  {
    CSSOMString output;

    for (const auto &selector : *this)
    {
      output += selector.SelectorText();
      output += u8", ";
    }

    output.pop_back(); // Remove the last comma
    output.pop_back(); // Remove the last space

    return output;
  }

#pragma region Static utility functions

  SelectorList SelectorList::Join(const SelectorList &a, const SelectorList &b) noexcept
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

    auto selectors = CreateUniqueArray<Selector>(aComponentCount + bComponentCount);

    for (size_t i = 0uz; i < aComponentCount; ++i)
    {
      new (NotNullTag {}, &selectors[i]) Selector(a._selectors[i]);
    }

    for (size_t i = 0uz; i < bComponentCount; ++i)
    {
      new (NotNullTag {}, &selectors[aComponentCount + i]) Selector(b._selectors[i]);
    }

    selectors[aComponentCount - 1uz].SetIsLastInSelectorList(false);
    selectors[aComponentCount + bComponentCount - 1uz].SetIsLastInSelectorList(true);

    return SelectorList {krys::move(selectors)};
  }

  SelectorList SelectorList::Join(const SmallList<const SelectorList *> &lists) noexcept
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

    auto selectors = CreateUniqueArray<Selector>(totalComponentCount);

    size_t componentIndex = 0uz;
    for (auto list : lists)
    {
      auto count = list->ComponentCount();
      for (size_t i = 0uz; i < count; ++i)
      {
        new (NotNullTag {}, &selectors[componentIndex++]) Selector(list->_selectors[i]);
      }

      selectors[componentIndex - 1uz].SetIsLastInSelectorList(false);
    }

    assert(componentIndex == totalComponentCount);
    selectors[componentIndex - 1uz].SetIsLastInSelectorList(true);

    return SelectorList {krys::move(selectors)};
  }

  SelectorList SelectorList::CopySimple(const Selector &simpleSelector) noexcept
  {
    auto selectors = CreateUniqueArray<Selector>(1uz);

    new (NotNullTag {}, &selectors[0uz]) Selector(simpleSelector);
    selectors[0].SetIsFirstInComplexSelector(true);
    selectors[0].SetIsLastInComplexSelector(true);
    selectors[0].SetIsLastInSelectorList(true);

    return SelectorList {krys::move(selectors)};
  }

  SelectorList SelectorList::CopyComplex(const Selector &complexSelector) noexcept
  {
    size_t length = 0uz;
    for (auto *selector = &complexSelector; selector;
         selector = selector->PrecedingComplexSelectorComponent())
    {
      ++length;
    }

    auto selectors = CreateUniqueArray<Selector>(length);

    size_t i = 0uz;
    for (auto *selector = &complexSelector; selector;
         selector = selector->PrecedingComplexSelectorComponent(), ++i)
    {
      new (NotNullTag {}, &selectors[i]) Selector(*selector);
    }
    selectors[length - 1uz].SetIsLastInSelectorList(true);

    return SelectorList {krys::move(selectors)};
  }

#pragma endregion
}
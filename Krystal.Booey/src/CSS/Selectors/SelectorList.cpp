#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"

namespace krys::boo::css
{
#pragma region Constructors

  SelectorList::SelectorList(const SelectorList &other) noexcept
  {
    size_t otherComponentCount = other.ComponentCount();
    if (!otherComponentCount)
    {
      return;
    }

    _selectors = CreateUniqueArray<Selector>(otherComponentCount);
    for (size_t i = 0uz; i < otherComponentCount; ++i)
    {
      new (NotNullTag {}, &_selectors[i]) Selector(other._selectors[i]);
    }
  }

  SelectorList::SelectorList(MutableSelectorList &&selectors) noexcept
  {
    krys_debug_assert(!selectors.empty());

    size_t flattenedSize = 0uz;
    for (size_t i = 0uz; i < selectors.size(); ++i)
    {
      for (auto *selector = selectors[i].get(); selector;
           selector = selector->PrecedingComplexSelectorComponent())
      {
        ++flattenedSize;
      }
    }

    krys_debug_assert(flattenedSize);

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
          auto *currentSelector = current->ReleaseSelector().release();
          std::memcpy(static_cast<void *>(&_selectors[arrayIndex]), static_cast<void *>(currentSelector),
                      sizeof(Selector));

          // Free the underlying memory without invoking the destructor.
          operator delete(currentSelector);
        }

        if (current != last)
        {
          _selectors[arrayIndex].SetFlag(SelectorFlag::IsLastInComplexSelector, false);
        }

        current = current->PrecedingComplexSelectorComponent();
        krys_debug_assert(!_selectors[arrayIndex].IsLastInSelectorList()
                          || (flattenedSize == arrayIndex + 1uz));

        if (current)
        {
          _selectors[arrayIndex].SetFlag(SelectorFlag::IsFirstInComplexSelector, false);
        }

        ++arrayIndex;
      }

      krys_debug_assert(_selectors[arrayIndex - 1uz].IsFirstInComplexSelector());
    }

    krys_debug_assert(flattenedSize == arrayIndex);
    _selectors[arrayIndex - 1uz].SetFlag(SelectorFlag::IsLastInSelectorList, true);
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

    return static_cast<size_t>(current - _selectors.get()) + 1uz;
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

  namespace
  {
    template <typename TFunctor>
    KRYS_NODISCARD bool ForEachTagSelector(TFunctor &functor, const Selector *selector) noexcept
    {
      krys_debug_assert(selector);

      do
      {
        if (functor(selector))
        {
          return true;
        }
        if (const auto *selectorList = selector->SubSelectors())
        {
          for (const auto &subSelector : *selectorList)
          {
            if (ForEachTagSelector(functor, &subSelector))
            {
              return true;
            }
          }
        }
      } while ((selector = selector->PrecedingComplexSelectorComponent()));

      return false;
    }

    template <typename TFunctor>
    KRYS_NODISCARD bool ForEachSelector(TFunctor &functor, const SelectorList &selectorList) noexcept
    {
      for (const auto &selector : selectorList)
      {
        if (ForEachTagSelector(functor, &selector))
        {
          return true;
        }
      }

      return false;
    }
  }

  bool SelectorList::HasExplicitNestingParent() const noexcept
  {
    auto functor = [](const Selector *selector)
    {
      return selector->HasExplicitNestingParent();
    };

    return ForEachSelector(functor, *this);
  }

  bool SelectorList::HasOnlyNestingSelector() const noexcept
  {
    if (ComponentCount() != 1uz)
    {
      return false;
    }

    auto singleSelector = FirstSelectorComponent();
    if (!singleSelector)
    {
      return false;
    }

    // Selector should be a single selector
    if (singleSelector->PrecedingComplexSelectorComponent())
    {
      return false;
    }

    return singleSelector->Match() == SelectorMatch::NestingParent;
  }

  CSSOMString SelectorList::SelectorsText() const noexcept
  {
    CSSOMString result;
    BuildSelectorsText(result);
    return result;
  }

  void SelectorList::BuildSelectorsText(CSSOMString &builder) const noexcept
  {
    bool first = true;
    for (const auto &selector : *this)
    {
      if (!first)
      {
        builder += u8", ";
      }
      first = false;
      builder += selector.SelectorText();
    }
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

    selectors[aComponentCount - 1uz].SetFlag(SelectorFlag::IsLastInSelectorList, false);
    selectors[aComponentCount + bComponentCount - 1uz].SetFlag(SelectorFlag::IsLastInSelectorList, true);

    return SelectorList {krys::move(selectors)};
  }

  SelectorList SelectorList::Join(const List<const SelectorList *> &lists) noexcept
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

      selectors[componentIndex - 1uz].SetFlag(SelectorFlag::IsLastInSelectorList, false);
    }

    krys_debug_assert(componentIndex == totalComponentCount);
    selectors[componentIndex - 1uz].SetFlag(SelectorFlag::IsLastInSelectorList, true);

    return SelectorList {krys::move(selectors)};
  }

  SelectorList SelectorList::CopySimple(const Selector &simpleSelector) noexcept
  {
    auto selectors = CreateUniqueArray<Selector>(1uz);

    new (NotNullTag {}, &selectors[0uz]) Selector(simpleSelector);
    selectors[0uz].SetFlag(SelectorFlag::IsFirstInComplexSelector, true);
    selectors[0uz].SetFlag(SelectorFlag::IsLastInComplexSelector, true);
    selectors[0uz].SetFlag(SelectorFlag::IsLastInSelectorList, true);

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
    selectors[length - 1uz].SetFlag(SelectorFlag::IsLastInSelectorList, true);

    return SelectorList {krys::move(selectors)};
  }

#pragma endregion
}
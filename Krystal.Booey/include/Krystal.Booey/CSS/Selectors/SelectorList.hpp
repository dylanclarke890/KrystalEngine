#pragma once

#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/MutableSelector.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/UniqueArray.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::css
{
  using MutableSelectorList = SmallList<UniquePtr<MutableSelector>>;

  class SelectorList
  {
  private:
    /// @brief The selectors in this list. They are stored contiguously, with complex selectors being stored
    /// right-to-left (i.e. the rightmost selector is first in memory, followed by its preceding selector
    /// component, and so on).
    UniqueArray<Selector> _selectors;

  public:
    SelectorList() noexcept = default;

    SelectorList(const SelectorList &other) noexcept;

    SelectorList(SelectorList &&selectors) noexcept = default;

    explicit SelectorList(MutableSelectorList &&selectors) noexcept;

    explicit SelectorList(UniqueArray<Selector> &&array) noexcept;

    KRYS_NODISCARD bool operator==(const SelectorList &other) const noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _selectors == nullptr;
    }

    KRYS_NODISCARD const Selector *FirstSelectorComponent() const noexcept
    {
      return _selectors.get();
    }

    KRYS_NODISCARD const Selector *SelectorComponentAt(size_t index) const noexcept
    {
      return &_selectors[index];
    }

    KRYS_NODISCARD size_t ComponentCount() const noexcept;

    KRYS_NODISCARD size_t SelectorCount() const noexcept;

    KRYS_NODISCARD bool HasExplicitNestingParent() const noexcept;

    KRYS_NODISCARD bool HasOnlyNestingSelector() const noexcept;

    KRYS_NODISCARD CSSOMString SelectorText() const noexcept;

#pragma region Iterator

    struct const_iterator
    {
      friend class SelectorList;

      using value_type = Selector;
      using difference_type = std::ptrdiff_t;
      using pointer = const Selector *;
      using reference = const Selector &;
      using iterator_category = std::forward_iterator_tag;

    private:
      pointer _ptr = nullptr;

    public:
      const_iterator() noexcept = default;

      const_iterator(pointer ptr) noexcept : _ptr(ptr)
      {
      }

      KRYS_NODISCARD bool operator==(const const_iterator &) const noexcept = default;

      KRYS_NODISCARD reference operator*() const noexcept
      {
        return *_ptr;
      }

      KRYS_NODISCARD pointer operator->() const noexcept
      {
        return _ptr;
      }

      const_iterator &operator++() noexcept
      {
        // Skip subparts of compound selectors.
        while (!_ptr->IsFirstInComplexSelector())
        {
          ++_ptr;
        }

        _ptr = _ptr->IsLastInSelectorList() ? nullptr : _ptr + 1;
        return *this;
      }

      KRYS_NODISCARD const_iterator operator++(int) noexcept
      {
        const_iterator temp = *this;
        ++*this;
        return temp;
      }
    };

    KRYS_NODISCARD const_iterator begin() const noexcept
    {
      return {FirstSelectorComponent()};
    };

    KRYS_NODISCARD const_iterator end() const noexcept
    {
      return {};
    }

#pragma endregion

#pragma region Static utility functions

    KRYS_NODISCARD static SelectorList Join(const SelectorList &a, const SelectorList &b) noexcept;

    KRYS_NODISCARD static SelectorList Join(const SmallList<const SelectorList *> &lists) noexcept;

    KRYS_NODISCARD static SelectorList CopySimple(const Selector &selector) noexcept;

    KRYS_NODISCARD static SelectorList CopyComplex(const Selector &selector) noexcept;

#pragma endregion

  private:
    template <typename Functor>
    KRYS_NODISCARD bool ForEachTagSelector(Functor &&functor, const Selector *selector) const noexcept
    {
      assert(selector);

      do
      {
        if (functor(selector))
        {
          return true;
        }

        if (auto subSelectors = selector->SubSelectors())
        {
          for (const auto &subSelector : *subSelectors)
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

    template <typename Functor>
    KRYS_NODISCARD bool ForEachSelector(Functor &&functor) const noexcept
    {
      for (const auto &selector : *this)
      {
        if (ForEachTagSelector(functor, &selector))
        {
          return true;
        }
      }

      return false;
    }
  };
}
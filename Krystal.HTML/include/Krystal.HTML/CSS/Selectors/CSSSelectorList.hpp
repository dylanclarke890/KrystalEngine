#pragma once

#include "Krystal.HTML/CSS/Selectors/CSSSelector.hpp"
#include "Krystal.HTML/CSS/Selectors/MutableCSSSelector.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include "Krystal.Lib/Types/Func.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"
#include "Krystal.Lib/Types/UniqueArray.hpp"

namespace Krys::HTML
{
  using MutableCSSSelectorList = SmallList<UniquePtr<MutableCSSSelector>>;

  class CSSSelectorList
  {
  private:
    /// @brief The selectors in this list. They are stored contiguously, with complex selectors being stored
    /// right-to-left (i.e. the rightmost selector is first in memory, followed by its preceding selector
    /// component, and so on).
    UniqueArray<CSSSelector> _selectors;

  public:
    CSSSelectorList() noexcept = default;

    CSSSelectorList(const CSSSelectorList &other) noexcept;

    CSSSelectorList(CSSSelectorList &&selectors) noexcept = default;

    explicit CSSSelectorList(MutableCSSSelectorList &&selectors) noexcept;

    explicit CSSSelectorList(UniqueArray<CSSSelector> &&array) noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _selectors == nullptr;
    }

    KRYS_NODISCARD RawPtr<const CSSSelector> FirstSelectorComponent() const noexcept
    {
      return _selectors.get();
    }

    KRYS_NODISCARD RawPtr<const CSSSelector> SelectorComponentAt(size_t index) const noexcept
    {
      return &_selectors[index];
    }

    KRYS_NODISCARD size_t ComponentCount() const noexcept;

    KRYS_NODISCARD size_t SelectorCount() const noexcept;

    KRYS_NODISCARD bool HasExplicitNestingParent() const noexcept;

    KRYS_NODISCARD bool HasOnlyNestingSelector() const noexcept;

#pragma region Iterator

    struct const_iterator
    {
      friend class CSSSelectorList;

      using value_type = CSSSelector;
      using difference_type = std::ptrdiff_t;
      using pointer = const CSSSelector *;
      using reference = const CSSSelector &;
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

    KRYS_NODISCARD static CSSSelectorList Join(const CSSSelectorList &a, const CSSSelectorList &b) noexcept;

    KRYS_NODISCARD static CSSSelectorList
      Join(const SmallList<RawPtr<const CSSSelectorList>> &lists) noexcept;

    KRYS_NODISCARD static CSSSelectorList CopySimple(const CSSSelector &selector) noexcept;

    KRYS_NODISCARD static CSSSelectorList CopyComplex(const CSSSelector &selector) noexcept;

#pragma endregion

  private:
    template <typename Functor>
    bool ForEachTagSelector(Functor &functor, RawPtr<const CSSSelector> selector) const noexcept
    {
      assert(selector);

      do
      {
        if (functor(selector))
        {
          return true;
        }

        // if (const CSSSelectorList *selectors = selector->selectorList())
        // {
        //   for (const auto &subSelector : *selectorList)
        //   {
        //     if (forEachTagSelector(functor, &subSelector))
        //       return true;
        //   }
        // }

      } while ((selector = selector->PrecedingComplexSelectorComponent()));

      return false;
    }

    template <typename Functor>
    bool ForEachSelector(Functor &functor) const noexcept
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
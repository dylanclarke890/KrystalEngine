#pragma once

#include "Krystal.Booey/CSS/Selectors/MutableSelector.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniqueArray.hpp"

namespace krys::boo::css
{
  /// @brief A flattened, comma-separated list of complex selectors.
  class SelectorList
  {
  private:
    /// @brief The selectors in this list. They are stored contiguously, with complex selectors being stored
    /// right-to-left (i.e. the rightmost selector is first in memory, followed by its preceding selector
    /// component, and so on). The end of a complex selector is indicated by IsLastInComplexSelector on its
    /// last item, and the end of the array by IsLastInSelectorList on the last item.
    UniqueArray<Selector> _selectors;

  public:
    SelectorList() noexcept = default;

    SelectorList(const SelectorList &other) noexcept;

    SelectorList(SelectorList &&selectors) noexcept = default;

    explicit SelectorList(MutableSelectorList &&selectors) noexcept;

    explicit SelectorList(UniqueArray<Selector> &&array) noexcept : _selectors(krys::move(array))
    {
    }

    SelectorList &operator=(SelectorList &&) noexcept = default;

    KRYS_NODISCARD static SelectorList CopySimple(const Selector &simpleSelector) noexcept;
    KRYS_NODISCARD static SelectorList CopyComplex(const Selector &complexSelector) noexcept;
    KRYS_NODISCARD static SelectorList Join(const SelectorList &a, const SelectorList &b) noexcept;
    KRYS_NODISCARD static SelectorList Join(const List<const SelectorList *> &lists) noexcept;

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

    /// @return The index of the first component of the complex selector after the one at @p index, or
    /// nothing if it is the last.
    KRYS_NODISCARD Maybe<size_t> IndexOfNextSelectorAfter(size_t index) const noexcept
    {
      const_iterator current = SelectorComponentAt(index);
      ++current;
      if (current == end())
      {
        return {};
      }
      return static_cast<size_t>(&*current - _selectors.get());
    }

    KRYS_NODISCARD size_t ComponentCount() const noexcept;

    KRYS_NODISCARD size_t SelectorCount() const noexcept;

    KRYS_NODISCARD bool HasExplicitNestingParent() const noexcept;

    KRYS_NODISCARD bool HasOnlyNestingSelector() const noexcept;

    KRYS_NODISCARD CSSOMString SelectorsText() const noexcept;

    void BuildSelectorsText(CSSOMString &builder) const noexcept;

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
  };
}

namespace krys
{
  template <>
  struct DefaultHash<boo::css::SelectorList>
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = false;

    KRYS_NODISCARD static size_t Hash(const boo::css::SelectorList &list) noexcept
    {
      size_t seed = 0;
      for (const auto &selector : list)
      {
        seed = krys::Hash::Combine(seed, boo::css::HashComplexSelector(selector));
      }
      return seed;
    }

    KRYS_NODISCARD static bool Equal(const boo::css::SelectorList &a,
                                     const boo::css::SelectorList &b) noexcept
    {
      return a == b;
    }
  };
}

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

    explicit CSSSelectorList(UniqueArray<CSSSelector> &&array) noexcept : _selectors(Krys::Move(array))
    {
    }

    KRYS_NODISCARD static CSSSelectorList Join(const CSSSelectorList &a, const CSSSelectorList &b) noexcept;

    KRYS_NODISCARD static CSSSelectorList
      Join(const SmallList<RawPtr<const CSSSelectorList>> &lists) noexcept;

    KRYS_NODISCARD static CSSSelectorList CopySimpleSelector(const CSSSelector &selector) noexcept;

    KRYS_NODISCARD static CSSSelectorList CopyComplexSelector(const CSSSelector &selector) noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _selectors == nullptr;
    }

    KRYS_NODISCARD size_t ComponentCount() const noexcept;

    KRYS_NODISCARD size_t SelectorCount() const noexcept;

    KRYS_NODISCARD bool HasExplicitNestingParent() const noexcept;

    KRYS_NODISCARD bool HasOnlyNestingSelector() const noexcept;

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
      // for (const auto &selector : _selectors)
      // {
      //   if (ForEachTagSelector(functor, &selector))
      //   {
      //     return true;
      //   }
      // }

      return false;
    }
  };
}
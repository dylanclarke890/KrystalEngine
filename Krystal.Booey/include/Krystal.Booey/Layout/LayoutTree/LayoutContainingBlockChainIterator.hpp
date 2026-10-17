#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/FormattingContext.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::layout
{
  /// @brief Walks a box's containing blocks outwards.
  class LayoutContainingBlockChainIterator
  {
  private:
    const ElementBox *_current {nullptr};

  public:
    LayoutContainingBlockChainIterator() noexcept = default;
    explicit LayoutContainingBlockChainIterator(const ElementBox *current) noexcept : _current(current)
    {
    }

    KRYS_NODISCARD const ElementBox &operator*() const noexcept
    {
      return *_current;
    }
    KRYS_NODISCARD const ElementBox *operator->() const noexcept
    {
      return _current;
    }

    LayoutContainingBlockChainIterator &operator++() noexcept
    {
      krys_debug_assert(_current);
      _current =
        _current->IsInitialContainingBlock() ? nullptr : &FormattingContext::ContainingBlock(*_current);
      return *this;
    }

    KRYS_NODISCARD friend bool operator==(LayoutContainingBlockChainIterator,
                                          LayoutContainingBlockChainIterator) noexcept = default;
  };

  /// @brief A containing block and its containing blocks, up to (but not including) an optional ancestor.
  class LayoutContainingBlockChainIteratorAdapter
  {
  private:
    const ElementBox &_containingBlock;
    const ElementBox *_stayWithin {nullptr};

  public:
    explicit LayoutContainingBlockChainIteratorAdapter(const ElementBox &containingBlock,
                                                       const ElementBox *stayWithin = nullptr) noexcept
        : _containingBlock(containingBlock), _stayWithin(stayWithin)
    {
    }

    KRYS_NODISCARD LayoutContainingBlockChainIterator begin() const noexcept
    {
      return LayoutContainingBlockChainIterator(&_containingBlock);
    }
    KRYS_NODISCARD LayoutContainingBlockChainIterator end() const noexcept
    {
      return LayoutContainingBlockChainIterator(_stayWithin);
    }
  };

  KRYS_NODISCARD inline LayoutContainingBlockChainIteratorAdapter
    ContainingBlockChain(const Box &layoutBox) noexcept
  {
    return LayoutContainingBlockChainIteratorAdapter(FormattingContext::ContainingBlock(layoutBox));
  }

  KRYS_NODISCARD inline LayoutContainingBlockChainIteratorAdapter
    ContainingBlockChain(const Box &layoutBox, const ElementBox &stayWithin) noexcept
  {
    krys_debug_assert(layoutBox.IsDescendantOf(stayWithin));
    return LayoutContainingBlockChainIteratorAdapter(FormattingContext::ContainingBlock(layoutBox),
                                                     &stayWithin);
  }

  KRYS_NODISCARD inline LayoutContainingBlockChainIteratorAdapter
    ContainingBlockChainWithinFormattingContext(const Box &layoutBox, const ElementBox &root) noexcept
  {
    krys_debug_assert(root.EstablishesFormattingContext());
    return ContainingBlockChain(layoutBox, root);
  }
}

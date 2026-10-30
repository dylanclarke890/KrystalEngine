#pragma once

#include "Krystal.Booey/Layout/LayoutTree/LayoutDescendantIterator.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::layout
{
  /// @brief Walks the boxes of a formatting context: the root's descendants, without descending into the
  /// boxes that establish formatting contexts of their own.
  class FormattingContextBoxIterator : public LayoutDescendantIterator<Box>
  {
  public:
    explicit FormattingContextBoxIterator(const ElementBox &root) noexcept
        : LayoutDescendantIterator<Box>(root)
    {
    }

    enum FirstTag
    {
      First
    };

    FormattingContextBoxIterator(const ElementBox &root, FirstTag) noexcept
        : LayoutDescendantIterator<Box>(root, Traversal::FirstWithin<Box>(root))
    {
    }

    FormattingContextBoxIterator &operator++() noexcept
    {
      if (Get().EstablishesFormattingContext())
      {
        TraverseNextSkippingChildren();
      }
      else
      {
        TraverseNext();
      }
      return *this;
    }
  };

  /// @brief The boxes of a formatting context.
  class FormattingContextBoxIteratorAdapter
  {
  private:
    const ElementBox &_root;

  public:
    explicit FormattingContextBoxIteratorAdapter(const ElementBox &root) noexcept : _root(root)
    {
    }

    KRYS_NODISCARD FormattingContextBoxIterator begin() const noexcept
    {
      return {_root, FormattingContextBoxIterator::First};
    }
    KRYS_NODISCARD FormattingContextBoxIterator end() const noexcept
    {
      return FormattingContextBoxIterator(_root);
    }
  };

  KRYS_NODISCARD inline FormattingContextBoxIteratorAdapter
    FormattingContextBoxes(const ElementBox &root) noexcept
  {
    krys_debug_assert(root.EstablishesFormattingContext());
    return FormattingContextBoxIteratorAdapter(root);
  }
}

#pragma once

#include "Krystal.Lib/Ranges/Range.hpp"
#include "Krystal.Lib/Ranges/Reconstruct.hpp"
#include "Krystal.Lib/Ranges/UnreachableSentinel.hpp"
#include <iterator>
#include <utility>

namespace Krys::Ranges
{
  /// @brief A class whose iterator and sentinel denote an infinity-range that, if iterated with a
  /// traditional for range loop, will never cease.
  template <typename TIterator>
  class UnboundedView : public ViewBase
  {
  private:
    TIterator _iterator;

  public:
    /// @brief The iterator type.
    using iterator = TIterator;

    /// @brief The iterator type that can iterate indefinitely (or some approximation thereof).
    using const_iterator = iterator;

    /// @brief The sentinel type, an infinity type that compares equal to nothing.
    using sentinel = UnreachableSentinel;

    /// @brief The const sentinel type.
    /// @remarks It's just the sentinal type.
    using const_sentinel = sentinel;

    /// @brief The pointer type related to the iterator.
    using pointer = iterator_pointer_t<iterator>;

    /// @brief The const pointer type related to the iterator.
    /// @remarks It's just the pointer type.
    using const_pointer = pointer;

    /// @brief The reference type for this range.
    using reference = iterator_reference_t<iterator>;

    /// @brief The const reference type for this range.
    /// @remarks It's just the reference type.
    using const_reference = reference;

    /// @brief The value type for this range.
    using value_type = iterator_value_type_t<iterator>;

    /// @brief The difference type that results from iterator subtraction (not practical for this range).
    using difference_type = iterator_difference_type_t<iterator>;

    /// @brief The iterator concept - no matter what, this is a forward range at best.
    using iterator_concept = conditional_t<IsIteratorConceptOrBetter<std::forward_iterator_tag, iterator>,
                                           std::forward_iterator_tag, iterator_concept_t<iterator>>;

    /// @brief Constructs a default-constructed iterator and an infinity sentinel as the range.
    /// @remarks Not very useful for anything other than generic programming shenanigans.
    constexpr UnboundedView() = default;

    /// @brief Constructs an UnboundedView using the specified iterator value iterator and an infinity
    /// sentinel.
    constexpr UnboundedView(iterator it) noexcept(NoThrowMoveConstructible<iterator>)
        : _iterator(std::move(it))
    {
    }

    /// @brief The iterator the UnboundedView was constructed with.
    /// @remarks This function copies the contained iterator.
    constexpr iterator begin() & noexcept
    {
      if constexpr (CopyConstructible<iterator>)
      {
        return this->_iterator;
      }
      else
      {
        return std::move(this->_iterator);
      }
    }

    /// @brief The iterator the UnboundedView was constructed with.
    /// @remarks This function copies the contained iterator.
    constexpr iterator begin() const & noexcept
    {
      return this->_iterator;
    }

    /// @brief The iterator the UnboundedView was constructed with.
    /// @remarks This function moves the contained iterator out.
    constexpr iterator begin() && noexcept
    {
      return std::move(this->_iterator);
    }

    /// @brief The ending sentinel.
    /// @remarks The sentinel is an infinity sentinel that never compares equal to any other thing: in
    /// short, any range composed of [iterator, unreachable_sentinel) will never cease.
    constexpr sentinel end() const noexcept
    {
      return sentinel {};
    }

    /// @brief The reconstruct extension point for re-creating this type from its iterator and sentinel.
    constexpr friend UnboundedView reconstruct(std::in_place_type_t<UnboundedView>, iterator iterator,
                                               sentinel) noexcept(NoThrowMoveConstructible<iterator>)
    {
      return UnboundedView<TIterator>(std::move(iterator));
    }

    /// @brief Checks whether this UnboundedView is empty.
    /// @remarks This can prevent needing to call `begin()` which may be beneficial for move-only iterators.
    /// This is always false for a UnboundedView.
    constexpr bool empty() const noexcept
    {
      return false;
    }

    /// @brief Produces a copy of the UnboundedView and advances the `begin()` iterator by 1.
    /// @remarks This function call only works if the underlying iterator and sentinal types are copyable.
    KRYS_NODISCARD constexpr UnboundedView next() const & noexcept(
      NoThrowCopyConstructible<iterator> && noexcept(Krys::Ranges::iter_advance(std::declval<iterator &>())))
    {
      iterator it = this->_iterator;
      Krys::Ranges::iter_advance(it);
      return UnboundedView(std::move(it));
    }

    /// @brief Produces a copy of the UnboundedView and advances the `begin()` iterator by 1.
    /// @remarks This function call can be more efficient and allows working with move-only iterators. This
    /// function call will move the iterators underlying this object.
    KRYS_NODISCARD constexpr UnboundedView next() && noexcept(
      NoThrowMoveConstructible<iterator> && noexcept(Krys::Ranges::iter_advance(std::declval<iterator &>())))
    {
      iterator it = std::move(this->_iterator);
      Krys::Ranges::iter_advance(it);
      return UnboundedView(std::move(it));
    }

    /// @brief Produces a copy of the UnboundedView and advances the `begin()` iterator by `diff`.
    /// @param[in] diff The amount to move this iterator by. Can be positive or negative.
    /// @remarks This function call only works if the underlying iterator and sentinal types are copyable.
    KRYS_NODISCARD constexpr UnboundedView next(difference_type diff) const & noexcept(
      NoThrowCopyConstructible<iterator>
      && noexcept(Krys::Ranges::iter_advance(std::declval<iterator &>(), diff)))
    {
      auto it = this->_iterator;
      Krys::Ranges::iter_advance(it, diff);
      return UnboundedView(std::move(it));
    }

    /// @param[in] diff The amount to move this iterator by. Can be positive or negative.
    /// @remarks This function call can be more efficient and allows working with move-only iterators. This
    /// function call will move the iterators underlying this object.
    KRYS_NODISCARD constexpr UnboundedView next(difference_type diff) && noexcept(
      NoThrowMoveConstructible<iterator>
      && noexcept(Krys::Ranges::iter_advance(std::declval<iterator &>(), diff)))
    {
      iterator it = std::move(this->_iterator);
      Krys::Ranges::iter_advance(it, diff);
      return UnboundedView(std::move(it));
    }

    /// @brief Produces a copy of the UnboundedView and recedes the `begin()` iterator by `diff`.
    /// @param[in] diff The amount to move this iterator by. Can be positive or negative.
    /// @remarks This function call requires that the underlying iterator are bidirectional.
    KRYS_NODISCARD constexpr UnboundedView prev(difference_type diff = 1) const
      noexcept(NoThrowCopyConstructible<iterator>
               && noexcept(Krys::Ranges::iter_recede(std::declval<iterator &>(), diff)))
    {
      auto it = this->_iterator;
      Krys::Ranges::iter_recede(it, diff);
      return UnboundedView(std::move(it));
    }

    /// @brief Advances the `begin()` iterator of this ztd::ranges::UnboundedView by `diff` or just `1`
    /// if the argument is not specified.
    /// @param[in] diff The amount to move this iterator by. Can be positive or negative.
    constexpr UnboundedView &advance(difference_type diff = 1) noexcept(
      noexcept(Krys::Ranges::iter_advance(std::declval<iterator &>(), diff)))
    {
      Krys::Ranges::iter_advance(this->_iterator, diff);
      return *this;
    }

    /// @brief Recedes the `begin()` iterator of this ztd::ranges::UnboundedView by `diff` or just `1` if
    /// the argument is not specified.
    /// @param[in] diff The amount to move this iterator by. Can be positive or negative.
    /// @remarks This function call requires that the underlying iterator are bidirectional.
    constexpr UnboundedView &recede(difference_type diff = 1) noexcept(
      noexcept(Krys::Ranges::iter_recede(std::declval<iterator &>(), std::declval<difference_type>())))
    {
      Krys::Ranges::iter_recede(this->_iterator, diff);
      return *this;
    }
  };
}

#if KRYS_CONFIG(STD_LIBRARY_BORROWED_RANGE)
namespace std::ranges
{
  template <typename TIterator>
  inline constexpr bool enable_borrowed_range<Krys::Ranges::UnboundedView<TIterator>> = true;
}
#else
namespace Krys::Ranges
{
  /// @brief Mark subranges as appropriately borrowed ranges.
  template <typename TIterator>
  inline constexpr bool enable_borrowed_range<UnboundedView<TIterator>> = true;
}
#endif

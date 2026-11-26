#pragma once

#include "Krystal.Booey/Css/Properties/PropertyId.hpp"
#include "Krystal.Booey/Css/Properties/PropertyShorthandFunctions.hpp"
#include "Krystal.Core/Containers/SmallList.hpp"
#include "Krystal.Core/Containers/Span.hpp"

namespace krys::boo::css
{
  /// @brief A shorthand property and the longhands it sets, such as `margin` and its four sides.
  /// @see https://drafts.csswg.org/css-cascade-5/#shorthand-property
  class PropertyShorthand
  {
  private:
    PropertyId _id {PropertyId::Invalid};
    Span<const PropertyId> _longhands;

  public:
    /// @brief No shorthand, with no longhands: what `ShorthandForProperty` gives a longhand.
    PropertyShorthand() noexcept = default;

    /// @param longhands A fixed-size array that outlives the shorthand, usually a static one.
    template <size_t Count>
    requires(Count != std::dynamic_extent)
    PropertyShorthand(PropertyId id, Span<const PropertyId, Count> longhands) noexcept
        : _id(id), _longhands(longhands)
    {
    }

    KRYS_NODISCARD const PropertyId *begin() const noexcept
    {
      return _longhands.data();
    }

    KRYS_NODISCARD const PropertyId *end() const noexcept
    {
      return _longhands.data() + _longhands.size();
    }

    /// @brief The number of longhands.
    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _longhands.size();
    }

    KRYS_NODISCARD PropertyId Id() const noexcept
    {
      return _id;
    }

    /// @brief The longhands, in the order the shorthand's definition lists them, which serialisation follows.
    KRYS_NODISCARD Span<const PropertyId> Properties() const noexcept
    {
      return _longhands;
    }
  };

  /// @brief `transition`'s longhands in the order its parser tries them.
  /// @note `<single-transition-property>` takes any `<custom-ident>`, so it's tried last, or a keyword such
  /// as `ease` would be read as a property name; the first `<time>` is the duration, so the delay follows it.
  /// @see https://drafts.csswg.org/css-transitions-2/#transition-shorthand-property
  KRYS_NODISCARD PropertyShorthand TransitionShorthandForParsing() noexcept;

  /// @brief The shorthand @p id, or an empty one if @p id is a longhand. Generated.
  KRYS_NODISCARD PropertyShorthand ShorthandForProperty(PropertyId id) noexcept;

  using PropertyShorthandList = SmallList<PropertyShorthand, MaxShorthandsForLonghand>;

  /// @brief The shorthands that set longhand @p id, starting with `all` if it's one. Generated.
  KRYS_NODISCARD PropertyShorthandList MatchingShorthandsForLonghand(PropertyId id) noexcept;

  /// @brief Where @p shorthandId is in @p shorthands, which must contain it.
  /// @return The value for `PropertyMetadata::ShorthandIndex`.
  KRYS_NODISCARD uint8 IndexOfShorthandForLonghand(PropertyId shorthandId,
                                                   const PropertyShorthandList &shorthands) noexcept;
}

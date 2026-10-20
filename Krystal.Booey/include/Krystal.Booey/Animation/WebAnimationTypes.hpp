#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include <bitset>
#include <functional>

namespace krys::boo
{
  /// @brief A property that can be animated: a property id, or a custom property's name.
  using AnimatableCSSProperty = Variant<css::PropertyId, css::CSSOMStringAtom>;

  enum class AnimationImpact : uint8
  {
    RequiresRecomposite,
    ForcesStackingContext
  };

  enum class UseAcceleratedAction : bool
  {
    No,
    Yes
  };

  enum class UseCachedCurrentTime : bool
  {
    No,
    Yes
  };

  enum class EndpointInclusiveActiveInterval : bool
  {
    No,
    Yes
  };

  enum class WebAnimationType : uint8
  {
    CSSAnimation,
    CSSTransition,
    WebAnimation
  };

  /// @brief A property an accelerated effect animates.
  enum class AcceleratedEffectProperty : uint16
  {
    Invalid = 1 << 0,
    Opacity = 1 << 1,
    Transform = 1 << 2,
    Translate = 1 << 3,
    Rotate = 1 << 4,
    Scale = 1 << 5,
    OffsetPath = 1 << 6,
    OffsetDistance = 1 << 7,
    OffsetPosition = 1 << 8,
    OffsetAnchor = 1 << 9,
    OffsetRotate = 1 << 10,
    Filter = 1 << 11,
    BackdropFilter = 1 << 12
  };

  /// @brief Hashes an animatable property, for maps keyed by it.
  struct AnimatableCSSPropertyHash
  {
    KRYS_NODISCARD size_t operator()(const AnimatableCSSProperty &property) const noexcept
    {
      if (const auto *id = std::get_if<css::PropertyId>(&property))
      {
        return DefaultHash<css::PropertyId>::Hash(*id);
      }
      return std::hash<css::CSSOMStringAtom> {}(std::get<css::CSSOMStringAtom>(property));
    }
  };

  /// @brief A set of animatable properties.
  using AnimatableCSSPropertySet = HashSet<AnimatableCSSProperty, AnimatableCSSPropertyHash>;

  /// @brief A set of property ids.
  struct CSSPropertiesBitSet
  {
    std::bitset<css::TotalPropertyIdValues> Properties {};

    void Set(css::PropertyId property) noexcept
    {
      Properties.set(static_cast<size_t>(property));
    }

    KRYS_NODISCARD bool Get(css::PropertyId property) const noexcept
    {
      return Properties.test(static_cast<size_t>(property));
    }

    void Merge(Span<const css::PropertyId> properties) noexcept
    {
      for (auto property : properties)
      {
        Set(property);
      }
    }
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::AnimationImpact, 2uz);

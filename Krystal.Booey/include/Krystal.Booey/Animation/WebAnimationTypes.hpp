/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

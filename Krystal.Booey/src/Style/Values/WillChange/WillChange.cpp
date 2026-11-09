/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Style/Values/WillChange/WillChange.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include <algorithm>

namespace krys::boo::style
{
  // MARK: WillChangeAnimatableFeature

  WillChangeAnimatableFeature::WillChangeAnimatableFeature(Feature willChange,
                                                           css::PropertyId willChangeProperty) noexcept
  {
    switch (willChange)
    {
      case Feature::Property:
      {
        krys_debug_assert(willChangeProperty != css::PropertyId::Invalid);
        _cssPropertyId = static_cast<uint16>(willChangeProperty);
        KRYS_FALLTHROUGH;
      }
      case Feature::ScrollPosition:
      case Feature::Contents:
      {
        _feature = willChange;
        break;
      }
    }
  }

  // "If any non-initial value of a property would create a stacking context on the element,
  // specifying that property in will-change must create a stacking context on the element."
  bool WillChangeAnimatableFeature::PropertyCreatesStackingContext(css::PropertyId property) noexcept
  {
    switch (property)
    {
      case css::PropertyId::Perspective:
      case css::PropertyId::Scale:
      case css::PropertyId::Rotate:
      case css::PropertyId::Translate:
      case css::PropertyId::Transform:
      case css::PropertyId::TransformStyle:
      case css::PropertyId::OffsetPath:
      case css::PropertyId::ClipPath:
      case css::PropertyId::Mask:
      case css::PropertyId::Opacity:
      case css::PropertyId::Position:
      case css::PropertyId::ZIndex:
      case css::PropertyId::MixBlendMode:
      case css::PropertyId::Isolation:
      case css::PropertyId::Filter:
      case css::PropertyId::BackdropFilter:
      case css::PropertyId::MaskImage:
      case css::PropertyId::MaskBorder:
      case css::PropertyId::Contain:        return true;
      default:                              return false;
    }
  }

  bool WillChangeAnimatableFeature::PropertyTriggersCompositing(css::PropertyId property) noexcept
  {
    switch (property)
    {
      case css::PropertyId::Opacity:
      case css::PropertyId::Filter:
      case css::PropertyId::BackdropFilter: return true;
      default:                              return false;
    }
  }

  bool WillChangeAnimatableFeature::PropertyTriggersCompositingOnBoxesOnly(css::PropertyId property) noexcept
  {
    // Don't trigger for perspective and transform-style, because those
    // only do compositing if they have a 3d-transformed descendant and
    // we don't want to do compositing all the time.
    switch (property)
    {
      case css::PropertyId::Scale:
      case css::PropertyId::Rotate:
      case css::PropertyId::Translate:
      case css::PropertyId::Transform:
      case css::PropertyId::OffsetPath: return true;
      default:                          return false;
    }
  }

  // MARK: WillChangeAnimatableFeatures::Data

  bool WillChangeAnimatableFeatures::Data::operator==(
    const WillChangeAnimatableFeatures::Data &other) const noexcept
  {
    return _animatableFeatures == other._animatableFeatures;
  }

  bool WillChangeAnimatableFeatures::Data::ContainsScrollPosition() const noexcept
  {
    return std::ranges::any_of(_animatableFeatures, [](const auto &feature)
                               { return feature.GetFeature() == Feature::ScrollPosition; });
  }

  bool WillChangeAnimatableFeatures::Data::ContainsContents() const noexcept
  {
    return std::ranges::any_of(_animatableFeatures,
                               [](const auto &feature) { return feature.GetFeature() == Feature::Contents; });
  }

  bool WillChangeAnimatableFeatures::Data::ContainsProperty(css::PropertyId property) const noexcept
  {
    return std::ranges::any_of(_animatableFeatures,
                               [&](const auto &feature) { return feature.Property() == property; });
  }

  bool WillChangeAnimatableFeatures::Data::CreatesContainingBlockForAbsolutelyPositioned(
    bool isRootElement) const noexcept
  {
    return CreatesContainingBlockForOutOfFlowPositioned(isRootElement)
           || ContainsProperty(css::PropertyId::Position);
  }

  bool WillChangeAnimatableFeatures::Data::CreatesContainingBlockForOutOfFlowPositioned(
    bool isRootElement) const noexcept
  {
    return ContainsProperty(css::PropertyId::Perspective)
           // CSS transforms
           || ContainsProperty(css::PropertyId::Transform)
           || ContainsProperty(css::PropertyId::TransformStyle)
           || ContainsProperty(css::PropertyId::Translate) || ContainsProperty(css::PropertyId::Rotate)
           || ContainsProperty(css::PropertyId::Scale)
           || ContainsProperty(css::PropertyId::OffsetPath)
           // CSS containment
           || ContainsProperty(css::PropertyId::Contain)
           // CSS filter & backdrop-filter
           || (ContainsProperty(css::PropertyId::BackdropFilter) && !isRootElement)
           || (ContainsProperty(css::PropertyId::Filter) && !isRootElement);
  }

  bool WillChangeAnimatableFeatures::Data::CanBeBackdropRoot() const noexcept
  {
    return ContainsProperty(css::PropertyId::Opacity) || ContainsProperty(css::PropertyId::BackdropFilter)
           || ContainsProperty(css::PropertyId::ClipPath) || ContainsProperty(css::PropertyId::Filter)
           || ContainsProperty(css::PropertyId::MixBlendMode) || ContainsProperty(css::PropertyId::Mask);
  }

  void WillChangeAnimatableFeatures::Data::AddFeature(Feature feature, css::PropertyId propertyId) noexcept
  {
    krys_debug_assert(feature == Feature::Property || propertyId == css::PropertyId::Invalid);
    _animatableFeatures.push_back(WillChangeAnimatableFeature(feature, propertyId));

    _canCreateStackingContext |= WillChangeAnimatableFeature::PropertyCreatesStackingContext(propertyId);
    _canTriggerCompositingOnInline |= WillChangeAnimatableFeature::PropertyTriggersCompositing(propertyId);
    _canTriggerCompositing |=
      _canTriggerCompositingOnInline
      | WillChangeAnimatableFeature::PropertyTriggersCompositingOnBoxesOnly(propertyId);
  }

  // MARK: - Conversion

  WillChange CSSValueConversion<WillChange>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    // FIXME: This should also be storing <custom-ident> values that aren't valid css::PropertyIds for
    // computed value serialization.

    // The document's property settings stand in for its settings.
    const auto &settings = state.GetDocument().CssParserContext().PropertySettings;

    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Auto: return css::keywords::Auto {};
        case css::ValueId::ScrollPosition:
          return WillChangeAnimatableFeatures {WillChangeAnimatableFeature::Feature::ScrollPosition};
        case css::ValueId::Contents:
          return WillChangeAnimatableFeatures {WillChangeAnimatableFeature::Feature::Contents};
        default:
        {
          if (primitiveValue->IsPropertyId())
          {
            if (auto propertyId = primitiveValue->PropertyId(); css::IsExposed(propertyId, &settings))
            {
              return WillChangeAnimatableFeatures {WillChangeAnimatableFeature::Feature::Property,
                                                   propertyId};
            }
          }

          return css::keywords::Auto {};
        }
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue, 1>(state, value);
    if (!list)
    {
      return css::keywords::Auto {};
    }

    auto result = WillChangeAnimatableFeatures {};

    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::ScrollPosition:
          result.AddFeature(WillChangeAnimatableFeature::Feature::ScrollPosition);
          break;
        case css::ValueId::Contents: result.AddFeature(WillChangeAnimatableFeature::Feature::Contents); break;
        default:
        {
          if (item.IsPropertyId())
          {
            if (auto propertyId = item.PropertyId(); css::IsExposed(propertyId, &settings))
            {
              result.AddFeature(WillChangeAnimatableFeature::Feature::Property, propertyId);
              break;
            }
          }
          break;
        }
      }
    }

    return result;
  }
}

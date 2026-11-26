/*
 * Copyright (C) 2011 Andreas Kling (kling@webkit.org)
 * Copyright (C) 2013 Adobe Systems Incorporated. All rights reserved.
 * Copyright (C) 2021-2023 Apple Inc. All rights reserved.
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
 *
 */

#include "Krystal.Booey/Css/Values/Value.hpp"
#include "Krystal.Booey/Css/Properties/Property.hpp"
#include "Krystal.Booey/Css/Values/AttrValue.hpp"
#include "Krystal.Booey/Css/Values/BackgroundRepeatValue.hpp"
#include "Krystal.Booey/Css/Values/BasicShapeValue.hpp"
#include "Krystal.Booey/Css/Values/BorderImageSliceValue.hpp"
#include "Krystal.Booey/Css/Values/BorderImageWidthValue.hpp"
#include "Krystal.Booey/Css/Values/BoxShadowPropertyValue.hpp"
#include "Krystal.Booey/Css/Values/ColorSchemeValue.hpp"
#include "Krystal.Booey/Css/Values/ColorValue.hpp"
#include "Krystal.Booey/Css/Values/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/Css/Values/CounterValue.hpp"
#include "Krystal.Booey/Css/Values/CrossFadeValue.hpp"
#include "Krystal.Booey/Css/Values/CursorImageValue.hpp"
#include "Krystal.Booey/Css/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/Css/Values/DynamicRangeLimitValue.hpp"
#include "Krystal.Booey/Css/Values/EasingFunctionValue.hpp"
#include "Krystal.Booey/Css/Values/FilterImageValue.hpp"
#include "Krystal.Booey/Css/Values/FilterValue.hpp"
#include "Krystal.Booey/Css/Values/FontFaceSrcValue.hpp"
#include "Krystal.Booey/Css/Values/FontFeatureValue.hpp"
#include "Krystal.Booey/Css/Values/FontStyleRangeValue.hpp"
#include "Krystal.Booey/Css/Values/FontStyleWithAngleValue.hpp"
#include "Krystal.Booey/Css/Values/FontVariationValue.hpp"
#include "Krystal.Booey/Css/Values/FunctionValue.hpp"
#include "Krystal.Booey/Css/Values/GradientValue.hpp"
#include "Krystal.Booey/Css/Values/GridAutoRepeatValue.hpp"
#include "Krystal.Booey/Css/Values/GridIntegerRepeatValue.hpp"
#include "Krystal.Booey/Css/Values/GridLineNamesValue.hpp"
#include "Krystal.Booey/Css/Values/GridLineValue.hpp"
#include "Krystal.Booey/Css/Values/GridTemplateAreasValue.hpp"
#include "Krystal.Booey/Css/Values/ImageSetOptionValue.hpp"
#include "Krystal.Booey/Css/Values/ImageSetValue.hpp"
#include "Krystal.Booey/Css/Values/ImageValue.hpp"
#include "Krystal.Booey/Css/Values/OffsetRotateValue.hpp"
#include "Krystal.Booey/Css/Values/PaintImageValue.hpp"
#include "Krystal.Booey/Css/Values/PathValue.hpp"
#include "Krystal.Booey/Css/Values/PendingSubstitutionValue.hpp"
#include "Krystal.Booey/Css/Values/PositionValue.hpp"
#include "Krystal.Booey/Css/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Css/Values/QuadValue.hpp"
#include "Krystal.Booey/Css/Values/RatioValue.hpp"
#include "Krystal.Booey/Css/Values/RayValue.hpp"
#include "Krystal.Booey/Css/Values/RectValue.hpp"
#include "Krystal.Booey/Css/Values/ReflectValue.hpp"
#include "Krystal.Booey/Css/Values/ScrollValue.hpp"
#include "Krystal.Booey/Css/Values/SerialisationContext.hpp"
#include "Krystal.Booey/Css/Values/SubgridValue.hpp"
#include "Krystal.Booey/Css/Values/TextShadowPropertyValue.hpp"
#include "Krystal.Booey/Css/Values/ToLengthConversionData.hpp"
#include "Krystal.Booey/Css/Values/TransformListValue.hpp"
#include "Krystal.Booey/Css/Values/Types.hpp"
#include "Krystal.Booey/Css/Values/UnicodeRangeValue.hpp"
#include "Krystal.Booey/Css/Values/UrlValue.hpp"
#include "Krystal.Booey/Css/Values/ValueList.hpp"
#include "Krystal.Booey/Css/Values/ValuePair.hpp"
#include "Krystal.Booey/Css/Values/VariableReferenceValue.hpp"
#include "Krystal.Booey/Css/Values/ViewValue.hpp"
#include "Krystal.Booey/Dom/Events/EventTarget.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Hashing/FastHash.hpp"

namespace krys::boo::css
{
  // The reference count, the packed header and the static flag, with nothing else.
  static_assert(sizeof(Value) == 8uz);

  template <typename Visitor>
  constexpr decltype(auto) Value::VisitDerived(Visitor &&visitor) noexcept
  {
    switch (_header.Type)
    {
      case ValueType::Attr:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<AttrValue>(*this));
      }
      case ValueType::BackgroundRepeat:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<BackgroundRepeatValue>(*this));
      }
      case ValueType::BasicShape:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<BasicShapeValue>(*this));
      }
      case ValueType::BorderImageSlice:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<BorderImageSliceValue>(*this));
      }
      case ValueType::BorderImageWidth:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<BorderImageWidthValue>(*this));
      }
      case ValueType::BoxShadowProperty:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<BoxShadowPropertyValue>(*this));
      }
      case ValueType::Color:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<ColorValue>(*this));
      }
      case ValueType::ColorScheme:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<ColorSchemeValue>(*this));
      }
      case ValueType::Counter:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<CounterValue>(*this));
      }
      case ValueType::CrossFade:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<CrossFadeValue>(*this));
      }
      case ValueType::CursorImage:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<CursorImageValue>(*this));
      }
      case ValueType::CustomProperty:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<CustomPropertyValue>(*this));
      }
      case ValueType::DynamicRangeLimit:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<DynamicRangeLimitValue>(*this));
      }
      case ValueType::EasingFunction:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<EasingFunctionValue>(*this));
      }
      case ValueType::FilterImage:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<FilterImageValue>(*this));
      }
      case ValueType::Filter:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<FilterValue>(*this));
      }
      case ValueType::FontFaceSrcLocal:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<FontFaceSrcLocalValue>(*this));
      }
      case ValueType::FontFaceSrcResource:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<FontFaceSrcResourceValue>(*this));
      }
      case ValueType::FontFeature:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<FontFeatureValue>(*this));
      }
      case ValueType::FontStyleWithAngle:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<FontStyleWithAngleValue>(*this));
      }
      case ValueType::FontStyleRange:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<FontStyleRangeValue>(*this));
      }
      case ValueType::FontVariation:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<FontVariationValue>(*this));
      }
      case ValueType::Function:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<FunctionValue>(*this));
      }
      case ValueType::Gradient:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<GradientValue>(*this));
      }
      case ValueType::GridAutoRepeat:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<GridAutoRepeatValue>(*this));
      }
      case ValueType::GridIntegerRepeat:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<GridIntegerRepeatValue>(*this));
      }
      case ValueType::GridLineNames:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<GridLineNamesValue>(*this));
      }
      case ValueType::GridLine:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<GridLineValue>(*this));
      }
      case ValueType::GridTemplateAreas:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<GridTemplateAreasValue>(*this));
      }
      case ValueType::Image:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<ImageValue>(*this));
      }
      case ValueType::ImageSetOption:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<ImageSetOptionValue>(*this));
      }
      case ValueType::ImageSet:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<ImageSetValue>(*this));
      }
      case ValueType::OffsetRotate:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<OffsetRotateValue>(*this));
      }
      case ValueType::Path:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<PathValue>(*this));
      }
      case ValueType::PendingSubstitution:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<PendingSubstitutionValue>(*this));
      }
      case ValueType::Position:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<PositionValue>(*this));
      }
      case ValueType::PositionX:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<PositionXValue>(*this));
      }
      case ValueType::PositionY:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<PositionYValue>(*this));
      }
      case ValueType::Primitive:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<PrimitiveValue>(*this));
      }
      case ValueType::Quad:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<QuadValue>(*this));
      }
      case ValueType::Ratio:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<RatioValue>(*this));
      }
      case ValueType::Ray:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<RayValue>(*this));
      }
      case ValueType::Rect:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<RectValue>(*this));
      }
      case ValueType::Reflect:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<ReflectValue>(*this));
      }
      case ValueType::Scroll:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<ScrollValue>(*this));
      }
      case ValueType::Subgrid:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<SubgridValue>(*this));
      }
      case ValueType::TextShadowProperty:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<TextShadowPropertyValue>(*this));
      }
      case ValueType::TransformList:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<TransformListValue>(*this));
      }
      case ValueType::Url:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<UrlValue>(*this));
      }
      case ValueType::UnicodeRange:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<UnicodeRangeValue>(*this));
      }
      case ValueType::ValueList:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<ValueList>(*this));
      }
      case ValueType::ValuePair:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<ValuePair>(*this));
      }
      case ValueType::VariableReference:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<VariableReferenceValue>(*this));
      }
      case ValueType::View:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<ViewValue>(*this));
      }
      case ValueType::PaintImage:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<PaintImageValue>(*this));
      }
    }

    krys_unreachable();
  }

  template <typename Visitor>
  constexpr decltype(auto) Value::VisitDerived(Visitor &&visitor) const noexcept
  {
    return const_cast<Value &>(*this).VisitDerived(
      [&](auto &value) { return std::invoke(std::forward<Visitor>(visitor), std::as_const(value)); });
  }

  IterationStatus Value::VisitChildren(const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitDerived(
      [&](auto &value)
      {
        if constexpr (requires { value.CustomVisitChildren(func); })
        {
          return value.CustomVisitChildren(func);
        }
        else
        {
          return IterationStatus::Continue;
        }
      });
  }

  bool Value::MayDependOnBaseUrl() const noexcept
  {
    return VisitDerived(
      [&](auto &value)
      {
        if constexpr (requires { value.CustomMayDependOnBaseUrl(); })
        {
          return value.CustomMayDependOnBaseUrl();
        }
        else
        {
          return false;
        }
      });
  }

  bool Value::TraverseSubresources(const Func<bool(const CachedResource &)> &handler) const noexcept
  {
    return VisitDerived(
      [&](auto &value)
      {
        if constexpr (requires { value.CustomTraverseSubresources(handler); })
        {
          return value.CustomTraverseSubresources(handler);
        }
        else
        {
          return false;
        }
      });
  }

  css::ComputedStyleDependencies Value::ComputedStyleDependencies() const noexcept
  {
    css::ComputedStyleDependencies dependencies;
    CollectComputedStyleDependencies(dependencies);
    return dependencies;
  }

  void Value::CollectComputedStyleDependencies(css::ComputedStyleDependencies &dependencies) const noexcept
  {
    // FIXME: Unclear why it's OK that we do not cover CSSValuePair, CSSQuadValue, CSSRectValue,
    // CSSBorderImageSliceValue, CSSBorderImageWidthValue, and others here. Probably should use visitDerived
    // unless they don't allow the primitive values that can have dependencies. May want to base this on a
    // traverseValues or forEachValue function instead.
    // FIXME: Consider a non-recursive algorithm for walking this tree of dependencies.
    if (auto *asList = DynamicDowncast<ValueContainingList>(*this))
    {
      for (auto &listValue : *asList)
      {
        listValue.CollectComputedStyleDependencies(dependencies);
      }

      return;
    }

    if (auto *asPrimitiveValue = DynamicDowncast<PrimitiveValue>(*this))
    {
      asPrimitiveValue->CollectComputedStyleDependencies(dependencies);
    }
  }

  bool Value::CanResolveDependenciesWith(const ToLengthConversionData &conversionData) const noexcept
  {
    return ComputedStyleDependencies().CanResolveDependenciesWithConversionData(conversionData);
  }

  bool Value::Equals(const Value &other) const noexcept
  {
    if (_header.Type == other._header.Type)
    {
      return VisitDerived(
        [&]<typename TValue>(TValue &typedThis)
        {
          static_assert(!SameType<decltype(&TValue::Equals), decltype(&Value::Equals)>);
          return typedThis.Equals(Downcast<TValue>(other));
        });
    }

    if (auto *thisList = DynamicDowncast<ValueList>(*this))
    {
      return thisList->ContainsSingleEqualItem(other);
    }

    if (auto *otherList = DynamicDowncast<ValueList>(other))
    {
      return otherList->ContainsSingleEqualItem(*this);
    }

    return false;
  }

  bool Value::AddHash(size_t &seed) const noexcept
  {
    // To match Equals() a single item list could have the same hash as the item.
    // FIXME: Some Style::Builder functions can only handle list values.

    seed = CombineFastHashes(seed, _header.Type);

    // A class hashes its contents with an `AddDerivedHash`; without one, its values aren't hashed.
    return VisitDerived(
      [&](auto &value)
      {
        if constexpr (requires { value.AddDerivedHash(seed); })
        {
          return value.AddDerivedHash(seed);
        }
        else
        {
          return false;
        }
      });
  }

  CSSOMString Value::CssText(const SerialisationContext &context) const noexcept
  {
    return VisitDerived([&](auto &value) { return value.CustomCSSText(context); });
  }

  void Value::operator delete(Value *value, std::destroying_delete_t) noexcept
  {
    value->VisitDerived(
      []<typename TValue>(TValue &value)
      {
        std::destroy_at(&value);
        ::operator delete(&value);
      });
  }
}

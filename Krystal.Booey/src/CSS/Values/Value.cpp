#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/AppleColorFilterValue.hpp"
#include "Krystal.Booey/CSS/Values/AttrValue.hpp"
#include "Krystal.Booey/CSS/Values/BackgroundRepeatValue.hpp"
#include "Krystal.Booey/CSS/Values/BasicShapeValue.hpp"
#include "Krystal.Booey/CSS/Values/BorderImageSliceValue.hpp"
#include "Krystal.Booey/CSS/Values/BorderImageWidthValue.hpp"
#include "Krystal.Booey/CSS/Values/BoxShadowPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/CanvasValue.hpp"
#include "Krystal.Booey/CSS/Values/ColorSchemeValue.hpp"
#include "Krystal.Booey/CSS/Values/ColorValue.hpp"
#include "Krystal.Booey/CSS/Values/CounterValue.hpp"
#include "Krystal.Booey/CSS/Values/CrossfadeValue.hpp"
#include "Krystal.Booey/CSS/Values/CursorImageValue.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/DynamicRangeLimitValue.hpp"
#include "Krystal.Booey/CSS/Values/EasingFunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/FilterImageValue.hpp"
#include "Krystal.Booey/CSS/Values/FilterValue.hpp"
#include "Krystal.Booey/CSS/Values/FontFaceSrcValue.hpp"
#include "Krystal.Booey/CSS/Values/FontFeatureValue.hpp"
#include "Krystal.Booey/CSS/Values/FontStyleRangeValue.hpp"
#include "Krystal.Booey/CSS/Values/FontStyleWithAngleValue.hpp"
#include "Krystal.Booey/CSS/Values/FontValue.hpp"
#include "Krystal.Booey/CSS/Values/FontVariationValue.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/GradientValue.hpp"
#include "Krystal.Booey/CSS/Values/GridAutoRepeatValue.hpp"
#include "Krystal.Booey/CSS/Values/GridIntegerRepeatValue.hpp"
#include "Krystal.Booey/CSS/Values/GridLineNamesValue.hpp"
#include "Krystal.Booey/CSS/Values/GridLineValue.hpp"
#include "Krystal.Booey/CSS/Values/GridTemplateAreasValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageSetOptionValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageSetValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/CSS/Values/NamedImageValue.hpp"
#include "Krystal.Booey/CSS/Values/OffsetRotateValue.hpp"
#include "Krystal.Booey/CSS/Values/PaintImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PathValue.hpp"
#include "Krystal.Booey/CSS/Values/PendingSubstitutionValue.hpp"
#include "Krystal.Booey/CSS/Values/PositionValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/QuadValue.hpp"
#include "Krystal.Booey/CSS/Values/RatioValue.hpp"
#include "Krystal.Booey/CSS/Values/RayValue.hpp"
#include "Krystal.Booey/CSS/Values/RectValue.hpp"
#include "Krystal.Booey/CSS/Values/ReflectValue.hpp"
#include "Krystal.Booey/CSS/Values/ScrollValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/SubgridValue.hpp"
#include "Krystal.Booey/CSS/Values/TextShadowPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/TransformListValue.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/CSS/Values/UnicodeRangeValue.hpp"
#include "Krystal.Booey/CSS/Values/URLValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"
#include "Krystal.Booey/CSS/Values/ViewValue.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"

namespace krys::boo::css
{
  template <typename Visitor>
  constexpr decltype(auto) Value::VisitDerived(Visitor &&visitor) noexcept
  {
    switch (_type)
    {
      case ValueType::AppleColorFilter:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<AppleColorFilterValue>(*this));
      }
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
      case ValueType::Canvas:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<CanvasValue>(*this));
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
      case ValueType::Font:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<FontValue>(*this));
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
      case ValueType::NamedImage:
      {
        return std::invoke(std::forward<Visitor>(visitor), Downcast<NamedImageValue>(*this));
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

  bool Value::CustomTraverseSubresources(const Func<bool(const CachedResource &)> &) noexcept
  {
    return false;
  }

  template <typename Visitor>
  constexpr decltype(auto) Value::VisitDerived(Visitor &&visitor) const noexcept
  {
    return const_cast<Value &>(*this).VisitDerived(
      [&](auto &value) { return std::invoke(std::forward<Visitor>(visitor), std::as_const(value)); });
  }

  IterationStatus Value::VisitChildren(const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitDerived([&](auto &value) { return value.CustomVisitChildren(func); });
  }

  bool Value::MayDependOnBaseUrl() const noexcept
  {
    return VisitDerived([&](auto &value) { return value.CustomMayDependOnBaseUrl(); });
  }

  bool Value::TraverseSubresources(const Func<bool(const CachedResource &)> &handler) const noexcept
  {
    return VisitDerived([&](auto &value) { return value.CustomTraverseSubresources(handler); });
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
    if (auto *asList = DynamicDowncast<ValueWithSmallList>(*this))
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
    if (_type == other._type)
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

    seed = Hash::Combine(seed, _type);

    return VisitDerived([&](auto &typedThis) { return typedThis.AddDerivedHash(seed); });
  }

  bool Value::AddDerivedHash(size_t &seed) const noexcept
  {
    seed = Hash::Combine(seed, this);
    return false;
  }

  bool Value::IsCSSLocalURL(CSSOMStringView relativeURL) noexcept
  {
    return relativeURL.empty() || relativeURL.starts_with(u8'#');
  }

  CSSOMStringView Value::SeparatorCSSText(ValueSeparator separator) noexcept
  {
    switch (separator)
    {
      case ValueSeparator::SpaceSeparator: return u8" ";
      case ValueSeparator::CommaSeparator: return u8", ";
      case ValueSeparator::SlashSeparator: return u8" / ";
    }

    krys_debug_assert(false);
    return u8" ";
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
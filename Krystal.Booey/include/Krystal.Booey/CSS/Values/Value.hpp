#pragma once

#include "Krystal.Booey/CSS/Values/UnitCategory.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/CSS/Values/ValueType.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  struct ComputedStyleDependencies;

  class Value : public RefCounted<Value>
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();

  protected:
    ValueType _type : BitCount<ValueType>();
    UnitType _unit : BitCount<UnitType>() {UnitType::Unknown};
    bool _isImplicitInitialValue : 1 {false};

    Value(ValueType type) noexcept : _type(type)
    {
    }

    struct StaticValueTag
    {
    };

    constexpr static StaticValueTag StaticValue {};

  public:
    virtual ~Value() noexcept = default;

    KRYS_NODISCARD css::ValueId ValueId() const noexcept;

    KRYS_NODISCARD bool IsImplicitInitialValue() const noexcept
    {
      return _isImplicitInitialValue;
    }

    KRYS_NODISCARD bool Equals(const Value &other) const noexcept;

    KRYS_NODISCARD bool operator==(const Value &other) const noexcept
    {
      return Equals(other);
    }

    KRYS_NODISCARD css::ComputedStyleDependencies ComputedStyleDependencies() const noexcept;

#pragma region Type Checks

    KRYS_NODISCARD bool IsAppleColorFilterValue() const noexcept
    {
      return _type == ValueType::AppleColorFilter;
    }

    KRYS_NODISCARD bool IsAttrValue() const noexcept
    {
      return _type == ValueType::Attr;
    }

    KRYS_NODISCARD bool IsBackgroundRepeatValue() const noexcept
    {
      return _type == ValueType::BackgroundRepeat;
    }

    KRYS_NODISCARD bool IsBasicShapeValue() const noexcept
    {
      return _type == ValueType::BasicShape;
    }

    KRYS_NODISCARD bool IsBorderImageSliceValue() const noexcept
    {
      return _type == ValueType::BorderImageSlice;
    }

    KRYS_NODISCARD bool IsBorderImageWidthValue() const noexcept
    {
      return _type == ValueType::BorderImageWidth;
    }

    KRYS_NODISCARD bool IsBoxShadowPropertyValue() const noexcept
    {
      return _type == ValueType::BoxShadowProperty;
    }

    KRYS_NODISCARD bool IsCanvasValue() const noexcept
    {
      return _type == ValueType::Canvas;
    }

    KRYS_NODISCARD bool IsColorValue() const noexcept
    {
      return _type == ValueType::Color;
    }

    KRYS_NODISCARD bool IsColorSchemeValue() const noexcept
    {
      return _type == ValueType::ColorScheme;
    }

    KRYS_NODISCARD bool IsCounterValue() const noexcept
    {
      return _type == ValueType::Counter;
    }

    KRYS_NODISCARD bool IsCrossFadeValue() const noexcept
    {
      return _type == ValueType::CrossFade;
    }

    KRYS_NODISCARD bool IsCursorImageValue() const noexcept
    {
      return _type == ValueType::CursorImage;
    }

    KRYS_NODISCARD bool IsCustomPropertyValue() const noexcept
    {
      return _type == ValueType::CustomProperty;
    }

    KRYS_NODISCARD bool IsDynamicRangeLimitValue() const noexcept
    {
      return _type == ValueType::DynamicRangeLimit;
    }

    KRYS_NODISCARD bool IsEasingFunctionValue() const noexcept
    {
      return _type == ValueType::EasingFunction;
    }

    KRYS_NODISCARD bool IsFilterImageValue() const noexcept
    {
      return _type == ValueType::FilterImage;
    }

    KRYS_NODISCARD bool IsFilterValue() const noexcept
    {
      return _type == ValueType::Filter;
    }

    KRYS_NODISCARD bool IsFontFeatureValue() const noexcept
    {
      return _type == ValueType::FontFeature;
    }

    KRYS_NODISCARD bool IsFontStyleRangeValue() const noexcept
    {
      return _type == ValueType::FontStyleRange;
    }

    KRYS_NODISCARD bool IsFontStyleWithAngleValue() const noexcept
    {
      return _type == ValueType::FontStyleWithAngle;
    }

    KRYS_NODISCARD bool IsFontVariationValue() const noexcept
    {
      return _type == ValueType::FontVariation;
    }

    KRYS_NODISCARD bool IsFunctionValue() const noexcept
    {
      return _type == ValueType::Function;
    }

    KRYS_NODISCARD bool IsGradientValue() const noexcept
    {
      return _type == ValueType::Gradient;
    }

    KRYS_NODISCARD bool IsGridAutoRepeatValue() const noexcept
    {
      return _type == ValueType::GridAutoRepeat;
    }

    KRYS_NODISCARD bool IsGridIntegerRepeatValue() const noexcept
    {
      return _type == ValueType::GridIntegerRepeat;
    }

    KRYS_NODISCARD bool IsGridLineValue() const noexcept
    {
      return _type == ValueType::GridLine;
    }

    KRYS_NODISCARD bool IsGridLineNamesValue() const noexcept
    {
      return _type == ValueType::GridLineNames;
    }

    KRYS_NODISCARD bool IsGridTemplateAreasValue() const noexcept
    {
      return _type == ValueType::GridTemplateAreas;
    }

    KRYS_NODISCARD bool IsImageValue() const noexcept
    {
      return _type == ValueType::Image;
    }

    KRYS_NODISCARD bool IsImageSetValue() const noexcept
    {
      return _type == ValueType::ImageSet;
    }

    KRYS_NODISCARD bool IsNamedImageValue() const noexcept
    {
      return _type == ValueType::NamedImage;
    }

    KRYS_NODISCARD bool IsOffsetRotateValue() const noexcept
    {
      return _type == ValueType::OffsetRotate;
    }

    KRYS_NODISCARD bool IsPaintImageValue() const noexcept
    {
      return _type == ValueType::PaintImage;
    }

    KRYS_NODISCARD bool IsPathValue() const noexcept
    {
      return _type == ValueType::Path;
    }

    KRYS_NODISCARD bool IsPendingSubstitutionValue() const noexcept
    {
      return _type == ValueType::PendingSubstitution;
    }

    KRYS_NODISCARD bool IsPositionValue() const noexcept
    {
      return _type == ValueType::Position;
    }

    KRYS_NODISCARD bool IsPositionXValue() const noexcept
    {
      return _type == ValueType::PositionX;
    }

    KRYS_NODISCARD bool IsPositionYValue() const noexcept
    {
      return _type == ValueType::PositionY;
    }

    KRYS_NODISCARD bool IsPrimitiveValue() const noexcept
    {
      return _type == ValueType::Primitive;
    }

    KRYS_NODISCARD bool IsQuadValue() const noexcept
    {
      return _type == ValueType::Quad;
    }

    KRYS_NODISCARD bool IsRatioValue() const noexcept
    {
      return _type == ValueType::Ratio;
    }

    KRYS_NODISCARD bool IsRayValue() const noexcept
    {
      return _type == ValueType::Ray;
    }

    KRYS_NODISCARD bool IsRectValue() const noexcept
    {
      return _type == ValueType::Rect;
    }

    KRYS_NODISCARD bool IsReflectValue() const noexcept
    {
      return _type == ValueType::Reflect;
    }

    KRYS_NODISCARD bool IsScrollValue() const noexcept
    {
      return _type == ValueType::Scroll;
    }

    KRYS_NODISCARD bool IsSubgridValue() const noexcept
    {
      return _type == ValueType::Subgrid;
    }

    KRYS_NODISCARD bool IsTextShadowPropertyValue() const noexcept
    {
      return _type == ValueType::TextShadowProperty;
    }

    KRYS_NODISCARD bool IsTransformListValue() const noexcept
    {
      return _type == ValueType::TransformList;
    }

    KRYS_NODISCARD bool IsUnicodeRangeValue() const noexcept
    {
      return _type == ValueType::UnicodeRange;
    }

    KRYS_NODISCARD bool IsUrlValue() const noexcept
    {
      return _type == ValueType::Url;
    }

    KRYS_NODISCARD bool IsValueListValue() const noexcept
    {
      return _type == ValueType::ValueList;
    }

    KRYS_NODISCARD bool IsValuePairValue() const noexcept
    {
      return _type == ValueType::ValuePair;
    }

    KRYS_NODISCARD bool IsVariableReferenceValue() const noexcept
    {
      return _type == ValueType::VariableReference;
    }

    KRYS_NODISCARD bool IsViewValue() const noexcept
    {
      return _type == ValueType::View;
    }

#pragma endregion
  };
}
#pragma once

#include "Krystal.HTML/CSS/Values/Enums/CSSUnitCategory.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSUnitType.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueType.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"

namespace Krys::HTML
{
  class CSSValue : public RefCounted<CSSValue>
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();

  protected:
    CSSValueType _type : BitCount<CSSValueType>();
    CSSUnitType _unit : BitCount<CSSUnitType>() {CSSUnitType::Unknown};
    bool _isImplicitInitialValue : 1 {false};

    CSSValue(CSSValueType type) noexcept : _type(type)
    {
    }

    struct StaticCSSValueTag
    {
    };

    constexpr inline static StaticCSSValueTag StaticCSSValue {};

  public:
    virtual ~CSSValue() noexcept = default;

    KRYS_NODISCARD CSSValueId ValueId() const noexcept;

    KRYS_NODISCARD bool IsImplicitInitialValue() const noexcept
    {
      return _isImplicitInitialValue;
    }

    KRYS_NODISCARD static bool IsCSSWideKeyword(CSSValueId id) noexcept
    {
      switch (id)
      {
        case CSSValueId::Initial:
        case CSSValueId::Inherit:
        case CSSValueId::Unset:
        case CSSValueId::Revert:
        case CSSValueId::RevertLayer:
        {
          return true;
        }
        default:
        {
          return false;
        }
      }
    }

    KRYS_NODISCARD bool Equals(const CSSValue &other) const noexcept;

    KRYS_NODISCARD bool operator==(const CSSValue &other) const noexcept
    {
      return Equals(other);
    }

#pragma region Type Checks

    KRYS_NODISCARD bool IsCSSAttrValue() const noexcept
    {
      return _type == CSSValueType::Attr;
    }

    KRYS_NODISCARD bool IsCSSColorValue() const noexcept
    {
      return _type == CSSValueType::Color;
    }

    KRYS_NODISCARD bool IsCSSColorSchemeValue() const noexcept
    {
      return _type == CSSValueType::ColorScheme;
    }

    KRYS_NODISCARD bool IsCSSCounterValue() const noexcept
    {
      return _type == CSSValueType::Counter;
    }

    KRYS_NODISCARD bool IsCSSCursorImageValue() const noexcept
    {
      return _type == CSSValueType::CursorImage;
    }

    KRYS_NODISCARD bool IsCSSGridAutoRepeatValue() const noexcept
    {
      return _type == CSSValueType::GridAutoRepeat;
    }

    KRYS_NODISCARD bool IsCSSGridIntegerRepeatValue() const noexcept
    {
      return _type == CSSValueType::GridIntegerRepeat;
    }

    KRYS_NODISCARD bool IsCSSGridLineValue() const noexcept
    {
      return _type == CSSValueType::GridLine;
    }

    KRYS_NODISCARD bool IsCSSGridLineNamesValue() const noexcept
    {
      return _type == CSSValueType::GridLineNames;
    }

    KRYS_NODISCARD bool IsCSSGridTemplateAreasValue() const noexcept
    {
      return _type == CSSValueType::GridTemplateAreas;
    }

    KRYS_NODISCARD bool IsCSSFunctionValue() const noexcept
    {
      return _type == CSSValueType::Function;
    }

    KRYS_NODISCARD bool IsCSSOffsetRotateValue() const noexcept
    {
      return _type == CSSValueType::OffsetRotate;
    }

    KRYS_NODISCARD bool IsCSSPositionValue() const noexcept
    {
      return _type == CSSValueType::Position;
    }

    KRYS_NODISCARD bool IsCSSPositionXValue() const noexcept
    {
      return _type == CSSValueType::PositionX;
    }

    KRYS_NODISCARD bool IsCSSPositionYValue() const noexcept
    {
      return _type == CSSValueType::PositionY;
    }

    KRYS_NODISCARD bool IsCSSPrimitiveValue() const noexcept
    {
      return _type == CSSValueType::Primitive;
    }

    KRYS_NODISCARD bool IsCSSQuadValue() const noexcept
    {
      return _type == CSSValueType::Quad;
    }

    KRYS_NODISCARD bool IsCSSRatioValue() const noexcept
    {
      return _type == CSSValueType::Ratio;
    }

    KRYS_NODISCARD bool IsCSSRayValue() const noexcept
    {
      return _type == CSSValueType::Ray;
    }

    KRYS_NODISCARD bool IsCSSRectValue() const noexcept
    {
      return _type == CSSValueType::Rect;
    }

    KRYS_NODISCARD bool IsCSSScrollValue() const noexcept
    {
      return _type == CSSValueType::Scroll;
    }

    KRYS_NODISCARD bool IsCSSSubgridValue() const noexcept
    {
      return _type == CSSValueType::Subgrid;
    }

    KRYS_NODISCARD bool IsCSSTextShadowPropertyValue() const noexcept
    {
      return _type == CSSValueType::TextShadowProperty;
    }

    KRYS_NODISCARD bool IsCSSTransformListValue() const noexcept
    {
      return _type == CSSValueType::TransformList;
    }

    KRYS_NODISCARD bool IsCSSUnicodeRangeValue() const noexcept
    {
      return _type == CSSValueType::UnicodeRange;
    }

    KRYS_NODISCARD bool IsCSSURLValue() const noexcept
    {
      return _type == CSSValueType::URL;
    }

    KRYS_NODISCARD bool IsCSSValueListValue() const noexcept
    {
      return _type == CSSValueType::ValueList;
    }

    KRYS_NODISCARD bool IsCSSValuePairValue() const noexcept
    {
      return _type == CSSValueType::ValuePair;
    }

    KRYS_NODISCARD bool IsCSSViewValue() const noexcept
    {
      return _type == CSSValueType::View;
    }

#pragma endregion
  };
}
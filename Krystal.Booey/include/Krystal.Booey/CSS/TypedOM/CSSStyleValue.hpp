#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class Value;

  /// @brief The kinds of Typed OM values.
  enum class CSSStyleValueType : uint8
  {
    CSSStyleValue,
    CSSStyleImageValue,
    CSSTransformValue,
    CSSMathClamp,
    CSSMathInvert,
    CSSMathMin,
    CSSMathMax,
    CSSMathNegate,
    CSSMathProduct,
    CSSMathSum,
    CSSColorCSSOMColor,
    CSSColorHSL,
    CSSColorHWB,
    CSSColorLab,
    CSSColorLCH,
    CSSColorOKLCH,
    CSSColorOKLab,
    CSSColorRGB,
    CSSUnitValue,
    CSSUnparsedValue,
    CSSKeywordValue
  };

  KRYS_NODISCARD constexpr bool IsCSSNumericValue(CSSStyleValueType type) noexcept
  {
    switch (type)
    {
      case CSSStyleValueType::CSSMathClamp:
      case CSSStyleValueType::CSSMathInvert:
      case CSSStyleValueType::CSSMathMin:
      case CSSStyleValueType::CSSMathMax:
      case CSSStyleValueType::CSSMathNegate:
      case CSSStyleValueType::CSSMathProduct:
      case CSSStyleValueType::CSSMathSum:
      case CSSStyleValueType::CSSUnitValue:
      {
        return true;
      }
      case CSSStyleValueType::CSSStyleValue:
      case CSSStyleValueType::CSSStyleImageValue:
      case CSSStyleValueType::CSSTransformValue:
      case CSSStyleValueType::CSSUnparsedValue:
      case CSSStyleValueType::CSSKeywordValue:
      case CSSStyleValueType::CSSColorCSSOMColor:
      case CSSStyleValueType::CSSColorHSL:
      case CSSStyleValueType::CSSColorHWB:
      case CSSStyleValueType::CSSColorLab:
      case CSSStyleValueType::CSSColorLCH:
      case CSSStyleValueType::CSSColorOKLab:
      case CSSStyleValueType::CSSColorOKLCH:
      case CSSStyleValueType::CSSColorRGB:
      {
        break;
      }
    }
    return false;
  }

  KRYS_NODISCARD constexpr bool IsCSSMathValue(CSSStyleValueType type) noexcept
  {
    switch (type)
    {
      case CSSStyleValueType::CSSMathClamp:
      case CSSStyleValueType::CSSMathInvert:
      case CSSStyleValueType::CSSMathMin:
      case CSSStyleValueType::CSSMathMax:
      case CSSStyleValueType::CSSMathNegate:
      case CSSStyleValueType::CSSMathProduct:
      case CSSStyleValueType::CSSMathSum:
      {
        return true;
      }
      case CSSStyleValueType::CSSUnitValue:
      case CSSStyleValueType::CSSStyleValue:
      case CSSStyleValueType::CSSStyleImageValue:
      case CSSStyleValueType::CSSTransformValue:
      case CSSStyleValueType::CSSUnparsedValue:
      case CSSStyleValueType::CSSKeywordValue:
      case CSSStyleValueType::CSSColorCSSOMColor:
      case CSSStyleValueType::CSSColorHSL:
      case CSSStyleValueType::CSSColorHWB:
      case CSSStyleValueType::CSSColorLab:
      case CSSStyleValueType::CSSColorLCH:
      case CSSStyleValueType::CSSColorOKLab:
      case CSSStyleValueType::CSSColorOKLCH:
      case CSSStyleValueType::CSSColorRGB:
      {
        break;
      }
    }
    return false;
  }

  KRYS_NODISCARD constexpr bool IsCSSColorValue(CSSStyleValueType type) noexcept
  {
    switch (type)
    {
      case CSSStyleValueType::CSSColorCSSOMColor:
      case CSSStyleValueType::CSSColorHSL:
      case CSSStyleValueType::CSSColorHWB:
      case CSSStyleValueType::CSSColorLab:
      case CSSStyleValueType::CSSColorLCH:
      case CSSStyleValueType::CSSColorOKLab:
      case CSSStyleValueType::CSSColorOKLCH:
      case CSSStyleValueType::CSSColorRGB:
      {
        return true;
      }
      case CSSStyleValueType::CSSMathClamp:
      case CSSStyleValueType::CSSMathInvert:
      case CSSStyleValueType::CSSMathMin:
      case CSSStyleValueType::CSSMathMax:
      case CSSStyleValueType::CSSMathNegate:
      case CSSStyleValueType::CSSMathProduct:
      case CSSStyleValueType::CSSMathSum:
      case CSSStyleValueType::CSSUnitValue:
      case CSSStyleValueType::CSSStyleValue:
      case CSSStyleValueType::CSSStyleImageValue:
      case CSSStyleValueType::CSSTransformValue:
      case CSSStyleValueType::CSSUnparsedValue:
      case CSSStyleValueType::CSSKeywordValue:
      {
        break;
      }
    }
    return false;
  }

  /// @brief How a value nested in a math value is serialised.
  enum class SerialisationArguments : uint8
  {
    Nested,
    WithoutParentheses,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::SerialisationArguments, 2uz);

namespace krys::boo::css
{
  /// @brief A CSS value as the Typed OM represents it.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssstylevalue
  // FIXME: `parse()` and `parseAll()`, which need the style value factory and the kinds of values it reifies
  // (unparsed, image, transform and colour values) and the property maps that use them.
  class CSSStyleValue : public RefCounted<CSSStyleValue>
  {
    KRYS_NON_COPY_MOVABLE(CSSStyleValue);

  protected:
    CSSOMString _customPropertyName;
    RefPtr<Value> _propertyValue;

  public:
    virtual ~CSSStyleValue() noexcept;

    KRYS_NODISCARD static Ref<CSSStyleValue> Create(RefPtr<Value> &&cssValue,
                                                    CSSOMString &&property = {}) noexcept;
    KRYS_NODISCARD static Ref<CSSStyleValue> Create() noexcept;

    /// @see https://drafts.css-houdini.org/css-typed-om/#stylevalue-serialization
    KRYS_NODISCARD CSSOMString ToString() const noexcept;
    virtual void Serialise(StringBuilder &builder,
                           EnumSet<SerialisationArguments> arguments = {}) const noexcept;

    KRYS_NODISCARD virtual CSSStyleValueType StyleValueType() const noexcept
    {
      return CSSStyleValueType::CSSStyleValue;
    }

    KRYS_NODISCARD virtual RefPtr<Value> ToCSSValue() const noexcept;
    KRYS_NODISCARD virtual RefPtr<Value> ToCSSValueWithProperty(PropertyId) const noexcept
    {
      return ToCSSValue();
    }

  protected:
    explicit CSSStyleValue(RefPtr<Value> &&cssValue, CSSOMString &&property = {}) noexcept;
    CSSStyleValue() noexcept;
  };
}

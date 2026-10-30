#include "Krystal.Booey/SVG/SVGTransformValue.hpp"
#include "Krystal.Booey/Core/FloatConversion.hpp"
#include "Krystal.Booey/Core/NumberFormatting.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <cmath>

namespace krys::boo::svg
{
  void SVGTransformValue::AppendFixedPrecisionNumbers(StringBuilder &builder) noexcept
  {
    builder.Append(u8')');
  }

  template <typename TNumber, typename... TNumbers>
  void SVGTransformValue::AppendFixedPrecisionNumbers(StringBuilder &builder, TNumber number,
                                                      TNumbers... numbers) noexcept
  {
    if (builder.Length() && builder[builder.Length() - 1] != u8'(')
    {
      builder.Append(u8' ');
    }

    // FIXME: Shortest form would be better, but fixed precision is required for now to smooth over precision
    // errors caused by converting float to double and back since we use AffineTransform to store transforms.
    FormatNumberFixedPrecision(builder, number);
    AppendFixedPrecisionNumbers(builder, numbers...);
  }

  dom::DOMString SVGTransformValue::ValueAsString() const noexcept
  {
    StringBuilder builder;
    builder.Append(PrefixForTransformType(_type));
    switch (_type)
    {
      case SVG_TRANSFORM_UNKNOWN:   break;
      case SVG_TRANSFORM_MATRIX:    AppendMatrix(builder); break;
      case SVG_TRANSFORM_TRANSLATE: AppendTranslate(builder); break;
      case SVG_TRANSFORM_SCALE:     AppendScale(builder); break;
      case SVG_TRANSFORM_ROTATE:    AppendRotate(builder); break;
      case SVG_TRANSFORM_SKEWX:     AppendSkewX(builder); break;
      case SVG_TRANSFORM_SKEWY:     AppendSkewY(builder); break;
    }
    return dom::DOMString(krys::move(builder).ToString());
  }

  utf8_stringview SVGTransformValue::PrefixForTransformType(SVGTransformType type) noexcept
  {
    switch (type)
    {
      case SVG_TRANSFORM_UNKNOWN:   return u8"";
      case SVG_TRANSFORM_MATRIX:    return u8"matrix(";
      case SVG_TRANSFORM_TRANSLATE: return u8"translate(";
      case SVG_TRANSFORM_SCALE:     return u8"scale(";
      case SVG_TRANSFORM_ROTATE:    return u8"rotate(";
      case SVG_TRANSFORM_SKEWX:     return u8"skewX(";
      case SVG_TRANSFORM_SKEWY:     return u8"skewY(";
    }
    krys_assert_msg(false, "Not reached");
    return u8"";
  }

  void SVGTransformValue::AppendMatrix(StringBuilder &builder) const noexcept
  {
    AppendFixedPrecisionNumbers(builder, _matrix->A(), _matrix->B(), _matrix->C(), _matrix->D(), _matrix->E(),
                                _matrix->F());
  }

  void SVGTransformValue::AppendTranslate(StringBuilder &builder) const noexcept
  {
    AppendFixedPrecisionNumbers(builder, _matrix->E(), _matrix->F());
  }

  void SVGTransformValue::AppendScale(StringBuilder &builder) const noexcept
  {
    AppendFixedPrecisionNumbers(builder, _matrix->A(), _matrix->D());
  }

  void SVGTransformValue::AppendRotate(StringBuilder &builder) const noexcept
  {
    double angleInRad = deg2rad(static_cast<double>(_angle));
    double cosAngle = std::cos(angleInRad);
    double sinAngle = std::sin(angleInRad);
    float cx = NarrowPrecisionToFloat(
      cosAngle != 1 ? (_matrix->E() * (1 - cosAngle) - _matrix->F() * sinAngle) / (1 - cosAngle) / 2 : 0);
    float cy = NarrowPrecisionToFloat(
      cosAngle != 1 ? (_matrix->E() * sinAngle / (1 - cosAngle) + _matrix->F()) / 2 : 0);
    if (cx || cy)
    {
      AppendFixedPrecisionNumbers(builder, _angle, cx, cy);
    }
    else
    {
      AppendFixedPrecisionNumbers(builder, _angle);
    }
  }

  void SVGTransformValue::AppendSkewX(StringBuilder &builder) const noexcept
  {
    AppendFixedPrecisionNumbers(builder, _angle);
  }

  void SVGTransformValue::AppendSkewY(StringBuilder &builder) const noexcept
  {
    AppendFixedPrecisionNumbers(builder, _angle);
  }
}

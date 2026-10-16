#include "Krystal.Booey/Style/Values/Transforms/TransformFunction.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"

#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/TransformListValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/Matrix3DTransformFunction.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/MatrixTransformFunction.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/PerspectiveTransformFunction.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/RotateTransformFunction.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/ScaleTransformFunction.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/SkewTransformFunction.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TranslateTransformFunction.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  KRYS_NODISCARD static TranslateTransformFunction::LengthPercentage
    ResolveAsTranslateLengthPercentage(const css::PrimitiveValue &primitiveValue,
                                       BuilderState &state) noexcept
  {
    // FIXME: This should use `ConvertCSSValueToStyle<TranslateTransformFunction::LengthPercentage>`, but
    // doing so breaks transforms/hittest-translated-content-off-to-infinity-and-back.html, due to it clamping
    // between css::MinValueForCssLength/css::MaxValueForCssLength.

    auto &conversionData = state.CssToLengthConversionData();
    if (primitiveValue.IsLength())
    {
      return TranslateTransformFunction::LengthPercentage::Fixed {
        static_cast<float>(primitiveValue.ResolveAsLength<double>(conversionData))};
    }
    if (primitiveValue.IsPercentage())
    {
      return TranslateTransformFunction::LengthPercentage::Percentage {
        static_cast<float>(primitiveValue.ResolveAsPercentage<double>(conversionData))};
    }
    if (primitiveValue.IsCalculated())
    {
      return TranslateTransformFunction::LengthPercentage::Calc {
        primitiveValue.ProtectedCSSCalcValue()->CreateCalculationValue(conversionData, css::SymbolTable {})};
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return 0_css_px;
  }

  KRYS_NODISCARD static TranslateTransformFunction::Length
    ResolveAsTranslateLength(const css::PrimitiveValue &primitiveValue, BuilderState &state) noexcept
  {
    // FIXME: This should use `ConvertCSSValueToStyle<TranslateTransformFunction::Length>`, but doing so
    // breaks transforms/hittest-translated-content-off-to-infinity-and-back.html, due to it clamping between
    // css::MinValueForCssLength/css::MaxValueForCssLength.

    auto &conversionData = state.CssToLengthConversionData();
    if (primitiveValue.IsLength())
    {
      return TranslateTransformFunction::Length {
        static_cast<float>(primitiveValue.ResolveAsLength<double>(conversionData))};
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return 0_css_px;
  }

  // MARK: Matrix

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateMatrixTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-1/#funcdef-transform-matrix
    // matrix() = matrix( <number>#{6} )

    auto function = RequiredFunctionDowncast<css::ValueId::Matrix, css::PrimitiveValue, 6>(state, value);
    if (!function)
    {
      return {};
    }

    auto zoom = state.CssToLengthConversionData().Zoom();
    return MatrixTransformFunction::Create(
      ConvertCSSValueToStyle<Number<>>(state, function->item(0)).Value,
      ConvertCSSValueToStyle<Number<>>(state, function->item(1)).Value,
      ConvertCSSValueToStyle<Number<>>(state, function->item(2)).Value,
      ConvertCSSValueToStyle<Number<>>(state, function->item(3)).Value,
      ConvertCSSValueToStyle<Number<>>(state, function->item(4)).Value * zoom,
      ConvertCSSValueToStyle<Number<>>(state, function->item(5)).Value * zoom);
  }

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateMatrix3dTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#funcdef-matrix3d
    // matrix3d() = matrix3d( <number>#{16} )

    auto function = RequiredFunctionDowncast<css::ValueId::Matrix3d, css::PrimitiveValue, 16>(state, value);
    if (!function)
    {
      return {};
    }

    TransformationMatrix matrix(ConvertCSSValueToStyle<Number<>>(state, function->item(0)).Value,
                                ConvertCSSValueToStyle<Number<>>(state, function->item(1)).Value,
                                ConvertCSSValueToStyle<Number<>>(state, function->item(2)).Value,
                                ConvertCSSValueToStyle<Number<>>(state, function->item(3)).Value,
                                ConvertCSSValueToStyle<Number<>>(state, function->item(4)).Value,
                                ConvertCSSValueToStyle<Number<>>(state, function->item(5)).Value,
                                ConvertCSSValueToStyle<Number<>>(state, function->item(6)).Value,
                                ConvertCSSValueToStyle<Number<>>(state, function->item(7)).Value,
                                ConvertCSSValueToStyle<Number<>>(state, function->item(8)).Value,
                                ConvertCSSValueToStyle<Number<>>(state, function->item(9)).Value,
                                ConvertCSSValueToStyle<Number<>>(state, function->item(10)).Value,
                                ConvertCSSValueToStyle<Number<>>(state, function->item(11)).Value,
                                ConvertCSSValueToStyle<Number<>>(state, function->item(12)).Value,
                                ConvertCSSValueToStyle<Number<>>(state, function->item(13)).Value,
                                ConvertCSSValueToStyle<Number<>>(state, function->item(14)).Value,
                                ConvertCSSValueToStyle<Number<>>(state, function->item(15)).Value);
    matrix.Zoom(state.CssToLengthConversionData().Zoom());

    return Matrix3DTransformFunction::Create(krys::move(matrix));
  }

  // MARK: Rotate

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateRotateTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-1/#funcdef-transform-rotate
    // rotate() = rotate( [ <angle> | <zero> ] )

    auto function = RequiredFunctionDowncast<css::ValueId::Rotate, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto x = 0_css_number;
    auto y = 0_css_number;
    auto z = 1_css_number;
    auto angle = ConvertCSSValueToStyle<Angle<>>(state, function->item(0));

    return RotateTransformFunction::Create(x, y, z, angle, TransformFunctionType::Rotate);
  }

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateRotate3dTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#funcdef-rotate3d
    // rotate3d() = rotate3d( <number> , <number> , <number> , [ <angle> | <zero> ] )

    auto function = RequiredFunctionDowncast<css::ValueId::Rotate3d, css::PrimitiveValue, 4>(state, value);
    if (!function)
    {
      return {};
    }

    auto x = ConvertCSSValueToStyle<Number<>>(state, function->item(0));
    auto y = ConvertCSSValueToStyle<Number<>>(state, function->item(1));
    auto z = ConvertCSSValueToStyle<Number<>>(state, function->item(2));
    auto angle = ConvertCSSValueToStyle<Angle<>>(state, function->item(3));

    return RotateTransformFunction::Create(x, y, z, angle, TransformFunctionType::Rotate3D);
  }

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateRotateXTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#funcdef-rotatex
    // rotateX() = rotateX( [ <angle> | <zero> ] )

    auto function = RequiredFunctionDowncast<css::ValueId::RotateX, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto x = 1_css_number;
    auto y = 0_css_number;
    auto z = 0_css_number;
    auto angle = ConvertCSSValueToStyle<Angle<>>(state, function->item(0));

    return RotateTransformFunction::Create(x, y, z, angle, TransformFunctionType::RotateX);
  }

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateRotateYTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#funcdef-rotatey
    // rotateY() = rotateY( [ <angle> | <zero> ] )

    auto function = RequiredFunctionDowncast<css::ValueId::RotateY, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto x = 0_css_number;
    auto y = 1_css_number;
    auto z = 0_css_number;
    auto angle = ConvertCSSValueToStyle<Angle<>>(state, function->item(0));

    return RotateTransformFunction::Create(x, y, z, angle, TransformFunctionType::RotateY);
  }

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateRotateZTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#funcdef-rotatez
    // rotateZ() = rotateZ( [ <angle> | <zero> ] )

    auto function = RequiredFunctionDowncast<css::ValueId::RotateZ, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto x = 0_css_number;
    auto y = 0_css_number;
    auto z = 1_css_number;
    auto angle = ConvertCSSValueToStyle<Angle<>>(state, function->item(0));

    return RotateTransformFunction::Create(x, y, z, angle, TransformFunctionType::RotateZ);
  }

  // MARK: Skew

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateSkewTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-1/#funcdef-transform-skew
    // skew() = skew( [ <angle> | <zero> ] , [ <angle> | <zero> ]? )

    auto function = RequiredFunctionDowncast<css::ValueId::Skew, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto angleX = ConvertCSSValueToStyle<Angle<>>(state, function->item(0));
    auto angleY =
      function->size() > 1 ? ConvertCSSValueToStyle<Angle<>>(state, function->item(1)) : Angle<> {0_css_deg};

    return SkewTransformFunction::Create(angleX, angleY, TransformFunctionType::Skew);
  }

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateSkewXTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-1/#funcdef-transform-skewx
    // skewX() = skewX( [ <angle> | <zero> ] )

    auto function = RequiredFunctionDowncast<css::ValueId::SkewX, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto angleX = ConvertCSSValueToStyle<Angle<>>(state, function->item(0));
    auto angleY = 0_css_deg;

    return SkewTransformFunction::Create(angleX, angleY, TransformFunctionType::SkewX);
  }

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateSkewYTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-1/#funcdef-transform-skewy
    // skewY() = skewY( [ <angle> | <zero> ] )

    auto function = RequiredFunctionDowncast<css::ValueId::SkewY, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto angleX = 0_css_deg;
    auto angleY = ConvertCSSValueToStyle<Angle<>>(state, function->item(0));

    return SkewTransformFunction::Create(angleX, angleY, TransformFunctionType::SkewY);
  }

  // MARK: Scale

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateScaleTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#funcdef-scale
    // scale() = scale( [ <number> | <percentage> ]#{1,2} )

    auto function = RequiredFunctionDowncast<css::ValueId::Scale, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto sx = ConvertCSSValueToStyle<NumberOrPercentageResolvedToNumber<>>(state, function->item(0));
    auto sy = function->size() > 1
                ? ConvertCSSValueToStyle<NumberOrPercentageResolvedToNumber<>>(state, function->item(1))
                : sx;
    auto sz = 1_css_number;

    return ScaleTransformFunction::Create(sx, sy, sz, TransformFunctionType::Scale);
  }

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateScale3dTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#funcdef-scale3d
    // scale3d() = scale3d( [ <number> | <percentage> ]#{3} )

    auto function = RequiredFunctionDowncast<css::ValueId::Scale3d, css::PrimitiveValue, 3>(state, value);
    if (!function)
    {
      return {};
    }

    auto sx = ConvertCSSValueToStyle<NumberOrPercentageResolvedToNumber<>>(state, function->item(0));
    auto sy = ConvertCSSValueToStyle<NumberOrPercentageResolvedToNumber<>>(state, function->item(1));
    auto sz = ConvertCSSValueToStyle<NumberOrPercentageResolvedToNumber<>>(state, function->item(2));

    return ScaleTransformFunction::Create(sx, sy, sz, TransformFunctionType::Scale3D);
  }

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateScaleXTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#funcdef-scalex
    // scaleX() = scaleX( [ <number> | <percentage> ] )

    auto function = RequiredFunctionDowncast<css::ValueId::ScaleX, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto sx = ConvertCSSValueToStyle<NumberOrPercentageResolvedToNumber<>>(state, function->item(0));
    auto sy = 1_css_number;
    auto sz = 1_css_number;

    return ScaleTransformFunction::Create(sx, sy, sz, TransformFunctionType::ScaleX);
  }

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateScaleYTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#funcdef-scaley
    // scaleY() = scaleY( [ <number> | <percentage> ] )

    auto function = RequiredFunctionDowncast<css::ValueId::ScaleY, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto sx = 1_css_number;
    auto sy = ConvertCSSValueToStyle<NumberOrPercentageResolvedToNumber<>>(state, function->item(0));
    auto sz = 1_css_number;

    return ScaleTransformFunction::Create(sx, sy, sz, TransformFunctionType::ScaleY);
  }

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateScaleZTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#funcdef-scalez
    // scaleZ() = scaleZ( [ <number> | <percentage> ] )

    auto function = RequiredFunctionDowncast<css::ValueId::ScaleZ, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto sx = 1_css_number;
    auto sy = 1_css_number;
    auto sz = ConvertCSSValueToStyle<NumberOrPercentageResolvedToNumber<>>(state, function->item(0));

    return ScaleTransformFunction::Create(sx, sy, sz, TransformFunctionType::ScaleZ);
  }

  // MARK: Translate

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateTranslateTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-1/#funcdef-transform-translate
    // translate() = translate( <length-percentage> , <length-percentage>? )

    auto function = RequiredFunctionDowncast<css::ValueId::Translate, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto tx = ResolveAsTranslateLengthPercentage(function->item(0), state);
    auto ty = function->size() > 1 ? ResolveAsTranslateLengthPercentage(function->item(1), state)
                                   : TranslateTransformFunction::LengthPercentage {0_css_px};
    auto tz = 0_css_px;

    return TranslateTransformFunction::Create(krys::move(tx), krys::move(ty), krys::move(tz),
                                              TransformFunctionType::Translate);
  }

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateTranslate3dTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#funcdef-translate3d
    // translate3d() = translate3d( <length-percentage> , <length-percentage> , <length> )

    auto function = RequiredFunctionDowncast<css::ValueId::Translate3d, css::PrimitiveValue, 3>(state, value);
    if (!function)
    {
      return {};
    }

    auto tx = ResolveAsTranslateLengthPercentage(function->item(0), state);
    auto ty = ResolveAsTranslateLengthPercentage(function->item(1), state);
    auto tz = ResolveAsTranslateLength(function->item(2), state);

    return TranslateTransformFunction::Create(krys::move(tx), krys::move(ty), krys::move(tz),
                                              TransformFunctionType::Translate3D);
  }

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateTranslateXTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-1/#funcdef-transform-translatex
    // translateX() = translateX( <length-percentage> )

    auto function = RequiredFunctionDowncast<css::ValueId::TranslateX, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto tx = ResolveAsTranslateLengthPercentage(function->item(0), state);
    auto ty = 0_css_px;
    auto tz = 0_css_px;

    return TranslateTransformFunction::Create(krys::move(tx), krys::move(ty), krys::move(tz),
                                              TransformFunctionType::TranslateX);
  }

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateTranslateYTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-1/#funcdef-transform-translatey
    // translateY() = translateY( <length-percentage> )

    auto function = RequiredFunctionDowncast<css::ValueId::TranslateY, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto tx = 0_css_px;
    auto ty = ResolveAsTranslateLengthPercentage(function->item(0), state);
    auto tz = 0_css_px;

    return TranslateTransformFunction::Create(krys::move(tx), krys::move(ty), krys::move(tz),
                                              TransformFunctionType::TranslateY);
  }

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreateTranslateZTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#funcdef-translatez
    // translateZ() = translateZ( <length> )

    auto function = RequiredFunctionDowncast<css::ValueId::TranslateZ, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto tx = 0_css_px;
    auto ty = 0_css_px;
    auto tz = ResolveAsTranslateLength(function->item(0), state);

    return TranslateTransformFunction::Create(krys::move(tx), krys::move(ty), krys::move(tz),
                                              TransformFunctionType::TranslateZ);
  }

  // MARK: Perspective

  KRYS_NODISCARD static RefPtr<const TransformFunctionBase>
    CreatePerspectiveTransformFunction(const css::FunctionValue &value, BuilderState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#funcdef-perspective
    // perspective() = perspective( [ <length [0,∞]> | none ] )

    auto function = RequiredFunctionDowncast<css::ValueId::Perspective, css::PrimitiveValue, 1>(state, value);
    if (!function)
    {
      return {};
    }

    auto &parameter = function->item(0);
    if (parameter.IsValueId())
    {
      krys_debug_assert(parameter.ValueId() == css::ValueId::None);
      return PerspectiveTransformFunction::Create(css::keywords::None {});
    }

    if (parameter.IsLength())
    {
      return PerspectiveTransformFunction::Create(
        ConvertCSSValueToStyle<Length<css::NonNegative>>(state, parameter));
    }

    // FIXME: Support for <number> parameters for `perspective` is a quirk that should go away when 3d
    // transforms are finalized.
    return PerspectiveTransformFunction::Create(Length<css::NonNegative> {
      static_cast<float>(ConvertCSSValueToStyle<Number<css::NonNegative>>(state, parameter).Value)});
  }

  // MARK: - Conversion

  TransformFunction CSSValueConversion<TransformFunction>::operator()(BuilderState &state,
                                                                      const css::Value &value) noexcept
  {
    auto transform = RequiredDowncast<css::FunctionValue>(state, value);
    if (!transform)
    {
      return TransformFunction {MatrixTransformFunction::CreateIdentity()};
    }

    auto makeFunction = [](RefPtr<const TransformFunctionBase> &&function)
    {
      if (!function)
      {
        return TransformFunction {MatrixTransformFunction::CreateIdentity()};
      }
      return TransformFunction {Ref<const TransformFunctionBase> {krys::move(function)}};
    };

    switch (transform->Name())
    {
      case css::ValueId::Matrix:    return makeFunction(CreateMatrixTransformFunction(*transform, state));
      case css::ValueId::Matrix3d:  return makeFunction(CreateMatrix3dTransformFunction(*transform, state));
      case css::ValueId::Rotate:    return makeFunction(CreateRotateTransformFunction(*transform, state));
      case css::ValueId::Rotate3d:  return makeFunction(CreateRotate3dTransformFunction(*transform, state));
      case css::ValueId::RotateX:   return makeFunction(CreateRotateXTransformFunction(*transform, state));
      case css::ValueId::RotateY:   return makeFunction(CreateRotateYTransformFunction(*transform, state));
      case css::ValueId::RotateZ:   return makeFunction(CreateRotateZTransformFunction(*transform, state));
      case css::ValueId::Skew:      return makeFunction(CreateSkewTransformFunction(*transform, state));
      case css::ValueId::SkewX:     return makeFunction(CreateSkewXTransformFunction(*transform, state));
      case css::ValueId::SkewY:     return makeFunction(CreateSkewYTransformFunction(*transform, state));
      case css::ValueId::Scale:     return makeFunction(CreateScaleTransformFunction(*transform, state));
      case css::ValueId::Scale3d:   return makeFunction(CreateScale3dTransformFunction(*transform, state));
      case css::ValueId::ScaleX:    return makeFunction(CreateScaleXTransformFunction(*transform, state));
      case css::ValueId::ScaleY:    return makeFunction(CreateScaleYTransformFunction(*transform, state));
      case css::ValueId::ScaleZ:    return makeFunction(CreateScaleZTransformFunction(*transform, state));
      case css::ValueId::Translate: return makeFunction(CreateTranslateTransformFunction(*transform, state));
      case css::ValueId::Translate3d:
        return makeFunction(CreateTranslate3dTransformFunction(*transform, state));
      case css::ValueId::TranslateX:
        return makeFunction(CreateTranslateXTransformFunction(*transform, state));
      case css::ValueId::TranslateY:
        return makeFunction(CreateTranslateYTransformFunction(*transform, state));
      case css::ValueId::TranslateZ:
        return makeFunction(CreateTranslateZTransformFunction(*transform, state));
      case css::ValueId::Perspective:
        return makeFunction(CreatePerspectiveTransformFunction(*transform, state));
      default: break;
    }

    krys_unreachable();
  }

  Ref<css::Value> CSSValueCreation<TransformFunction>::operator()(css::ValuePool &pool,
                                                                  const RenderStyle &style,
                                                                  const TransformFunction &value) noexcept
  {
    auto translateLength = [&](const auto &length) -> Ref<css::Value>
    {
      if (length.IsKnownZero())
      {
        return CreateCSSValue(pool, style, Length<> {0_css_px});
      }
      else
      {
        return CreateCSSValue(pool, style, length);
      }
    };

    auto includeLength = [](const auto &length) -> bool
    {
      return !length.IsKnownZero() || length.IsPercent();
    };

    auto &function = value.GetFunction();
    switch (function.GetType())
    {
      case TransformFunctionType::TranslateX:
        return css::FunctionValue::Create(
          css::ValueId::TranslateX, translateLength(Downcast<TranslateTransformFunction>(function).X()));
      case TransformFunctionType::TranslateY:
        return css::FunctionValue::Create(
          css::ValueId::TranslateY, translateLength(Downcast<TranslateTransformFunction>(function).Y()));
      case TransformFunctionType::TranslateZ:
        return css::FunctionValue::Create(
          css::ValueId::TranslateZ, translateLength(Downcast<TranslateTransformFunction>(function).Z()));
      case TransformFunctionType::Translate:
      case TransformFunctionType::Translate3D:
      {
        auto &translate = Downcast<TranslateTransformFunction>(function);
        if (!translate.Is3DOperation())
        {
          if (!includeLength(translate.Y()))
          {
            return css::FunctionValue::Create(css::ValueId::Translate, translateLength(translate.X()));
          }
          return css::FunctionValue::Create(css::ValueId::Translate, translateLength(translate.X()),
                                            translateLength(translate.Y()));
        }
        return css::FunctionValue::Create(css::ValueId::Translate3d, translateLength(translate.X()),
                                          translateLength(translate.Y()), translateLength(translate.Z()));
      }
      case TransformFunctionType::ScaleX:
        return css::FunctionValue::Create(
          css::ValueId::ScaleX, CreateCSSValue(pool, style, Downcast<ScaleTransformFunction>(function).X()));
      case TransformFunctionType::ScaleY:
        return css::FunctionValue::Create(
          css::ValueId::ScaleY, CreateCSSValue(pool, style, Downcast<ScaleTransformFunction>(function).Y()));
      case TransformFunctionType::ScaleZ:
        return css::FunctionValue::Create(
          css::ValueId::ScaleZ, CreateCSSValue(pool, style, Downcast<ScaleTransformFunction>(function).Z()));
      case TransformFunctionType::Scale:
      case TransformFunctionType::Scale3D:
      {
        auto &scale = Downcast<ScaleTransformFunction>(function);
        if (!scale.Is3DOperation())
        {
          if (scale.X() == scale.Y())
          {
            return css::FunctionValue::Create(css::ValueId::Scale, CreateCSSValue(pool, style, scale.X()));
          }
          return css::FunctionValue::Create(css::ValueId::Scale, CreateCSSValue(pool, style, scale.X()),
                                            CreateCSSValue(pool, style, scale.Y()));
        }
        return css::FunctionValue::Create(css::ValueId::Scale3d, CreateCSSValue(pool, style, scale.X()),
                                          CreateCSSValue(pool, style, scale.Y()),
                                          CreateCSSValue(pool, style, scale.Z()));
      }
      case TransformFunctionType::RotateX:
        return css::FunctionValue::Create(
          css::ValueId::RotateX,
          CreateCSSValue(pool, style, Downcast<RotateTransformFunction>(function).GetAngle()));
      case TransformFunctionType::RotateY:
        return css::FunctionValue::Create(
          css::ValueId::RotateY,
          CreateCSSValue(pool, style, Downcast<RotateTransformFunction>(function).GetAngle()));
      case TransformFunctionType::RotateZ:
        return css::FunctionValue::Create(
          css::ValueId::RotateZ,
          CreateCSSValue(pool, style, Downcast<RotateTransformFunction>(function).GetAngle()));
      case TransformFunctionType::Rotate:
        return css::FunctionValue::Create(
          css::ValueId::Rotate,
          CreateCSSValue(pool, style, Downcast<RotateTransformFunction>(function).GetAngle()));
      case TransformFunctionType::Rotate3D:
      {
        auto &rotate = Downcast<RotateTransformFunction>(function);
        return css::FunctionValue::Create(css::ValueId::Rotate3d, CreateCSSValue(pool, style, rotate.X()),
                                          CreateCSSValue(pool, style, rotate.Y()),
                                          CreateCSSValue(pool, style, rotate.Z()),
                                          CreateCSSValue(pool, style, rotate.GetAngle()));
      }
      case TransformFunctionType::SkewX:
        return css::FunctionValue::Create(
          css::ValueId::SkewX,
          CreateCSSValue(pool, style, Downcast<SkewTransformFunction>(function).AngleX()));
      case TransformFunctionType::SkewY:
        return css::FunctionValue::Create(
          css::ValueId::SkewY,
          CreateCSSValue(pool, style, Downcast<SkewTransformFunction>(function).AngleY()));
      case TransformFunctionType::Skew:
      {
        auto &skew = Downcast<SkewTransformFunction>(function);
        if (skew.AngleY().IsZero())
        {
          return css::FunctionValue::Create(css::ValueId::Skew, CreateCSSValue(pool, style, skew.AngleX()));
        }
        return css::FunctionValue::Create(css::ValueId::Skew, CreateCSSValue(pool, style, skew.AngleX()),
                                          CreateCSSValue(pool, style, skew.AngleY()));
      }
      case TransformFunctionType::Perspective:
        return css::FunctionValue::Create(
          css::ValueId::Perspective,
          CreateCSSValue(pool, style, Downcast<PerspectiveTransformFunction>(function).GetPerspective()));
      case TransformFunctionType::Matrix:
      case TransformFunctionType::Matrix3D:
      {
        TransformationMatrix transform;
        function.Apply(transform, {});
        return CreateCSSValue(pool, style, transform);
      }
    }

    krys_unreachable();
  }

  Ref<css::Value>
    CSSValueCreation<TransformationMatrix>::operator()(css::ValuePool &, const RenderStyle &style,
                                                       const TransformationMatrix &transform) noexcept
  {
    auto zoom = style.UsedZoom();
    if (transform.IsAffine())
    {
      double values[] = {transform.A(), transform.B(),        transform.C(),
                         transform.D(), transform.E() / zoom, transform.F() / zoom};
      css::ValueListBuilder arguments;
      for (auto value : values)
      {
        arguments.push_back(css::PrimitiveValue::Create(value));
      }
      return css::FunctionValue::Create(css::ValueId::Matrix, krys::move(arguments));
    }

    double values[] = {
      transform.M11(),        transform.M12(),        transform.M13(),        transform.M14() * zoom,
      transform.M21(),        transform.M22(),        transform.M23(),        transform.M24() * zoom,
      transform.M31(),        transform.M32(),        transform.M33(),        transform.M34() * zoom,
      transform.M41() / zoom, transform.M42() / zoom, transform.M43() / zoom, transform.M44()};
    css::ValueListBuilder arguments;
    for (auto value : values)
    {
      arguments.push_back(css::PrimitiveValue::Create(value));
    }
    return css::FunctionValue::Create(css::ValueId::Matrix3d, krys::move(arguments));
  }

  // MARK: - Serialization

  void Serialiser<TransformFunction>::operator()(StringBuilder &builder,
                                                 const css::SerialisationContext &context,
                                                 const RenderStyle &style,
                                                 const TransformFunction &value) noexcept
  {
    auto translateLength = [&](const auto &length)
    {
      if (length.IsKnownZero())
      {
        Serialise(builder, context, style, Length<> {0_css_px});
      }
      else
      {
        Serialise(builder, context, style, length);
      }
    };

    auto includeLength = [](const auto &length) -> bool
    {
      return !length.IsKnownZero() || length.IsPercent();
    };

    auto &function = value.GetFunction();
    switch (function.GetType())
    {
      case TransformFunctionType::TranslateX:
        builder.Append(css::ToString(css::ValueId::TranslateX));
        builder.Append(u8'(');
        translateLength(Downcast<TranslateTransformFunction>(function).X());
        builder.Append(u8')');
        return;
      case TransformFunctionType::TranslateY:
        builder.Append(css::ToString(css::ValueId::TranslateY));
        builder.Append(u8'(');
        translateLength(Downcast<TranslateTransformFunction>(function).Y());
        builder.Append(u8')');
        return;
      case TransformFunctionType::TranslateZ:
        builder.Append(css::ToString(css::ValueId::TranslateZ));
        builder.Append(u8'(');
        translateLength(Downcast<TranslateTransformFunction>(function).Z());
        builder.Append(u8')');
        return;
      case TransformFunctionType::Translate:
      case TransformFunctionType::Translate3D:
      {
        auto &translate = Downcast<TranslateTransformFunction>(function);
        if (!translate.Is3DOperation())
        {
          if (!includeLength(translate.Y()))
          {
            builder.Append(css::ToString(css::ValueId::Translate));
            builder.Append(u8'(');
            translateLength(translate.X());
            builder.Append(u8')');
            return;
          }
          builder.Append(css::ToString(css::ValueId::Translate));
          builder.Append(u8'(');
          translateLength(translate.X());
          builder.Append(u8", ");
          translateLength(translate.Y());
          builder.Append(u8')');
          return;
        }
        builder.Append(css::ToString(css::ValueId::Translate3d));
        builder.Append(u8'(');
        translateLength(translate.X());
        builder.Append(u8", ");
        translateLength(translate.Y());
        builder.Append(u8", ");
        translateLength(translate.Z());
        builder.Append(u8')');
        return;
      }
      case TransformFunctionType::ScaleX:
        builder.Append(css::ToString(css::ValueId::ScaleX));
        builder.Append(u8'(');
        Serialise(builder, context, style, Downcast<ScaleTransformFunction>(function).X());
        builder.Append(u8')');
        return;
      case TransformFunctionType::ScaleY:
        builder.Append(css::ToString(css::ValueId::ScaleY));
        builder.Append(u8'(');
        Serialise(builder, context, style, Downcast<ScaleTransformFunction>(function).Y());
        builder.Append(u8')');
        return;
      case TransformFunctionType::ScaleZ:
        builder.Append(css::ToString(css::ValueId::ScaleZ));
        builder.Append(u8'(');
        Serialise(builder, context, style, Downcast<ScaleTransformFunction>(function).Z());
        builder.Append(u8')');
        return;
      case TransformFunctionType::Scale:
      case TransformFunctionType::Scale3D:
      {
        auto &scale = Downcast<ScaleTransformFunction>(function);
        if (!scale.Is3DOperation())
        {
          if (scale.X() == scale.Y())
          {
            builder.Append(css::ToString(css::ValueId::Scale));
            builder.Append(u8'(');
            Serialise(builder, context, style, scale.X());
            builder.Append(u8')');
            return;
          }
          builder.Append(css::ToString(css::ValueId::Scale));
          builder.Append(u8'(');
          Serialise(builder, context, style, scale.X());
          builder.Append(u8", ");
          Serialise(builder, context, style, scale.Y());
          builder.Append(u8')');
          return;
        }
        builder.Append(css::ToString(css::ValueId::Scale3d));
        builder.Append(u8'(');
        Serialise(builder, context, style, scale.X());
        builder.Append(u8", ");
        Serialise(builder, context, style, scale.Y());
        builder.Append(u8", ");
        Serialise(builder, context, style, scale.Z());
        builder.Append(u8')');
        return;
      }
      case TransformFunctionType::RotateX:
        builder.Append(css::ToString(css::ValueId::RotateX));
        builder.Append(u8'(');
        Serialise(builder, context, style, Downcast<RotateTransformFunction>(function).GetAngle());
        builder.Append(u8')');
        return;
      case TransformFunctionType::RotateY:
        builder.Append(css::ToString(css::ValueId::RotateY));
        builder.Append(u8'(');
        Serialise(builder, context, style, Downcast<RotateTransformFunction>(function).GetAngle());
        builder.Append(u8')');
        return;
      case TransformFunctionType::RotateZ:
        builder.Append(css::ToString(css::ValueId::RotateZ));
        builder.Append(u8'(');
        Serialise(builder, context, style, Downcast<RotateTransformFunction>(function).GetAngle());
        builder.Append(u8')');
        return;
      case TransformFunctionType::Rotate:
        builder.Append(css::ToString(css::ValueId::Rotate));
        builder.Append(u8'(');
        Serialise(builder, context, style, Downcast<RotateTransformFunction>(function).GetAngle());
        builder.Append(u8')');
        return;
      case TransformFunctionType::Rotate3D:
      {
        auto &rotate = Downcast<RotateTransformFunction>(function);
        builder.Append(css::ToString(css::ValueId::Rotate3d));
        builder.Append(u8'(');
        Serialise(builder, context, style, rotate.X());
        builder.Append(u8", ");
        Serialise(builder, context, style, rotate.Y());
        builder.Append(u8", ");
        Serialise(builder, context, style, rotate.Z());
        builder.Append(u8", ");
        Serialise(builder, context, style, rotate.GetAngle());
        builder.Append(u8')');
        return;
      }
      case TransformFunctionType::SkewX:
        builder.Append(css::ToString(css::ValueId::SkewX));
        builder.Append(u8'(');
        Serialise(builder, context, style, Downcast<SkewTransformFunction>(function).AngleX());
        builder.Append(u8')');
        return;
      case TransformFunctionType::SkewY:
        builder.Append(css::ToString(css::ValueId::SkewY));
        builder.Append(u8'(');
        Serialise(builder, context, style, Downcast<SkewTransformFunction>(function).AngleY());
        builder.Append(u8')');
        return;
      case TransformFunctionType::Skew:
      {
        auto &skew = Downcast<SkewTransformFunction>(function);
        if (skew.AngleY().IsZero())
        {
          builder.Append(css::ToString(css::ValueId::Skew));
          builder.Append(u8'(');
          Serialise(builder, context, style, skew.AngleX());
          builder.Append(u8')');
          return;
        }
        builder.Append(css::ToString(css::ValueId::Skew));
        builder.Append(u8'(');
        Serialise(builder, context, style, skew.AngleX());
        builder.Append(u8", ");
        Serialise(builder, context, style, skew.AngleY());
        builder.Append(u8')');
        return;
      }
      case TransformFunctionType::Perspective:
        builder.Append(css::ToString(css::ValueId::Perspective));
        builder.Append(u8'(');
        Serialise(builder, context, style, Downcast<PerspectiveTransformFunction>(function).GetPerspective());
        builder.Append(u8')');
        return;
      case TransformFunctionType::Matrix:
      case TransformFunctionType::Matrix3D:
      {
        TransformationMatrix transform;
        function.Apply(transform, {});
        Serialise(builder, context, style, transform);
        return;
      }
    }

    krys_unreachable();
  }

  namespace
  {
    template <size_t N>
    void SerialiseMatrix(StringBuilder &builder, const css::SerialisationContext &context, css::ValueId name,
                         const std::array<double, N> &values) noexcept
    {
      builder.Append(css::ToString(name));
      builder.Append(u8'(');
      for (size_t i = 0; i < N; ++i)
      {
        if (i)
        {
          builder.Append(u8", ");
        }
        css::Serialise(builder, context, css::NumberRaw<> {values[i]});
      }
      builder.Append(u8')');
    }
  }

  void Serialiser<TransformationMatrix>::operator()(StringBuilder &builder,
                                                    const css::SerialisationContext &context,
                                                    const RenderStyle &style,
                                                    const TransformationMatrix &transform) noexcept
  {
    auto zoom = style.UsedZoom();
    if (transform.IsAffine())
    {
      std::array values {transform.A(), transform.B(),        transform.C(),
                         transform.D(), transform.E() / zoom, transform.F() / zoom};
      SerialiseMatrix(builder, context, css::ValueId::Matrix, values);
      return;
    }

    std::array values {
      transform.M11(),        transform.M12(),        transform.M13(),        transform.M14() * zoom,
      transform.M21(),        transform.M22(),        transform.M23(),        transform.M24() * zoom,
      transform.M31(),        transform.M32(),        transform.M33(),        transform.M34() * zoom,
      transform.M41() / zoom, transform.M42() / zoom, transform.M43() / zoom, transform.M44()};
    SerialiseMatrix(builder, context, css::ValueId::Matrix3d, values);
  }

  // MARK: - Blending

  TransformFunction Blending<TransformFunction>::Blend(const TransformFunction &from,
                                                       const TransformFunction &to,
                                                       const BlendingContext &context) noexcept
  {
    return TransformFunction {to.GetFunction().Blend(&from.GetFunction(), context)};
  }

  // MARK: - Platform

  Ref<TransformOperation> ToPlatform<TransformFunction>::operator()(const TransformFunction &value,
                                                                    const FloatSize &size) noexcept
  {
    return value.Value->ToPlatform(size);
  }
}

#include "Krystal.Booey/Style/Values/FilterEffects/AppleColorFilter.hpp"

#include "Krystal.Booey/CSS/Values/AppleColorFilterValue.hpp"
#include "Krystal.Booey/Graphics/Color/ColorConversion.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/AppleInvertLightnessFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/BrightnessFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/ContrastFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/FilterInterpolation.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/GrayscaleFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/HueRotateFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/InvertFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/OpacityFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/SaturateFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/SepiaFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::style
{
  const AppleColorFilter &AppleColorFilter::None() noexcept
  {
    static NeverDestroyed<AppleColorFilter> value {css::keywords::None {}};
    return value.get();
  }

  bool AppleColorFilter::TransformColor(boo::Color &color) const noexcept
  {
    if (IsNone() || !color.IsValid())
    {
      return false;
    }
    // Color filter does not apply to semantic CSS colors (like "Windowframe").
    if (color.IsSemantic())
    {
      return false;
    }

    auto sRGBAColor = color.ToColorTypeLossy<SRGBA<float>>();

    for (const auto &value : *this)
    {
      if (!value->TransformColor(sRGBAColor))
      {
        return false;
      }
    }

    color = ConvertColor<SRGBA<uint8>>(sRGBAColor);
    return true;
  }

  bool AppleColorFilter::InverseTransformColor(boo::Color &color) const noexcept
  {
    if (IsNone() || !color.IsValid())
    {
      return false;
    }
    // Color filter does not apply to semantic CSS colors (like "Windowframe").
    if (color.IsSemantic())
    {
      return false;
    }

    auto sRGBAColor = color.ToColorTypeLossy<SRGBA<float>>();

    for (const auto &value : *this)
    {
      if (!value->InverseTransformColor(sRGBAColor))
      {
        return false;
      }
    }

    color = ConvertColor<SRGBA<uint8>>(sRGBAColor);
    return true;
  }

  // MARK: - Conversions

  // MARK: (AppleColorFilterValue)

  css::AppleColorFilterData ToCSS<AppleColorFilterValue>::operator()(const AppleColorFilterValue &value,
                                                                     const RenderStyle &style) noexcept
  {
    const auto &op = value.Value;
    switch (op->GetType())
    {
      case FilterOperation::Type::AppleInvertLightness:
        return css::AppleColorFilterData {css::AppleInvertLightnessFunction {
          ToCSSAppleInvertLightness(Downcast<InvertLightnessFilterOperation>(*op), style)}};
      case FilterOperation::Type::Grayscale:
        return css::AppleColorFilterData {
          css::GrayscaleFunction {ToCSSGrayscale(Downcast<BasicColorMatrixFilterOperation>(*op), style)}};
      case FilterOperation::Type::Sepia:
        return css::AppleColorFilterData {
          css::SepiaFunction {ToCSSSepia(Downcast<BasicColorMatrixFilterOperation>(*op), style)}};
      case FilterOperation::Type::Saturate:
        return css::AppleColorFilterData {
          css::SaturateFunction {ToCSSSaturate(Downcast<BasicColorMatrixFilterOperation>(*op), style)}};
      case FilterOperation::Type::HueRotate:
        return css::AppleColorFilterData {
          css::HueRotateFunction {ToCSSHueRotate(Downcast<BasicColorMatrixFilterOperation>(*op), style)}};
      case FilterOperation::Type::Invert:
        return css::AppleColorFilterData {
          css::InvertFunction {ToCSSInvert(Downcast<BasicComponentTransferFilterOperation>(*op), style)}};
      case FilterOperation::Type::Opacity:
        return css::AppleColorFilterData {
          css::OpacityFunction {ToCSSOpacity(Downcast<BasicComponentTransferFilterOperation>(*op), style)}};
      case FilterOperation::Type::Brightness:
        return css::AppleColorFilterData {css::BrightnessFunction {
          ToCSSBrightness(Downcast<BasicComponentTransferFilterOperation>(*op), style)}};
      case FilterOperation::Type::Contrast:
        return css::AppleColorFilterData {
          css::ContrastFunction {ToCSSContrast(Downcast<BasicComponentTransferFilterOperation>(*op), style)}};
      default: break;
    }
    krys_unreachable();
  }

  AppleColorFilterValue ToStyle<css::AppleColorFilterData>::operator()(const css::AppleColorFilterData &value,
                                                                       const BuilderState &state) noexcept
  {
    return krys::SwitchOn(value, [&](const auto &function) -> AppleColorFilterValue
                          { return AppleColorFilterValue {CreateFilterOperation(function, state)}; });
  }

  // MARK: (AppleColorFilter)

  css::AppleColorFilter ToCSS<AppleColorFilter>::operator()(const AppleColorFilter &value,
                                                            const RenderStyle &style) noexcept
  {
    return krys::SwitchOn(
      value, [&](css::keywords::None keyword) -> css::AppleColorFilter { return keyword; },
      [&](const AppleColorFilterValueList &list) -> css::AppleColorFilter
      {
        return css::AppleColorFilterValueList::map(list, [&](const AppleColorFilterValue &value)
                                                   { return ConvertToCSS(value, style); });
      });
  }

  AppleColorFilter ToStyle<css::AppleColorFilter>::operator()(const css::AppleColorFilter &value,
                                                              const BuilderState &state) noexcept
  {
    return krys::SwitchOn(
      value, [&](css::keywords::None keyword) -> AppleColorFilter { return keyword; },
      [&](const css::AppleColorFilterValueList &list) -> AppleColorFilter
      {
        return AppleColorFilterValueList::map(list, [&](const css::AppleColorFilterData &value)
                                              { return ConvertToStyle(value, state); });
      });
  }

  AppleColorFilter CSSValueConversion<AppleColorFilter>::operator()(BuilderState &state,
                                                                    const css::Value &value) noexcept
  {
    if (value.ValueId() == css::ValueId::None)
    {
      return css::keywords::None {};
    }

    const auto *filter = RequiredDowncast<css::AppleColorFilterValue>(state, value);
    if (!filter)
    {
      return css::keywords::None {};
    }

    return ConvertToStyle(filter->GetFilter(), state);
  }

  Ref<css::Value> CSSValueCreation<AppleColorFilter>::operator()(css::ValuePool &, const RenderStyle &style,
                                                                 const AppleColorFilter &value) noexcept
  {
    return css::AppleColorFilterValue::Create(ConvertToCSS(value, style));
  }

  // MARK: - Serialization

  void Serialiser<AppleColorFilter>::operator()(StringBuilder &builder,
                                                const css::SerialisationContext &context,
                                                const RenderStyle &style,
                                                const AppleColorFilter &value) noexcept
  {
    css::Serialise(builder, context, ConvertToCSS(value, style));
  }

  // MARK: - Blending

  bool Blending<AppleColorFilter>::CanBlend(const AppleColorFilter &from, const AppleColorFilter &to,
                                            CompositeOperation compositeOperation) noexcept
  {
    return CanBlendFilterLists(from._value, to._value, compositeOperation);
  }

  AppleColorFilter Blending<AppleColorFilter>::Blend(const AppleColorFilter &from, const AppleColorFilter &to,
                                                     const BlendingContext &context) noexcept
  {
    auto blendedFilterList = BlendFilterLists(from._value, to._value, context);

    if (blendedFilterList.isEmpty())
    {
      return css::keywords::None {};
    }

    return AppleColorFilter {krys::move(blendedFilterList)};
  }

  // MARK: - Platform

  Ref<FilterOperation>
    ToPlatform<AppleColorFilterValue>::operator()(const AppleColorFilterValue &value) noexcept
  {
    return value.Value;
  }

  FilterOperations ToPlatform<AppleColorFilter>::operator()(const AppleColorFilter &value) noexcept
  {
    List<Ref<FilterOperation>> operations;
    for (const auto &filterValue : value)
    {
      operations.push_back(ConvertToPlatform(filterValue));
    }
    return FilterOperations {krys::move(operations)};
  }
}

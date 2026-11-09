/*
 * Copyright (C) 2024-2025 Samuel Weinig <sam@webkit.org>
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/FilterEffects/Filter.hpp"

#include "Krystal.Booey/CSS/Values/FilterValue.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Filters/FEGaussianBlur.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Booey/Rendering/Style/DropShadowFilterOperationWithStyleColor.hpp"
#include "Krystal.Booey/Rendering/Style/ReferenceFilterOperation.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/BlurFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/BrightnessFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/ContrastFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/DropShadowFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/FilterInterpolation.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/FilterReference.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/GrayscaleFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/HueRotateFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/InvertFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/OpacityFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/SaturateFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/SepiaFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  bool Filter::HasReferenceFilter() const noexcept
  {
    return HasFilterOfType<FilterOperation::Type::Reference>();
  }

  bool Filter::IsReferenceFilter() const noexcept
  {
    return size() == 1 && first()->GetType() == FilterOperation::Type::Reference;
  }

  bool Filter::HasFilterThatRequiresRepaintForCurrentColorChange() const noexcept
  {
    return std::ranges::any_of(*this,
                               [](const auto &op) { return op->RequiresRepaintForCurrentColorChange(); });
  }

  bool Filter::HasFilterThatAffectsOpacity() const noexcept
  {
    return std::ranges::any_of(*this, [](const auto &op) { return op->AffectsOpacity(); });
  }

  bool Filter::HasFilterThatMovesPixels() const noexcept
  {
    return std::ranges::any_of(*this, [](const auto &op) { return op->MovesPixels(); });
  }

  bool Filter::HasFilterThatShouldBeRestrictedBySecurityOrigin() const noexcept
  {
    return std::ranges::any_of(*this,
                               [](const auto &op) { return op->ShouldBeRestrictedBySecurityOrigin(); });
  }

  IntOutsets Filter::Outsets() const noexcept
  {
    IntOutsets totalOutsets;
    for (const auto &value : *this)
    {
      const auto &operation = *value.Value;
      switch (operation.GetType())
      {
        case FilterOperation::Type::Blur:
        {
          const auto &blurOperation = Downcast<BlurFilterOperation>(operation);
          float stdDeviation = blurOperation.StdDeviation();
          IntSize outsetSize = FEGaussianBlur::CalculateOutsetSize({stdDeviation, stdDeviation});
          IntOutsets outsets {outsetSize.Height(), outsetSize.Width(), outsetSize.Height(),
                              outsetSize.Width()};
          totalOutsets += outsets;
          break;
        }
        case FilterOperation::Type::DropShadow:
        case FilterOperation::Type::DropShadowWithStyleColor:
        {
          const auto &dropShadowOperation = Downcast<DropShadowFilterOperationBase>(operation);
          auto stdDeviation = static_cast<float>(dropShadowOperation.StdDeviation());
          IntSize outsetSize = FEGaussianBlur::CalculateOutsetSize({stdDeviation, stdDeviation});

          int top = std::max(0, outsetSize.Height() - dropShadowOperation.Y());
          int right = std::max(0, outsetSize.Width() + dropShadowOperation.X());
          int bottom = std::max(0, outsetSize.Height() + dropShadowOperation.Y());
          int left = std::max(0, outsetSize.Width() - dropShadowOperation.X());

          auto outsets = IntOutsets {top, right, bottom, left};
          totalOutsets += outsets;
          break;
        }
        case FilterOperation::Type::Reference: krys_debug_assert_msg(false, "Unreachable"); break;
        default:                               break;
      }
    }
    return totalOutsets;
  }

  // MARK: - Conversions

  // MARK: (FilterValue)

  css::FilterData ToCSS<FilterValue>::operator()(const FilterValue &value, const RenderStyle &style) noexcept
  {
    const auto &op = value.Value;
    switch (op->GetType())
    {
      case FilterOperation::Type::Reference:
        return css::FilterData {
          css::FilterReference {ToCSSFilterReference(Downcast<ReferenceFilterOperation>(*op), style)}};
      case FilterOperation::Type::Grayscale:
        return css::FilterData {
          css::GrayscaleFunction {ToCSSGrayscale(Downcast<BasicColorMatrixFilterOperation>(*op), style)}};
      case FilterOperation::Type::Sepia:
        return css::FilterData {
          css::SepiaFunction {ToCSSSepia(Downcast<BasicColorMatrixFilterOperation>(*op), style)}};
      case FilterOperation::Type::Saturate:
        return css::FilterData {
          css::SaturateFunction {ToCSSSaturate(Downcast<BasicColorMatrixFilterOperation>(*op), style)}};
      case FilterOperation::Type::HueRotate:
        return css::FilterData {
          css::HueRotateFunction {ToCSSHueRotate(Downcast<BasicColorMatrixFilterOperation>(*op), style)}};
      case FilterOperation::Type::Invert:
        return css::FilterData {
          css::InvertFunction {ToCSSInvert(Downcast<BasicComponentTransferFilterOperation>(*op), style)}};
      case FilterOperation::Type::Opacity:
        return css::FilterData {
          css::OpacityFunction {ToCSSOpacity(Downcast<BasicComponentTransferFilterOperation>(*op), style)}};
      case FilterOperation::Type::Brightness:
        return css::FilterData {css::BrightnessFunction {
          ToCSSBrightness(Downcast<BasicComponentTransferFilterOperation>(*op), style)}};
      case FilterOperation::Type::Contrast:
        return css::FilterData {
          css::ContrastFunction {ToCSSContrast(Downcast<BasicComponentTransferFilterOperation>(*op), style)}};
      case FilterOperation::Type::Blur:
        return css::FilterData {css::BlurFunction {ToCSSBlur(Downcast<BlurFilterOperation>(*op), style)}};
      case FilterOperation::Type::DropShadowWithStyleColor:
        return css::FilterData {css::DropShadowFunction {
          ToCSSDropShadow(Downcast<DropShadowFilterOperationWithStyleColor>(*op), style)}};
      default: break;
    }
    krys_unreachable();
  }

  FilterValue ToStyle<css::FilterData>::operator()(const css::FilterData &value,
                                                   const BuilderState &state) noexcept
  {
    return krys::SwitchOn(value, [&](const auto &function) -> FilterValue
                          { return FilterValue {CreateFilterOperation(function, state)}; });
  }

  // MARK: (Filter)

  css::Filter ToCSS<Filter>::operator()(const Filter &value, const RenderStyle &style) noexcept
  {
    return krys::SwitchOn(
      value, [&](css::keywords::None keyword) -> css::Filter { return keyword; },
      [&](const FilterValueList &list) -> css::Filter
      {
        return css::FilterValueList::map(list, [&](const FilterValue &value)
                                         { return ConvertToCSS(value, style); });
      });
  }

  Filter ToStyle<css::Filter>::operator()(const css::Filter &value, const BuilderState &state) noexcept
  {
    return krys::SwitchOn(
      value, [&](css::keywords::None keyword) -> Filter { return keyword; },
      [&](const css::FilterValueList &list) -> Filter
      {
        return FilterValueList::map(list, [&](const css::FilterData &value)
                                    { return ConvertToStyle(value, state); });
      });
  }

  Filter CSSValueConversion<Filter>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (value.ValueId() == css::ValueId::None)
    {
      return css::keywords::None {};
    }

    const auto *filter = RequiredDowncast<css::FilterValue>(state, value);
    if (!filter)
    {
      return css::keywords::None {};
    }

    return ConvertToStyle(filter->GetFilter(), state);
  }

  Ref<css::Value> CSSValueCreation<Filter>::operator()(css::ValuePool &, const RenderStyle &style,
                                                       const Filter &value) noexcept
  {
    return css::FilterValue::Create(ConvertToCSS(value, style));
  }

  // MARK: - Serialization

  void Serialiser<Filter>::operator()(StringBuilder &builder, const css::SerialisationContext &context,
                                      const RenderStyle &style, const Filter &value) noexcept
  {
    css::Serialise(builder, context, ConvertToCSS(value, style));
  }

  // MARK: - Blending

  bool Blending<Filter>::CanBlend(const Filter &from, const Filter &to,
                                  CompositeOperation compositeOperation) noexcept
  {
    return CanBlendFilterLists(from._value, to._value, compositeOperation);
  }

  Filter Blending<Filter>::Blend(const Filter &from, const Filter &to,
                                 const BlendingContext &context) noexcept
  {
    auto blendedFilterList = BlendFilterLists(from._value, to._value, context);

    if (blendedFilterList.isEmpty())
    {
      return css::keywords::None {};
    }

    return Filter {krys::move(blendedFilterList)};
  }

  // MARK: - Platform

  Ref<FilterOperation> ToPlatform<FilterValue>::operator()(const FilterValue &value) noexcept
  {
    return value.Value;
  }

  FilterOperations ToPlatform<Filter>::operator()(const Filter &value) noexcept
  {
    List<Ref<FilterOperation>> operations;
    for (const auto &filterValue : value)
    {
      operations.push_back(ConvertToPlatform(filterValue));
    }
    return FilterOperations {krys::move(operations)};
  }
}

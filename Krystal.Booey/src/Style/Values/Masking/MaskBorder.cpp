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

#include "Krystal.Booey/Style/Values/Masking/MaskBorder.hpp"
#include "Krystal.Booey/CSS/Values/BorderImageSliceValue.hpp"
#include "Krystal.Booey/CSS/Values/BorderImageWidthValue.hpp"
#include "Krystal.Booey/CSS/Values/Borders/BorderImage.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  MaskBorder::MaskBorder() noexcept
      : Source {ComputedStyle::InitialMaskBorderSource()}, Slice {ComputedStyle::InitialMaskBorderSlice()},
        Width {ComputedStyle::InitialMaskBorderWidth()}, Outset {ComputedStyle::InitialMaskBorderOutset()},
        Repeat {ComputedStyle::InitialMaskBorderRepeat()}
  {
  }

  MaskBorder::MaskBorder(MaskBorderSource &&source, MaskBorderSlice &&slice, MaskBorderWidth &&width,
                         MaskBorderOutset &&outset, MaskBorderRepeat &&repeat) noexcept
      : Source {krys::move(source)}, Slice {krys::move(slice)}, Width {krys::move(width)},
        Outset {krys::move(outset)}, Repeat {krys::move(repeat)}
  {
  }

  // MARK: - Conversion

  MaskBorder
    CSSValueConversion<MaskBorder>::operator()(BuilderState &state, const css::Value &value,
                                               MaskBorderSliceOverride maskBorderSliceOverride) noexcept
  {
    MaskBorder result {};

    const auto *maskBorder = RequiredDowncast<css::ValueList>(state, value);
    if (!maskBorder)
    {
      return result;
    }

    for (const auto &current : *maskBorder)
    {
      if (current.IsImage())
      {
        result.Source = ConvertCSSValueToStyle<MaskBorderSource>(state, current);
      }
      else if (const auto *slice = DynamicDowncast<css::BorderImageSliceValue>(current))
      {
        result.Slice = ConvertCSSValueToStyle<MaskBorderSlice>(state, *slice);
      }
      else if (const auto *slashList = DynamicDowncast<css::ValueList>(current))
      {
        if (const auto *slashSlice = DynamicDowncast<css::BorderImageSliceValue>(slashList->Item(0)))
        {
          result.Slice = ConvertCSSValueToStyle<MaskBorderSlice>(state, *slashSlice);
        }
        if (const auto *width = DynamicDowncast<css::BorderImageWidthValue>(slashList->Item(1)))
        {
          result.Width = ConvertCSSValueToStyle<MaskBorderWidth>(state, *width);
        }
        if (const auto *outset = slashList->Item(2))
        {
          result.Outset = ConvertCSSValueToStyle<MaskBorderOutset>(state, *outset);
        }
      }
      else if (current.IsValuePairValue())
      {
        result.Repeat = ConvertCSSValueToStyle<MaskBorderRepeat>(state, current);
      }
    }

    if (maskBorderSliceOverride == MaskBorderSliceOverride::AlwaysFill)
    {
      result.Slice.Fill = css::keywords::Fill {};
    }

    return result;
  }

  Ref<css::Value> CSSValueCreation<MaskBorder>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                           const MaskBorder &value) noexcept
  {
    return css::CreateBorderImageValue({
      .Source = CreateCSSValue(pool, style, value.Source),
      .Slice = CreateCSSValue(pool, style, value.Slice),
      .Width = CreateCSSValue(pool, style, value.Width),
      .Outset = CreateCSSValue(pool, style, value.Outset),
      .Repeat = CreateCSSValue(pool, style, value.Repeat),
    });
  }

  // MARK: - Serialization

  void Serialiser<MaskBorder>::operator()(StringBuilder &builder, const css::SerialisationContext &context,
                                          const RenderStyle &style, const MaskBorder &value) noexcept
  {
    if (value.Source.IsNone())
    {
      Serialise(builder, context, style, value.Source);
      return;
    }

    // FIXME: Omit values that have their initial value.

    Serialise(builder, context, style, value.Source);
    builder.Append(u8' ');
    Serialise(builder, context, style, value.Slice);
    builder.Append(u8" / ");
    Serialise(builder, context, style, value.Width);
    builder.Append(u8" / ");
    Serialise(builder, context, style, value.Outset);
    builder.Append(u8' ');
    Serialise(builder, context, style, value.Repeat);
  }
}

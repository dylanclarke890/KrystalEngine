#include "Krystal.Booey/Style/Values/Masking/MaskBorder.hpp"
#include "Krystal.Booey/CSS/Values/BorderImageSliceValue.hpp"
#include "Krystal.Booey/CSS/Values/BorderImageWidthValue.hpp"
#include "Krystal.Booey/CSS/Values/Borders/BorderImage.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style
{
  MaskBorder::MaskBorder() noexcept
      // FIXME: Use `ComputedStyle`'s initial values once the generated storage is ported.
      : Source {css::keywords::None {}}, Slice {MaskBorderSlice::Value::Number {0}},
        Width {css::keywords::Auto {}}, Outset {MaskBorderOutset::Value::Number {0}},
        Repeat {css::keywords::Stretch {}}
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

  void Serialiser<MaskBorder>::operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                                          const RenderStyle &style, const MaskBorder &value) noexcept
  {
    if (value.Source.IsNone())
    {
      Serialise(builder, context, style, value.Source);
      return;
    }

    // FIXME: Omit values that have their initial value.

    Serialise(builder, context, style, value.Source);
    builder.push_back(u8' ');
    Serialise(builder, context, style, value.Slice);
    builder.append(u8" / ");
    Serialise(builder, context, style, value.Width);
    builder.append(u8" / ");
    Serialise(builder, context, style, value.Outset);
    builder.push_back(u8' ');
    Serialise(builder, context, style, value.Repeat);
  }
}

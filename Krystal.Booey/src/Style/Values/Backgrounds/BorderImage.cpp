#include "Krystal.Booey/Style/Values/Backgrounds/BorderImage.hpp"
#include "Krystal.Booey/CSS/Values/Borders/BorderImage.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  BorderImage::BorderImage() noexcept
      // FIXME: Use `ComputedStyle`'s initial values once the generated storage is ported.
      : Source {css::keywords::None {}}, Slice {BorderImageSlice::Value::Percentage {100}},
        Width {BorderImageWidth::Value::Number {1}}, Outset {BorderImageOutset::Value::Number {0}},
        Repeat {css::keywords::Stretch {}}
  {
  }

  BorderImage::BorderImage(BorderImageSource &&source, BorderImageSlice &&slice, BorderImageWidth &&width,
                           BorderImageOutset &&outset, BorderImageRepeat &&repeat) noexcept
      : Source {krys::move(source)}, Slice {krys::move(slice)}, Width {krys::move(width)},
        Outset {krys::move(outset)}, Repeat {krys::move(repeat)}
  {
  }

  // MARK: - Conversion

  Ref<css::Value> CSSValueCreation<BorderImage>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                            const BorderImage &value) noexcept
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

  void Serialiser<BorderImage>::operator()(StringBuilder &builder, const css::SerialisationContext &context,
                                           const RenderStyle &style, const BorderImage &value) noexcept
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

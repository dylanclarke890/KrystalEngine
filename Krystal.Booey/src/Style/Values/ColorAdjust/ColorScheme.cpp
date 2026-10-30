#include "Krystal.Booey/Style/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/ColorSchemeValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  EnumSet<boo::ColorScheme> ColorScheme::GetColorScheme() const noexcept
  {
    EnumSet<boo::ColorScheme> result;
    for (const auto &scheme : Schemes)
    {
      if (text::IsASCIICaselessEqual(scheme.Value.View(), css::CSSOMStringView {u8"light"}))
      {
        result.Add(boo::ColorScheme::Light);
      }
      else if (text::IsASCIICaselessEqual(scheme.Value.View(), css::CSSOMStringView {u8"dark"}))
      {
        result.Add(boo::ColorScheme::Dark);
      }
    }
    return result;
  }

  // MARK: - Conversion

  Ref<css::Value> CSSValueCreation<ColorScheme>::operator()(css::ValuePool &, const RenderStyle &style,
                                                            const ColorScheme &value) noexcept
  {
    return css::ColorSchemeValue::Create(ConvertToCSS(value, style));
  }

  ColorScheme CSSValueConversion<ColorScheme>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    auto *colorSchemeValue = RequiredDowncast<css::ColorSchemeValue>(state, value);
    if (!colorSchemeValue)
    {
      return css::keywords::Normal {};
    }
    return ConvertToStyle(colorSchemeValue->GetColorScheme(), state);
  }

  // MARK: - Serialisation

  void Serialiser<ColorScheme>::operator()(StringBuilder &builder, const css::SerialisationContext &context,
                                           const RenderStyle &style, const ColorScheme &value) noexcept
  {
    if (value.IsNormal())
    {
      Serialise(builder, context, style, css::keywords::Normal {});
      return;
    }

    Serialise(builder, context, style, value.Schemes);
    if (value.Only)
    {
      builder.Append(u8" ");
      Serialise(builder, context, style, *value.Only);
    }
  }
}

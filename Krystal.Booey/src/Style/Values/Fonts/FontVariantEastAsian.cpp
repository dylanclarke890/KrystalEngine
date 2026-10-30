#include "Krystal.Booey/Style/Values/Fonts/FontVariantEastAsian.hpp"

#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  FontVariantEastAsian CSSValueConversion<FontVariantEastAsian>::operator()(BuilderState &state,
                                                                            const css::Value &value) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Normal: return css::keywords::Normal {};
        default:                   state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::Normal {};
    }

    auto variant = FontVariantEastAsianVariant::Normal;
    auto width = FontVariantEastAsianWidth::Normal;
    auto ruby = FontVariantEastAsianRuby::Normal;

    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::Jis78:             variant = FontVariantEastAsianVariant::Jis78; break;
        case css::ValueId::Jis83:             variant = FontVariantEastAsianVariant::Jis83; break;
        case css::ValueId::Jis90:             variant = FontVariantEastAsianVariant::Jis90; break;
        case css::ValueId::Jis04:             variant = FontVariantEastAsianVariant::Jis04; break;
        case css::ValueId::Simplified:        variant = FontVariantEastAsianVariant::Simplified; break;
        case css::ValueId::Traditional:       variant = FontVariantEastAsianVariant::Traditional; break;
        case css::ValueId::FullWidth:         width = FontVariantEastAsianWidth::Full; break;
        case css::ValueId::ProportionalWidth: width = FontVariantEastAsianWidth::Proportional; break;
        case css::ValueId::Ruby:              ruby = FontVariantEastAsianRuby::Yes; break;
        default:                              state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }

    return FontVariantEastAsian::Platform {variant, width, ruby};
  }
}

#include "Krystal.Booey/Style/Values/Fonts/FontVariantLigatures.hpp"

#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  FontVariantLigatures CSSValueConversion<FontVariantLigatures>::operator()(BuilderState &state,
                                                                            const css::Value &value) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Normal: return css::keywords::Normal {};
        case css::ValueId::None:   return css::keywords::None {};
        default:                   state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::Normal {};
    }

    using enum boo::FontVariantLigatures;

    auto common = Normal;
    auto discretionary = Normal;
    auto historical = Normal;
    auto contextual = Normal;

    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::NoCommonLigatures:        common = No; break;
        case css::ValueId::CommonLigatures:          common = Yes; break;
        case css::ValueId::NoDiscretionaryLigatures: discretionary = No; break;
        case css::ValueId::DiscretionaryLigatures:   discretionary = Yes; break;
        case css::ValueId::NoHistoricalLigatures:    historical = No; break;
        case css::ValueId::HistoricalLigatures:      historical = Yes; break;
        case css::ValueId::Contextual:               contextual = Yes; break;
        case css::ValueId::NoContextual:             contextual = No; break;
        default:                                     state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }

    return FontVariantLigatures::Platform {common, discretionary, historical, contextual};
  }
}

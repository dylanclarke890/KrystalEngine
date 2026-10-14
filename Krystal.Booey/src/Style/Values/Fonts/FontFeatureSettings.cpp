#include "Krystal.Booey/Style/Values/Fonts/FontFeatureSettings.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Values/FontFeatureValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontOpentypeTag.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  FontFeatureSettings CSSValueConversion<FontFeatureSettings>::operator()(BuilderState &state,
                                                                          const css::Value &value) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (auto valueID = primitiveValue->ValueId(); valueID)
      {
        case css::ValueId::Normal: return css::keywords::Normal {};
        default:
          if (css::PropertyParserHelpers::IsSystemFontShorthand(valueID))
          {
            return css::keywords::Normal {};
          }
          state.SetCurrentPropertyInvalidAtComputedValueTime();
          return css::keywords::Normal {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::FontFeatureValue>(state, value);
    if (!list)
    {
      return css::keywords::Normal {};
    }

    boo::FontFeatureSettings platformSettings;
    for (const auto &setting : *list)
    {
      platformSettings.insert(
        {setting.Tag(), ConvertCSSValueToStyle<FontFeatureSettings::Value>(state, setting.GetValue()).Value});
    }

    return {krys::move(platformSettings)};
  }

  Ref<css::Value> CSSValueCreation<FontFeatureSettings>::operator()(css::ValuePool &pool,
                                                                    const RenderStyle &style,
                                                                    const FontFeatureSettings &value) noexcept
  {
    if (value.Platform().isEmpty())
    {
      return CreateCSSValue(pool, style, css::keywords::Normal {});
    }

    css::ValueListBuilder list;
    for (const auto &setting : value.Platform())
    {
      list.push_back(css::FontFeatureValue::Create(
        setting.Tag(), CreateCSSValue(pool, style, FontFeatureSettings::Value {setting.Value()})));
    }
    return css::ValueList::CreateCommaSeparated(krys::move(list));
  }

  // MARK: - Serialization

  void Serialiser<FontFeatureSettings>::operator()(css::CSSOMString &builder,
                                                   const css::SerialisationContext &context,
                                                   const RenderStyle &style,
                                                   const FontFeatureSettings &value) noexcept
  {
    if (value.Platform().isEmpty())
    {
      Serialise(builder, context, style, css::keywords::Normal {});
      return;
    }

    bool first = true;
    for (const auto &setting : value.Platform())
    {
      if (!first)
      {
        builder.append(u8", ");
      }
      first = false;
      Serialise(builder, context, style, FontOpentypeTag {setting.Tag()});
      if (setting.Value() != 1)
      {
        builder.push_back(u8' ');
        Serialise(builder, context, style, FontFeatureSettings::Value {setting.Value()});
      }
    }
  }
}

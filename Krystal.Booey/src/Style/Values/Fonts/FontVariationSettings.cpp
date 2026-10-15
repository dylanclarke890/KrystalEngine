#include "Krystal.Booey/Style/Values/Fonts/FontVariationSettings.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Values/FontVariationValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontOpentypeTag.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  FontVariationSettings
    CSSValueConversion<FontVariationSettings>::operator()(BuilderState &state,
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

    auto list = RequiredListDowncast<css::ValueList, css::FontVariationValue>(state, value);
    if (!list)
    {
      return css::keywords::Normal {};
    }

    boo::FontVariationSettings platformSettings;
    for (const auto &setting : *list)
    {
      platformSettings.insert(
        {setting.Tag(),
         ConvertCSSValueToStyle<FontVariationSettings::Value>(state, setting.GetValue()).Value});
    }

    return {krys::move(platformSettings)};
  }

  Ref<css::Value>
    CSSValueCreation<FontVariationSettings>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                        const FontVariationSettings &value) noexcept
  {
    if (value.Platform().isEmpty())
    {
      return CreateCSSValue(pool, style, css::keywords::Normal {});
    }

    css::ValueListBuilder list;
    for (const auto &setting : value.Platform())
    {
      list.push_back(css::FontVariationValue::Create(
        setting.Tag(), CreateCSSValue(pool, style, FontVariationSettings::Value {setting.Value()})));
    }
    return css::ValueList::CreateCommaSeparated(krys::move(list));
  }

  // MARK: - Serialization

  void Serialiser<FontVariationSettings>::operator()(css::CSSOMString &builder,
                                                     const css::SerialisationContext &context,
                                                     const RenderStyle &style,
                                                     const FontVariationSettings &value) noexcept
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
      builder.push_back(u8' ');
      Serialise(builder, context, style, FontVariationSettings::Value {setting.Value()});
    }
  }

  // MARK: - Blending

  bool Blending<FontVariationSettings>::CanBlend(const FontVariationSettings &a,
                                                 const FontVariationSettings &b) noexcept
  {
    if (a.Platform().size() != b.Platform().size())
    {
      return false;
    }

    for (size_t i = 0; i < a.Platform().size(); ++i)
    {
      if (a.Platform()[i].Tag() != b.Platform()[i].Tag())
      {
        return false;
      }
    }

    return true;
  }

  FontVariationSettings Blending<FontVariationSettings>::Blend(const FontVariationSettings &a,
                                                               const FontVariationSettings &b,
                                                               const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? b : a;
    }

    krys_debug_assert(CanBlend(a, b));

    boo::FontVariationSettings platformSettingsResult;

    for (size_t i = 0; i < a.Platform().size(); ++i)
    {
      const auto &aSetting = a.Platform()[i];
      const auto &bSetting = b.Platform()[i];
      krys_debug_assert(aSetting.Tag() == bSetting.Tag());

      platformSettingsResult.insert(
        {aSetting.Tag(), style::Blend(FontVariationSettings::Value {aSetting.Value()},
                                      FontVariationSettings::Value {bSetting.Value()}, context)
                           .Value});
    }

    return {krys::move(platformSettingsResult)};
  }
}

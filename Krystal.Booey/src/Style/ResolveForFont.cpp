#include "Krystal.Booey/Style/ResolveForFont.hpp"
#include "Krystal.Booey/CSS/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/FontSelectionValueInlines.hpp"
#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Values/FontStyleWithAngleValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesEvaluateCalc.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Style/FontSizeFunctions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include <algorithm>

namespace krys::boo::style
{
  // MARK: - 'font-weight'

  FontSelectionValue FontWeightFromCSSValueDeprecated(const css::Value &value) noexcept
  {
    const auto &primitiveValue = Downcast<css::PrimitiveValue>(value);

    if (primitiveValue.IsNumber())
    {
      return FontSelectionValue(std::clamp(primitiveValue.ResolveAsNumberDeprecated<float>(), 1.0f, 1000.0f));
    }

    krys_debug_assert(primitiveValue.IsValueId());
    switch (primitiveValue.ValueId())
    {
      case css::ValueId::Normal:  return NormalWeightValue();
      case css::ValueId::Bold:
      case css::ValueId::Bolder:  return BoldWeightValue();
      case css::ValueId::Lighter: return LightWeightValue();
      default:                    krys_debug_assert(false); return NormalWeightValue();
    }
  }

  // MARK: - 'font-stretch'

  FontSelectionValue FontStretchFromCSSValueDeprecated(const css::Value &value) noexcept
  {
    const auto &primitiveValue = Downcast<css::PrimitiveValue>(value);

    if (primitiveValue.IsPercentage())
    {
      return FontSelectionValue::ClampFloat(primitiveValue.ResolveAsPercentageDeprecated<float>());
    }

    krys_debug_assert(primitiveValue.IsValueId());
    if (auto width = FontWidthValue(primitiveValue.ValueId()))
    {
      return *width;
    }

    krys_debug_assert(css::PropertyParserHelpers::IsSystemFontShorthand(primitiveValue.ValueId()));
    return NormalWidthValue();
  }

  // MARK: - 'font-style'

  FontSelectionValue FontStyleAngleFromCSSValueDeprecated(const css::Value &value) noexcept
  {
    return NormalizedFontItalicValue(Downcast<css::PrimitiveValue>(value).ResolveAsAngleDeprecated<float>());
  }

  Maybe<FontSelectionValue>
    FontStyleAngleFromCSSFontStyleWithAngleValueDeprecated(const css::FontStyleWithAngleValue &value) noexcept
  {
    if (css::RequiresConversionData(value.GetObliqueAngle()))
    {
      return null;
    }
    return FontSelectionValue {
      static_cast<float>(ConvertToStyleNoConversionDataRequired(value.GetObliqueAngle()).Value)};
  }

  Maybe<FontSelectionValue> FontStyleFromCSSValueDeprecated(const css::Value &value) noexcept
  {
    if (const auto *fontStyleValue = DynamicDowncast<css::FontStyleWithAngleValue>(value))
    {
      return FontStyleAngleFromCSSFontStyleWithAngleValueDeprecated(*fontStyleValue);
    }

    auto valueID = css::GetValueId(value);
    if (valueID == css::ValueId::Normal)
    {
      return null;
    }

    krys_debug_assert(valueID == css::ValueId::Italic || valueID == css::ValueId::Oblique);
    return ItalicValue();
  }

  static FontSelectionValue FontWeightFromUnresolvedFontWeight(
    const css::PropertyParserHelpers::UnresolvedFontWeight &unresolvedWeight,
    const FontCascadeDescription &fontDescription) noexcept
  {
    return krys::SwitchOn(
      unresolvedWeight,
      [&](css::ValueId ident)
      {
        switch (ident)
        {
          case css::ValueId::Normal:  return NormalWeightValue();
          case css::ValueId::Bold:    return BoldWeightValue();
          case css::ValueId::Bolder:  return FontCascadeDescription::BolderWeight(fontDescription.Weight());
          case css::ValueId::Lighter: return FontCascadeDescription::LighterWeight(fontDescription.Weight());
          default:                    break;
        }
        krys_debug_assert(false);
        return NormalWeightValue();
      },
      [&](const css::PropertyParserHelpers::UnresolvedFontWeightNumber &weight)
      {
        // FIXME: Figure out correct behavior when conversion data is required.
        if (css::RequiresConversionData(weight))
        {
          return NormalWeightValue();
        }
        return FontSelectionValue::ClampFloat(
          static_cast<float>(ConvertToStyleNoConversionDataRequired(weight).Value));
      });
  }

  static FontSelectionValue FontWidthFromUnresolvedFontWidth(
    const css::PropertyParserHelpers::UnresolvedFontWidth &unresolvedWidth) noexcept
  {
    return krys::SwitchOn(
      unresolvedWidth,
      [&](css::ValueId ident)
      {
        if (auto value = FontWidthValue(ident))
        {
          return *value;
        }
        return NormalWidthValue();
      },
      [&](const css::PropertyParserHelpers::UnresolvedFontWidthPercentage &percentage)
      {
        // FIXME: Figure out correct behavior when conversion data is required.
        if (css::RequiresConversionData(percentage))
        {
          return NormalWidthValue();
        }
        return FontSelectionValue::ClampFloat(
          static_cast<float>(ConvertToStyleNoConversionDataRequired(percentage).Value));
      });
  }

  namespace
  {
    struct ResolvedFontStyle
    {
      Maybe<FontSelectionValue> Slope;
      FontStyleAxis Axis;
    };
  }

  static ResolvedFontStyle FontStyleFromUnresolvedFontStyle(
    const css::PropertyParserHelpers::UnresolvedFontStyle &unresolvedStyle) noexcept
  {
    // Font style applied in the same way as BuilderConverter::ConvertFontStyleFromValue
    return krys::SwitchOn(
      unresolvedStyle,
      [](css::ValueId ident) -> ResolvedFontStyle
      {
        switch (ident)
        {
          case css::ValueId::Normal:  return {.Slope = null, .Axis = FontStyleAxis::slnt};
          case css::ValueId::Italic:  return {.Slope = ItalicValue(), .Axis = FontStyleAxis::ital};
          case css::ValueId::Oblique: return {.Slope = FontSelectionValue(0.0f), .Axis = FontStyleAxis::slnt};
          default:                    break;
        }

        krys_debug_assert(false);
        return {.Slope = null, .Axis = FontStyleAxis::slnt};
      },
      [](const css::PropertyParserHelpers::UnresolvedFontStyleObliqueAngle &angle) -> ResolvedFontStyle
      {
        // FIXME: Figure out correct behavior when conversion data is required.
        if (css::RequiresConversionData(angle))
        {
          return {.Slope = null, .Axis = FontStyleAxis::slnt};
        }

        return {.Slope = FontSelectionValue::ClampFloat(
                  static_cast<float>(ConvertToStyleNoConversionDataRequired(angle).Value)),
                .Axis = FontStyleAxis::slnt};
      });
  }

  namespace
  {
    struct ResolvedFontSize
    {
      float Size;
      css::ValueId Keyword;
    };
  }

  static ResolvedFontSize
    FontSizeFromUnresolvedFontSize(const css::PropertyParserHelpers::UnresolvedFontSize &unresolvedSize,
                                   float parentSize, const FontCascadeDescription &fontDescription,
                                   dom::Document &document) noexcept
  {
    return krys::SwitchOn(
      unresolvedSize,
      [&](css::ValueId ident) -> ResolvedFontSize
      {
        switch (ident)
        {
          case css::ValueId::XxSmall:
          case css::ValueId::XSmall:
          case css::ValueId::Small:
          case css::ValueId::Medium:
          case css::ValueId::Large:
          case css::ValueId::XLarge:
          case css::ValueId::XxLarge:
          case css::ValueId::XxxLarge:
            return {.Size = FontSizeForKeyword(ToUnderlying(ident), fontDescription.UseFixedDefaultSize(),
                                               document.GetSettings().Values),
                    .Keyword = ident};

          // As the style builder resolves them (BuilderCustom::LargerFontSize and SmallerFontSize).
          case css::ValueId::Larger:  return {.Size = parentSize * 1.2f, .Keyword = css::ValueId::Invalid};

          case css::ValueId::Smaller: return {.Size = parentSize / 1.2f, .Keyword = css::ValueId::Invalid};

          case css::ValueId::Math:    return {.Size = 0.0f, .Keyword = css::ValueId::Invalid};

          default:                    break;
        }

        krys_debug_assert(false);
        return {.Size = 0.0f, .Keyword = css::ValueId::Invalid};
      },
      [&](const css::LengthPercentage<css::NonNegative> &lengthPercentage) -> ResolvedFontSize
      {
        // FIXME: Figure out correct behavior when conversion data is required.
        if (css::RequiresConversionData(lengthPercentage) && css::IsUnevaluatedCalc(lengthPercentage))
        {
          return {.Size = 0.0f, .Keyword = css::ValueId::Invalid};
        }
        if (auto raw = lengthPercentage.GetRaw())
        {
          return css::SwitchOnUnitType(
            raw->Unit,
            [&](css::PercentageUnit) -> ResolvedFontSize
            {
              return {.Size = parentSize * static_cast<float>(raw->Value) / 100.0f,
                      .Keyword = css::ValueId::Invalid};
            },
            [&](css::LengthUnit lengthUnit) -> ResolvedFontSize
            {
              auto fontCascade = FontCascade(FontCascadeDescription(fontDescription));
              fontCascade.Update(ShareRefPtr(&document.GetFontSelector()));

              // FIXME: Passing null for the RenderView parameter means that vw and vh units will evaluate to
              //        zero and vmin and vmax units will evaluate as if they were px units.
              return {.Size = static_cast<float>(ComputeUnzoomedNonCalcLengthDouble(
                        raw->Value, lengthUnit, css::PropertyId::FontSize, &fontCascade,
                        css::RangeZoomOptions::Default, document.GetRenderView())),
                      .Keyword = css::ValueId::Invalid};
            });
        }
        return {.Size = Evaluate<float>(ConvertToStyleNoConversionDataRequired(lengthPercentage), parentSize,
                                        ZoomNeeded {}),
                .Keyword = css::ValueId::Invalid};
      });
  }

  namespace
  {
    struct ResolvedFontFamily
    {
      List<StringAtom> Family;
      bool IsSpecifiedFont;
    };
  }

  static ResolvedFontFamily FontFamilyFromUnresolvedFontFamily(
    const css::PropertyParserHelpers::UnresolvedFontFamily &unresolvedFamily) noexcept
  {
    bool isFirstFont = true;
    bool isSpecifiedFont = false;

    List<StringAtom> families;
    for (const auto &item : unresolvedFamily)
    {
      auto [family, isGenericFamily] = krys::SwitchOn(
        item,
        [&](css::ValueId ident) -> std::pair<StringAtom, bool>
        {
          // FIXME: Treat system-ui like other generic font families
          if (ident == css::ValueId::SystemUi)
          {
            return {css::WebKitFontFamilyNames::SystemUiFamily(), true};
          }
          return {
            css::WebKitFontFamilyNames::FamilyName(css::PropertyParserHelpers::GenericFontFamilyIndex(ident)),
            true};
        },
        [&](const css::CSSOMStringAtom &familyString) -> std::pair<StringAtom, bool>
        { return {familyString, false}; });

      if (family.View().empty())
      {
        continue;
      }

      if (isFirstFont)
      {
        isSpecifiedFont = !isGenericFamily;
        isFirstFont = false;
      }
      families.push_back(krys::move(family));
    }

    return {.Family = krys::move(families), .IsSpecifiedFont = isSpecifiedFont};
  }

  // MARK: - Unresolved Font Shorthand Resolution

  Maybe<FontCascade>
    ResolveForUnresolvedFont(const css::PropertyParserHelpers::UnresolvedFont &unresolvedFont,
                             FontCascadeDescription &&fontDescription, dom::Document &document) noexcept
  {
    // Map the font property longhands into the style.
    float parentSize = fontDescription.SpecifiedSize();

    // Font family applied in the same way as BuilderCustom::ApplyValueFontFamily
    // Before mapping in a new font-family property, we should reset the generic family.
    bool oldFamilyUsedFixedDefaultSize = fontDescription.UseFixedDefaultSize();

    auto resolvedFamily = FontFamilyFromUnresolvedFontFamily(unresolvedFont.Family);
    if (resolvedFamily.Family.empty())
    {
      return null;
    }
    fontDescription.SetFamilies(krys::move(resolvedFamily.Family));
    fontDescription.SetIsSpecifiedFont(resolvedFamily.IsSpecifiedFont);

    const auto &settings = document.GetSettings().Values;
    if (fontDescription.UseFixedDefaultSize() != oldFamilyUsedFixedDefaultSize)
    {
      if (auto sizeIdentifier = fontDescription.KeywordSizeAsIdentifier();
          sizeIdentifier != css::ValueId::Invalid)
      {
        auto size =
          FontSizeForKeyword(ToUnderlying(sizeIdentifier), !oldFamilyUsedFixedDefaultSize, settings);
        fontDescription.SetSpecifiedSize(size);
        fontDescription.SetComputedSize(ComputedFontSizeFromSpecifiedSize(
          size, fontDescription.IsAbsoluteSize(), 1.0f, MinimumFontSizeRule::None, settings));
      }
    }

    auto resolvedFontStyle = FontStyleFromUnresolvedFontStyle(unresolvedFont.Style);
    fontDescription.SetFontStyleSlope(resolvedFontStyle.Slope);
    fontDescription.SetFontStyleAxis(resolvedFontStyle.Axis);

    fontDescription.SetVariantCaps(FromValueId<FontVariantCaps>(unresolvedFont.VariantCaps));

    fontDescription.SetWeight(FontWeightFromUnresolvedFontWeight(unresolvedFont.Weight, fontDescription));

    fontDescription.SetWidth(FontWidthFromUnresolvedFontWidth(unresolvedFont.Width));

    auto resolvedSize =
      FontSizeFromUnresolvedFontSize(unresolvedFont.Size, parentSize, fontDescription, document);
    fontDescription.SetKeywordSizeFromIdentifier(resolvedSize.Keyword);
    if (resolvedSize.Size > 0)
    {
      fontDescription.SetSpecifiedSize(resolvedSize.Size);
      fontDescription.SetComputedSize(resolvedSize.Size);
    }

    // As there is no line-height on FontCascade, there's no need to resolve it, even
    // though there is line-height information on css::PropertyParserHelpers::UnresolvedFont.

    auto fontCascade = FontCascade(krys::move(fontDescription));
    fontCascade.Update(ShareRefPtr(&document.GetFontSelector()));
    return fontCascade;
  }
}

#include "Krystal.Booey/CSS/CSSOM/CSSFontFeatureValuesRule.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/Graphics/FontFeatureValues.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <charconv>

namespace krys::boo::css
{
#pragma region CSSFontFeatureValuesRule

  CSSFontFeatureValuesRule::CSSFontFeatureValuesRule(css::FontFeatureValuesRule &fontFeatureValuesRule,
                                                     CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _fontFeatureValuesRule(ShareRef(fontFeatureValuesRule))
  {
  }

  Ref<CSSFontFeatureValuesRule> CSSFontFeatureValuesRule::Create(css::FontFeatureValuesRule &rule,
                                                                 CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSFontFeatureValuesRule(rule, sheet));
  }

  const List<CSSOMStringAtom> &CSSFontFeatureValuesRule::FontFamilies() const noexcept
  {
    return _fontFeatureValuesRule->FontFamilies();
  }

  CSSOMStringAtom CSSFontFeatureValuesRule::FontFamily() const noexcept
  {
    StringBuilder builder;
    bool first = true;
    for (const auto &family : _fontFeatureValuesRule->FontFamilies())
    {
      if (first)
      {
        first = false;
      }
      else
      {
        builder.Append(u8", ");
      }

      builder.Append(family.View());
    }
    return CSSOMStringAtom(krys::move(builder));
  }

  CSSOMString CSSFontFeatureValuesRule::CssText() const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@font-feature-values ");

    auto joinFontFamiliesWithSeparator = [&builder](const auto &elements, CSSOMStringView separator)
    {
      bool first = true;
      for (const auto &element : elements)
      {
        if (!first)
        {
          builder.Append(separator);
        }
        builder.Append(SerialiseFontFamily(CSSOMString(element.View())));
        first = false;
      }
    };
    joinFontFamiliesWithSeparator(_fontFeatureValuesRule->FontFamilies(), u8", ");
    builder.Append(u8" { ");
    auto value = _fontFeatureValuesRule->Value();

    auto addVariant = [&builder](CSSOMStringView variantName, const auto &tags)
    {
      if (!tags.empty())
      {
        builder.Append(u8"@", variantName, u8" { ");
        for (const auto &[key, integers] : tags)
        {
          SerialiseIdentifier(key, builder);
          builder.Append(u8':');
          for (auto integer : integers)
          {
            builder.Append(u8' ', integer);
          }
          builder.Append(u8"; ");
        }
        builder.Append(u8"} ");
      }
    };

    // WPT expects the order used in Servo.
    // https://searchfox.org/mozilla-central/source/servo/components/style/stylesheets/font_feature_values_rule.rs#430
    addVariant(u8"swash", value->Swash());
    addVariant(u8"stylistic", value->Stylistic());
    addVariant(u8"ornaments", value->Ornaments());
    addVariant(u8"annotation", value->Annotation());
    addVariant(u8"character-variant", value->CharacterVariant());
    addVariant(u8"styleset", value->Styleset());

    builder.Append(u8'}');
    return krys::move(builder).ToString();
  }

  void CSSFontFeatureValuesRule::Reattach(Rule &rule) noexcept
  {
    _fontFeatureValuesRule = ShareRef(Downcast<css::FontFeatureValuesRule>(rule));
  }

#pragma endregion

#pragma region CSSFontFeatureValuesBlockRule

  CSSFontFeatureValuesBlockRule::CSSFontFeatureValuesBlockRule(css::FontFeatureValuesBlockRule &block,
                                                               CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _fontFeatureValuesBlockRule(ShareRef(block))
  {
  }

  Ref<CSSFontFeatureValuesBlockRule>
    CSSFontFeatureValuesBlockRule::Create(css::FontFeatureValuesBlockRule &rule,
                                          CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSFontFeatureValuesBlockRule(rule, sheet));
  }

  CSSOMString CSSFontFeatureValuesBlockRule::CssText() const noexcept
  {
    // This rule is always contained inside a FontFeatureValuesRule,
    // which is the only one we are expected to serialize to CSS.
    // We should never serialize a Block by itself.
    krys_debug_assert(false);
    return {};
  }

  void CSSFontFeatureValuesBlockRule::Reattach(Rule &rule) noexcept
  {
    _fontFeatureValuesBlockRule = ShareRef(Downcast<css::FontFeatureValuesBlockRule>(rule));
  }

#pragma endregion
}

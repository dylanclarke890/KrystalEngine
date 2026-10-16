#include "Krystal.Booey/CSS/CSSOM/CSSFontPaletteValuesRule.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <charconv>

namespace krys::boo::css
{
  namespace
  {
  }

  CSSFontPaletteValuesRule::CSSFontPaletteValuesRule(css::FontPaletteValuesRule &fontPaletteValuesRule,
                                                     CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _fontPaletteValuesRule(ShareRef(fontPaletteValuesRule))
  {
  }

  CSSFontPaletteValuesRule::~CSSFontPaletteValuesRule() noexcept = default;

  Ref<CSSFontPaletteValuesRule> CSSFontPaletteValuesRule::Create(css::FontPaletteValuesRule &rule,
                                                                 CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSFontPaletteValuesRule(rule, sheet));
  }

  CSSOMString CSSFontPaletteValuesRule::Name() const noexcept
  {
    return CSSOMString(_fontPaletteValuesRule->Name().View());
  }

  CSSOMString CSSFontPaletteValuesRule::FontFamily() const noexcept
  {
    StringBuilder result;
    for (const auto &family : _fontPaletteValuesRule->FontFamilies())
    {
      if (!result.IsEmpty())
      {
        result.Append(u8", ");
      }
      result.Append(SerialiseFontFamily(CSSOMString(family.View())));
    }
    return krys::move(result).ToString();
  }

  CSSOMString CSSFontPaletteValuesRule::BasePalette() const noexcept
  {
    auto basePalette = _fontPaletteValuesRule->BasePalette();
    if (!basePalette)
    {
      return {};
    }

    switch (basePalette->IndexType)
    {
      case FontPaletteIndex::Type::Light:
      {
        return u8"light";
      }
      case FontPaletteIndex::Type::Dark:
      {
        return u8"dark";
      }
      case FontPaletteIndex::Type::Integer:
      {
        StringBuilder result;
        result.Append(basePalette->Integer);
        return krys::move(result).ToString();
      }
    }
    krys_unreachable();
  }

  CSSOMString CSSFontPaletteValuesRule::OverrideColors() const noexcept
  {
    StringBuilder result;
    const auto &overrideColors = _fontPaletteValuesRule->OverrideColors();
    for (size_t i = 0; i < overrideColors.size(); ++i)
    {
      if (i)
      {
        result.Append(u8", ");
      }
      const auto &item = overrideColors[i];
      result.Append(item.first);
      result.Append(u8' ');
      result.Append(SerialisationForCSS(item.second));
    }
    return krys::move(result).ToString();
  }

  CSSOMString CSSFontPaletteValuesRule::CssText() const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@font-palette-values ", Name(), u8" { ");

    if (!_fontPaletteValuesRule->FontFamilies().empty())
    {
      builder.Append(u8"font-family: ", FontFamily(), u8"; ");
    }

    if (auto basePalette = _fontPaletteValuesRule->BasePalette())
    {
      switch (basePalette->IndexType)
      {
        case FontPaletteIndex::Type::Light:
        {
          builder.Append(u8"base-palette: light; ");
          break;
        }
        case FontPaletteIndex::Type::Dark:
        {
          builder.Append(u8"base-palette: dark; ");
          break;
        }
        case FontPaletteIndex::Type::Integer:
        {
          builder.Append(u8"base-palette: ");
          builder.Append(basePalette->Integer);
          builder.Append(u8"; ");
          break;
        }
      }
    }

    const auto &overrideColors = _fontPaletteValuesRule->OverrideColors();
    if (!overrideColors.empty())
    {
      builder.Append(u8"override-colors:");
      for (size_t i = 0; i < overrideColors.size(); ++i)
      {
        if (i)
        {
          builder.Append(u8',');
        }
        builder.Append(u8' ');
        builder.Append(overrideColors[i].first);
        builder.Append(u8' ');
        builder.Append(SerialisationForCSS(overrideColors[i].second));
      }
      builder.Append(u8"; ");
    }

    builder.Append(u8'}');
    return krys::move(builder).ToString();
  }

  void CSSFontPaletteValuesRule::Reattach(Rule &rule) noexcept
  {
    _fontPaletteValuesRule = ShareRef(Downcast<css::FontPaletteValuesRule>(rule));
  }
}

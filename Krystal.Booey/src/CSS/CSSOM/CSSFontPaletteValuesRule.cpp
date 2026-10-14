#include "Krystal.Booey/CSS/CSSOM/CSSFontPaletteValuesRule.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Core/Assert.hpp"
#include <charconv>

namespace krys::boo::css
{
  namespace
  {
    void AppendInteger(CSSOMString &builder, uint32 integer) noexcept
    {
      char buffer[16];
      auto result = std::to_chars(buffer, buffer + sizeof(buffer), integer);
      builder.append(reinterpret_cast<const char8 *>(buffer), reinterpret_cast<const char8 *>(result.ptr));
    }
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
    CSSOMString result;
    for (const auto &family : _fontPaletteValuesRule->FontFamilies())
    {
      if (!result.empty())
      {
        result.append(u8", ");
      }
      result.append(SerialiseFontFamily(CSSOMString(family.View())));
    }
    return result;
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
        CSSOMString result;
        AppendInteger(result, basePalette->Integer);
        return result;
      }
    }
    krys_unreachable();
  }

  CSSOMString CSSFontPaletteValuesRule::OverrideColors() const noexcept
  {
    CSSOMString result;
    const auto &overrideColors = _fontPaletteValuesRule->OverrideColors();
    for (size_t i = 0; i < overrideColors.size(); ++i)
    {
      if (i)
      {
        result.append(u8", ");
      }
      const auto &item = overrideColors[i];
      AppendInteger(result, item.first);
      result.push_back(u8' ');
      result.append(SerialisationForCSS(item.second));
    }
    return result;
  }

  CSSOMString CSSFontPaletteValuesRule::CssText() const noexcept
  {
    CSSOMString builder;
    builder.append(u8"@font-palette-values ").append(Name()).append(u8" { ");

    if (!_fontPaletteValuesRule->FontFamilies().empty())
    {
      builder.append(u8"font-family: ").append(FontFamily()).append(u8"; ");
    }

    if (auto basePalette = _fontPaletteValuesRule->BasePalette())
    {
      switch (basePalette->IndexType)
      {
        case FontPaletteIndex::Type::Light:
        {
          builder.append(u8"base-palette: light; ");
          break;
        }
        case FontPaletteIndex::Type::Dark:
        {
          builder.append(u8"base-palette: dark; ");
          break;
        }
        case FontPaletteIndex::Type::Integer:
        {
          builder.append(u8"base-palette: ");
          AppendInteger(builder, basePalette->Integer);
          builder.append(u8"; ");
          break;
        }
      }
    }

    const auto &overrideColors = _fontPaletteValuesRule->OverrideColors();
    if (!overrideColors.empty())
    {
      builder.append(u8"override-colors:");
      for (size_t i = 0; i < overrideColors.size(); ++i)
      {
        if (i)
        {
          builder.push_back(u8',');
        }
        builder.push_back(u8' ');
        AppendInteger(builder, overrideColors[i].first);
        builder.push_back(u8' ');
        builder.append(SerialisationForCSS(overrideColors[i].second));
      }
      builder.append(u8"; ");
    }

    builder.push_back(u8'}');
    return builder;
  }

  void CSSFontPaletteValuesRule::Reattach(Rule &rule) noexcept
  {
    _fontPaletteValuesRule = ShareRef(Downcast<css::FontPaletteValuesRule>(rule));
  }
}

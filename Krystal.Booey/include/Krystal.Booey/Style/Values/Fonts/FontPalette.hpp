#pragma once

#include "Krystal.Booey/Graphics/FontPalette.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'font-palette'> = normal | light | dark | <palette-identifier> | <palette-mix()>
  // FIXME: <palette-mix()> is not yet supported.
  // https://drafts.csswg.org/css-fonts/#propdef-font-palette
  struct FontPalette
  {
    FontPalette(css::keywords::Normal) noexcept
        : _platform {boo::FontPalette::Type::Normal, css::CSSOMStringAtom::Null()}
    {
    }

    FontPalette(css::keywords::Light) noexcept
        : _platform {boo::FontPalette::Type::Light, css::CSSOMStringAtom::Null()}
    {
    }

    FontPalette(css::keywords::Dark) noexcept
        : _platform {boo::FontPalette::Type::Dark, css::CSSOMStringAtom::Null()}
    {
    }

    FontPalette(css::CustomIdentifier &&identifier) noexcept
        : _platform {boo::FontPalette::Type::Custom, krys::move(identifier.Value)}
    {
    }

    FontPalette(boo::FontPalette &&platform) noexcept : _platform {krys::move(platform)}
    {
    }

    FontPalette(const boo::FontPalette &platform) noexcept : _platform {platform}
    {
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      switch (_platform.PaletteType)
      {
        case boo::FontPalette::Type::Normal: return visitor(css::keywords::Normal {});
        case boo::FontPalette::Type::Light:  return visitor(css::keywords::Light {});
        case boo::FontPalette::Type::Dark:   return visitor(css::keywords::Dark {});
        case boo::FontPalette::Type::Custom: return visitor(css::CustomIdentifier {_platform.Identifier});
      }
      krys_unreachable();
    }

    KRYS_NODISCARD const boo::FontPalette &Platform() const noexcept
    {
      return _platform;
    }

    KRYS_NODISCARD bool operator==(const FontPalette &) const noexcept = default;

  private:
    boo::FontPalette _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontPalette>
  {
    KRYS_NODISCARD FontPalette operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::FontPalette)

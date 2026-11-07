#include "Krystal.UI/FontFaceCache.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Fonts/Font.hpp"

namespace krys::ui
{
  const gfx::FontFace2D *FontFaceCache::FaceFor(const boo::Font &font) noexcept
  {
    const auto &platformData = font.PlatformData();
    auto *face = platformData.Face();
    if (!face)
    {
      return nullptr;
    }

    const bool bold = platformData.SyntheticBold();
    const bool oblique = platformData.SyntheticOblique();
    const uint32 paletteIndex = platformData.PaletteIndex();
    List<gfx::PaletteColour> overrides;
    for (const auto &[index, colour] : platformData.PaletteOverrides())
    {
      auto [r, g, b, a] = colour.ToColorTypeLossy<SRGBA<uint8>>().Resolved();
      overrides.push_back({.Index = index, .R = r, .G = g, .B = b, .A = a});
    }
    for (const auto &entry : _entries)
    {
      if (entry.Face == face && entry.SyntheticBold == bold && entry.SyntheticOblique == oblique
          && entry.PaletteIndex == paletteIndex && entry.PaletteOverrides == overrides)
      {
        return entry.Face2D.get();
      }
    }

    auto created = gfx::FontFace2D::Create(ShareRef(*face), {.SyntheticBold = bold,
                                                             .SyntheticOblique = oblique,
                                                             .PaletteIndex = paletteIndex,
                                                             .PaletteOverrides = overrides});
    if (!created)
    {
      return nullptr;
    }
    _entries.push_back({face, bold, oblique, paletteIndex, krys::move(overrides), krys::move(*created)});
    return _entries.back().Face2D.get();
  }
}

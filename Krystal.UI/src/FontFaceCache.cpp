#include "Krystal.UI/FontFaceCache.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"

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
    for (const auto &entry : _entries)
    {
      if (entry.Face == face && entry.SyntheticBold == bold && entry.SyntheticOblique == oblique)
      {
        return entry.Face2D.get();
      }
    }

    auto created =
      gfx::FontFace2D::Create(ShareRef(*face), {.SyntheticBold = bold, .SyntheticOblique = oblique});
    if (!created)
    {
      return nullptr;
    }
    _entries.push_back({face, bold, oblique, krys::move(*created)});
    return _entries.back().Face2D.get();
  }
}

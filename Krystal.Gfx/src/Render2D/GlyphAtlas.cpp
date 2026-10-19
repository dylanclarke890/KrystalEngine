#include "Krystal.Gfx/Render2D/GlyphAtlas.hpp"
#include <algorithm>
#include <bit>
#include <cmath>

namespace krys::gfx
{
  size_t GlyphAtlas::KeyHash::operator()(const Key &key) const noexcept
  {
    size_t seed = static_cast<size_t>(key.Face);
    auto mix = [&](uint32 value)
    {
      seed ^= value + 0x9e'37'79'b9u + (seed << 6u) + (seed >> 2u);
    };
    mix(key.Glyph);
    mix(std::bit_cast<uint32>(key.Size));
    mix(key.Subpixel);
    for (const auto value : key.Linear)
    {
      mix(std::bit_cast<uint32>(value));
    }
    return seed;
  }

  GlyphAtlas::GlyphAtlas(Device &device, const GlyphAtlasDesc &desc) noexcept
      : _coverage(device, {.Format = TextureFormat::R8Unorm,
                           .PageSize = desc.PageSize,
                           .MaxPages = desc.MaxPages,
                           .DebugName = u8"Render2D.GlyphPage"}),
        _colour(device, {.Format = TextureFormat::RGBA8Unorm,
                         .PageSize = desc.PageSize,
                         .MaxPages = desc.MaxColourPages,
                         .DebugName = u8"Render2D.ColourGlyphPage"})
  {
  }

  Maybe<GlyphLocation> GlyphAtlas::GetGlyph(const FontFace2D &face, uint32 glyph, float size, uint32 subpixel,
                                            const Transform2D &linear) noexcept
  {
    subpixel %= SubpixelPositions;
    const Key key {.Face = face.Id(),
                   .Glyph = glyph,
                   .Size = size,
                   .Subpixel = subpixel,
                   .Linear = {linear.A, linear.B, linear.C, linear.D}};
    auto locate = [&](const Entry &entry)
    {
      auto &pages = entry.Colour ? _colour : _coverage;
      if (entry.Page == NoPage)
      {
        return GlyphLocation {.Texture = {}, .Texels = {}, .Bounds = entry.Bounds, .Colour = entry.Colour};
      }
      pages.MarkUsed(entry.Page);
      return GlyphLocation {.Texture = pages.Texture(entry.Page),
                            .Texels = entry.Texels,
                            .Bounds = entry.Bounds,
                            .Colour = entry.Colour};
    };

    if (auto found = _entries.find(key); found != _entries.end())
    {
      ++_stats.Hits;
      return locate(found->second);
    }

    const auto bitmap = face.Rasterise(glyph, size, static_cast<float>(subpixel) / SubpixelPositions,
                                       {.A = linear.A, .B = linear.B, .C = linear.C, .D = linear.D});
    if (!bitmap)
    {
      return {};
    }
    ++_stats.Rasterised;

    Entry entry {.Colour = bitmap->Colour, .Page = NoPage, .Texels = {}, .Bounds = bitmap->Bounds};
    if (!bitmap->Bounds.IsEmpty())
    {
      auto &pages = bitmap->Colour ? _colour : _coverage;
      const bool colour = bitmap->Colour;
      const auto slot =
        pages.Allocate(bitmap->Bounds.Width, bitmap->Bounds.Height,
                       [&](uint32 page)
                       {
                         std::erase_if(_entries, [&](const auto &item)
                                       { return item.second.Colour == colour && item.second.Page == page; });
                         ++_stats.PageEvictions;
                       });
      if (!slot)
      {
        return {};
      }
      pages.Write(*slot, Span<const byte>(reinterpret_cast<const byte *>(bitmap->Pixels.data()),
                                          bitmap->Pixels.size()));
      entry.Page = slot->Page;
      entry.Texels = slot->Texels;
    }
    _entries.emplace(key, entry);
    return locate(entry);
  }

  std::pair<int32, uint32> GlyphAtlas::SnapPen(float x) noexcept
  {
    const float quarters = std::round(x * static_cast<float>(SubpixelPositions));
    const auto steps = static_cast<int64>(quarters);
    const auto positions = static_cast<int64>(SubpixelPositions);
    // Floor division, so pens left of the origin land in the pixel to their left.
    int64 pixel = steps / positions;
    int64 quarter = steps % positions;
    if (quarter < 0)
    {
      quarter += positions;
      --pixel;
    }
    return {static_cast<int32>(pixel), static_cast<uint32>(quarter)};
  }
}

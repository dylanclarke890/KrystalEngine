#pragma once

#include "Krystal.Gfx.OpenGL/Registries/FontRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Context.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Font.hpp"
#include "Krystal.Lib/DebugBreak.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.Platform/Platform.hpp"

namespace Krys::Gfx::OpenGL
{
  FontRegistry::FontRegistry(IContext &context) noexcept
      : _context(static_cast<Context &>(context)),
        _dpi(Platform::GetDPIForWindow(Platform::GetActiveWindow()))
  {
  }

  FontRegistry::~FontRegistry() noexcept
  {
  }

  void FontRegistry::Startup()
  {
    const IO::Path defaultFontPath("data/assets/fonts/Antonio-Bold.ttf");
    _defaultFontFamily = Register(_context.Strings().Intern("Antonio"), defaultFontPath);
    if (!_defaultFontFamily.IsValid())
    {
      KRYS_ERROR("Failed to register default font '{}'", defaultFontPath.ToString());
      KRYS_DEBUG_BREAK();
    }
  }

  void FontRegistry::Shutdown() noexcept
  {
    // TODO: Unload all fonts and font families
  }

  FontFamilyHandle FontRegistry::Register(StringRef name, const IO::Path &path) noexcept
  {
    FontFamily fontFamily {name, path};
    FontFamilyHandle handle = _fontFamilies.Add(std::move(fontFamily));
    return handle;
  }

  bool FontRegistry::Unload(FontFamilyHandle handle) noexcept
  {
    auto &fontFamily = _fontFamilies.Get(handle);
    for (const auto &fontHandle : fontFamily.Fonts())
    {
      Unload(fontHandle);
    }
    return _fontFamilies.Remove(handle);
  }

  FontHandle FontRegistry::Get(const FontDesc &desc) noexcept
  {
    assert(desc.Family.IsValid() && "Invalid font family handle.");
    auto &fontFamily = _fontFamilies.Get(desc.Family);
    const auto &path = fontFamily.Path();

    // non-bitmap fonts are resolution-independent, so we can cache them without size
    auto key = desc.Type == FontType::Bitmap ? desc : FontDesc {desc.Family, desc.Type, 0.f};
    if (FontHandle cached = _cache.Get(key); cached.IsValid())
    {
      KRYS_DEBUG("Font cache hit.");
      return cached;
    }

    Expected<FontAtlasData> expected;
    switch (desc.Type)
    {
      case FontType::Bitmap: expected = _loader.LoadBitmap(path, PtSizeToPixels(desc.Size)); break;
      case FontType::SDF:    expected = _loader.LoadSDF(path); break;
      case FontType::MSDF:   expected = _loader.LoadMSDF(path); break;
      case FontType::MTSDF:  expected = _loader.LoadMTSDF(path); break;
      default:               std::unreachable();
    }

    if (!expected.has_value())
    {
      KRYS_ERROR("Failed to load font '{}': {}", _fontFamilies.Get(desc.Family).Path().ToString(),
                 expected.error());
      return {};
    }

    FontAtlasData &data = expected.value();
    FontHandle font;
    switch (desc.Type)
    {
      case FontType::Bitmap: font = Add(Font::Bitmap(desc.Size, desc.Family, data), key); break;
      case FontType::SDF:    font = Add(Font::SDF(desc.Size, desc.Family, data), key); break;
      case FontType::MSDF:   font = Add(Font::MSDF(desc.Size, desc.Family, data), key); break;
      case FontType::MTSDF:  font = Add(Font::MTSDF(desc.Size, desc.Family, data), key); break;
      default:               std::unreachable();
    }

    fontFamily.AddFont(font);
    return font;
  }

  bool FontRegistry::Unload(FontHandle handle) noexcept
  {
    if (_cache.Remove(handle))
    {
      auto &font = _fonts.Get(handle);
      auto &fontFamily = _fontFamilies.Get(font.FontFamily());
      fontFamily.RemoveFont(handle);
      return _fonts.Remove(handle);
    }

    return false;
  }

  Font &FontRegistry::Get(FontHandle handle)
  {
    return _fonts.Get(handle);
  }

  Font *FontRegistry::TryGet(FontHandle handle) noexcept
  {
    return _fonts.TryGet(handle);
  }

  void FontRegistry::DPIChanged(int dpi) noexcept
  {
    if (dpi == _dpi)
    {
      return; // no change
    }
    _dpi = dpi;

    // We want to preserve the validity of existing FontHandles, so we need to reload
    // all bitmap fonts at the new DPI in-place.
    for (auto &[key, resource] : _cache)
    {
      Font &font = _fonts.Get(resource.Handle);
      if (font.Type() != FontType::Bitmap)
      {
        continue; // non-bitmap font types resolution-independent
      }

      auto &fontFamily = _fontFamilies.Get(font.FontFamily());
      auto expected = _loader.LoadBitmap(fontFamily.Path(), PtSizeToPixels(font.PtSize()));
      if (!expected.has_value())
      {
        KRYS_ERROR("Failed to load font '{}'", fontFamily.Path().ToString());
        KRYS_DEBUG_BREAK();
        continue;
      }

      font.SetAtlasData(expected.value());
    }
  }

  FontFamilyHandle FontRegistry::GetDefaultFontFamily() const noexcept
  {
    return _defaultFontFamily;
  }

  uint32 FontRegistry::PtSizeToPixels(float ptSize) const noexcept
  {
    auto pixelSize = (ptSize * _dpi) / 72.0;
    return static_cast<uint32>(pixelSize);
  }

  FontHandle FontRegistry::Add(Font &&font, const FontDesc &cacheKey)
  {
    FontHandle handle = _fonts.Add(std::move(font));
    _cache.Add(cacheKey, handle);
    return handle;
  }
}
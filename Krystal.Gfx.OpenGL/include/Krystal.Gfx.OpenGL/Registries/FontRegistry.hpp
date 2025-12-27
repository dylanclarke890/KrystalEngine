#pragma once

#include "Krystal.Gfx.OpenGL/Resources/Font.hpp"
#include "Krystal.Gfx/FontAtlasLoader.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Gfx/Registries/IFontRegistry.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.Gfx/ResourceManager.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/String/String.hpp"

namespace Krys::Gfx::OpenGL
{
  class FontRegistry final : public IFontRegistry
  {
    using FontFamilyManager = ResourceManager<FontFamily, FontFamilyHandle>;
    using FontManager = ResourceManager<Font, FontHandle>;
    using FontCache = ResourceHandleCache<FontDesc, FontHandle>;

    class Context &_context;
    int _dpi;
    FontFamilyManager _fontFamilies;
    FontManager _fonts;
    FontFamilyHandle _defaultFontFamily;
    FontCache _cache;
    FontAtlasLoader _loader;

  public:
    FontRegistry(IContext &context) noexcept;

    ~FontRegistry() noexcept override;

    void Startup() override;

    void Shutdown() noexcept override;

    KRYS_NODISCARD FontFamilyHandle Register(StringRef name, const IO::Path &path) noexcept override;

    bool Unload(FontFamilyHandle handle) noexcept override;

    KRYS_NODISCARD virtual FontHandle Get(const FontDesc &desc) noexcept override;

    bool Unload(FontHandle handle) noexcept override;

    KRYS_NODISCARD Font &Get(FontHandle handle);

    KRYS_NODISCARD Font *TryGet(FontHandle handle) noexcept;

    void DPIChanged(int dpi) noexcept;

    KRYS_NODISCARD FontFamilyHandle GetDefaultFontFamily() const noexcept override;

    KRYS_NODISCARD const CharacterMap &GetCharacterMap(FontHandle handle) const override;

    KRYS_NODISCARD TextureHandle GetFontAtlas(FontHandle handle) const override;

    KRYS_NODISCARD const FontMetrics &GetMetrics(FontHandle handle) const override;

    KRYS_NODISCARD uint32 PtSizeToPixels(float ptSize) const noexcept;

  private:
    KRYS_NODISCARD FontHandle Add(Font &&font, const FontDesc &cacheKey);

    KRYS_NODISCARD TextureHandle CreateFontAtlasTexture(const FontDesc &desc, const FontAtlasData &data);
  };
}
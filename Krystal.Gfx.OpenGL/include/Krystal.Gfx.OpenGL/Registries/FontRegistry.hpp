#pragma once

#include "Krystal.Gfx.Lib/ResourceManager.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Font.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Gfx/Registries/IFontRegistry.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/String.hpp"

namespace Krys::Gfx::OpenGL
{
  class FontRegistry final : public IFontRegistry
  {
    NO_COPY_MOVE(FontRegistry)

    using FontFamilyManager = ResourceManager<FontFamily, FontFamilyHandle>;
    using FontManager = ResourceManager<Font, FontHandle>;
    using FontCache = ResourceHandleCache<FontDesc, FontHandle>;

    class Context &_context;
    int _dpi;
    FontFamilyManager _fontFamilies;
    FontManager _fonts;
    FontFamilyHandle _defaultFontFamily;
    FontCache _cache;

  public:
    FontRegistry(IContext &context) noexcept;

    ~FontRegistry() noexcept override;

    void Startup() override;

    void Shutdown() noexcept override;

    NO_DISCARD FontFamilyHandle Register(StringRef name, const IO::Path &path) noexcept override;

    NO_DISCARD virtual FontHandle Get(const FontDesc &desc) noexcept override;

    bool Unload(FontHandle handle) noexcept override;

    bool Unload(FontFamilyHandle handle) noexcept override;

    NO_DISCARD Font &Get(FontHandle handle);

    NO_DISCARD Font *TryGet(FontHandle handle) noexcept;

    void DPIChanged(int dpi) noexcept;

    NO_DISCARD FontFamilyHandle GetDefaultFontFamily() noexcept override;
  };
}
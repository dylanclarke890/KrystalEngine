#pragma once

#include "Krystal.Gfx.Lib/ResourceManager.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Font.hpp"
#include "Krystal.Gfx/Handle.hpp"
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

    using FontManager = ResourceManager<Font, FontHandle>;
    using FontCache = ResourceHandleCache<string, FontHandle>;

    FontManager _fonts;
    FontHandle _defaultFont;
    FontCache _cache;
    int _dpi;

  public:
    FontRegistry(int dpi) noexcept;

    ~FontRegistry() noexcept override;

    void Startup() override;

    void Shutdown() noexcept override;

    NO_DISCARD FontHandle Load(const IO::Path &path, float ptSize, FontType fontType) noexcept override;

    bool Unload(FontHandle handle) noexcept override;

    NO_DISCARD Font &Get(FontHandle handle);

    NO_DISCARD Font *TryGet(FontHandle handle) noexcept;

    void DPIChanged(int dpi) noexcept;

    NO_DISCARD FontHandle GetDefault() noexcept override;
  };
}
#pragma once

#include "Krystal.Gfx.OpenGL/Fonts/Font.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Registries/IFontRegistry.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.Gfx/ResourceManager.hpp"
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
    FontCache _cache;
    int _dpi;

  public:
    FontRegistry(int dpi) noexcept;

    ~FontRegistry() noexcept override;

    void Startup() noexcept override;

    void Shutdown() noexcept override;

    NO_DISCARD FontHandle Load(const IO::Path &path, float ptSize, FontType fontType) noexcept override;

    void Unload(FontHandle handle) noexcept override;

    NO_DISCARD Font &Get(FontHandle handle);

    void DPIChanged(int dpi) noexcept;
  };
}
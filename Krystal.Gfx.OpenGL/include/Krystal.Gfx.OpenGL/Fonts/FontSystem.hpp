#pragma once

#include "Krystal.Gfx.OpenGL/Fonts/Font.hpp"
#include "Krystal.Gfx.OpenGL/Shaders/ShaderSystem.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/IFontSystem.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.Gfx/ResourceManager.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/String.hpp"

namespace Krys::Gfx::OpenGL
{
  class FontSystem final : public IFontSystem
  {
    NO_COPY_MOVE(FontSystem)

    using FontManager = ResourceManager<Font, FontHandle>;
    using FontCache = ResourceHandleCache<string, FontHandle>;

    FontManager _fonts;
    FontCache _cache;
    ShaderSystem &_shaders;
    int _dpi;

  public:
    FontSystem(ShaderSystem& shaders, int dpi) noexcept;

    ~FontSystem() noexcept override;

    NO_DISCARD FontHandle Load(const IO::Path &path, float ptSize, FontType fontType) noexcept override;

    void Unload(FontHandle handle) noexcept override;

    NO_DISCARD Font &Get(FontHandle handle);

    void DPIChanged(int dpi) noexcept;
  };
}
#pragma once

#include "Krystal.Gfx.OpenGL/Fonts/Font.hpp"
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

  public:
    FontSystem() noexcept;

    ~FontSystem() noexcept override;

    NO_DISCARD FontHandle Load(const IO::Path &path, float size,
                               FontType fontType = FontType::Bitmap) noexcept override;

    Font &Get(FontHandle);
  };
}
#pragma once

#include "Krystal.Gfx/Resources/Font.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Expected.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Gfx
{
  class FontAtlasLoader
  {
    NO_COPY_MOVE(FontAtlasLoader)

  public:
    FontAtlasLoader() noexcept = default;

    ~FontAtlasLoader() = default;

    KRYS_NODISCARD Expected<FontAtlasData> LoadBitmap(const IO::Path &path, uint32 fontSizeInPixels,
                                                  uint8 paddingPerGlyph = 2u) noexcept;

    KRYS_NODISCARD Expected<FontAtlasData> LoadSDF(const IO::Path &path,
                                               const SDFParams &params = SDFParams::Defaults()) noexcept;

    KRYS_NODISCARD Expected<FontAtlasData> LoadMSDF(const IO::Path &path,
                                                const SDFParams &params = SDFParams::Defaults()) noexcept;

    KRYS_NODISCARD Expected<FontAtlasData> LoadMTSDF(const IO::Path &path,
                                                 const SDFParams &params = SDFParams::Defaults()) noexcept;
  };
}
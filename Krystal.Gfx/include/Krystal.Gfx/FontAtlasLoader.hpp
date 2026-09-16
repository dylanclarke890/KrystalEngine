#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/IO/Path.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Gfx/Resources/Font.hpp"

namespace krys::Gfx
{
  class FontAtlasLoader
  {
    KRYS_NON_COPY_MOVABLE(FontAtlasLoader);

  public:
    FontAtlasLoader() noexcept = default;

    ~FontAtlasLoader() = default;

    KRYS_NODISCARD Expected<FontAtlasData> LoadBitmap(const io::Path &path, uint32 fontSizeInPixels,
                                                      uint8 paddingPerGlyph = 2u) noexcept;

    KRYS_NODISCARD Expected<FontAtlasData> LoadSDF(const io::Path &path,
                                                   const SDFParams &params = SDFParams::Defaults()) noexcept;

    KRYS_NODISCARD Expected<FontAtlasData> LoadMSDF(const io::Path &path,
                                                    const SDFParams &params = SDFParams::Defaults()) noexcept;

    KRYS_NODISCARD Expected<FontAtlasData>
      LoadMTSDF(const io::Path &path, const SDFParams &params = SDFParams::Defaults()) noexcept;
  };
}
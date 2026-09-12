#pragma once

#include "Krystal.Core/IO/Path.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Font.hpp"

namespace krys::Gfx
{
  class IFontRegistry
  {
    KRYS_NON_COPY_MOVABLE(IFontRegistry);

  protected:
    IFontRegistry() noexcept = default;

  public:
    virtual ~IFontRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    KRYS_NODISCARD virtual FontFamilyHandle Register(const utf8_string &name,
                                                     const io::Path &path) noexcept = 0;

    KRYS_NODISCARD virtual FontHandle Get(const FontDesc &desc) noexcept = 0;

    virtual bool Unload(FontHandle handle) noexcept = 0;

    virtual bool Unload(FontFamilyHandle handle) noexcept = 0;

    virtual void DPIChanged(int dpi) noexcept = 0;

    KRYS_NODISCARD virtual const CharacterMap &GetCharacterMap(FontHandle handle) const = 0;

    KRYS_NODISCARD virtual const FontMetrics &GetMetrics(FontHandle handle) const = 0;

    KRYS_NODISCARD virtual TextureHandle GetFontAtlas(FontHandle handle) const = 0;

    KRYS_NODISCARD virtual FontFamilyHandle GetDefaultFontFamily() const noexcept = 0;
  };
}
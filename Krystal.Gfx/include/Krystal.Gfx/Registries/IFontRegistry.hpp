#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Font.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Allocators/StringRef.hpp"
#include "Krystal.Lib/Macros.hpp"

namespace Krys::Gfx
{
  class IFontRegistry
  {
    NO_COPY_MOVE(IFontRegistry)

  protected:
    IFontRegistry() noexcept = default;

  public:
    virtual ~IFontRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    NO_DISCARD virtual FontFamilyHandle Register(StringRef name, const IO::Path &path) noexcept = 0;

    NO_DISCARD virtual FontHandle Get(const FontDesc &desc) noexcept = 0;

    virtual bool Unload(FontHandle handle) noexcept = 0;

    virtual bool Unload(FontFamilyHandle handle) noexcept = 0;

    virtual void DPIChanged(int dpi) noexcept = 0;

    NO_DISCARD virtual const CharacterMap &GetCharacterMap(FontHandle handle) const = 0;

    NO_DISCARD virtual const FontMetrics &GetMetrics(FontHandle handle) const = 0;

    NO_DISCARD virtual FontFamilyHandle GetDefaultFontFamily() const noexcept = 0;
  };
}
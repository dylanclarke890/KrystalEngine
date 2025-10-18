#pragma once

#include "Krystal.Gfx/Common.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Macros.hpp"

namespace Krys::Gfx
{
  class IFontRegistry
  {
    NO_COPY_MOVE(IFontRegistry)

  protected:
    IFontRegistry() = default;

  public:
    virtual ~IFontRegistry() noexcept = default;

    virtual void Startup() noexcept = 0;

    virtual void Shutdown() noexcept = 0;

    virtual FontHandle Load(const IO::Path &path, float size, FontType fontType) noexcept = 0;

    virtual void Unload(FontHandle handle) noexcept = 0;

    virtual void DPIChanged(int dpi) noexcept = 0;
  };
}
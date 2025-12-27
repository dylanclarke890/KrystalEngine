#pragma once

#include "Krystal.Gfx/ICamera.hpp"
#include "Krystal.Gfx/Registries/IBufferRegistry.hpp"
#include "Krystal.Gfx/Registries/IFontRegistry.hpp"
#include "Krystal.Gfx/Registries/IImageRegistry.hpp"
#include "Krystal.Gfx/Registries/IImageViewRegistry.hpp"
#include "Krystal.Gfx/Registries/IMaterialRegistry.hpp"
#include "Krystal.Gfx/Registries/IMeshRegistry.hpp"
#include "Krystal.Gfx/Registries/IRenderTargetRegistry.hpp"
#include "Krystal.Gfx/Registries/ISamplerRegistry.hpp"
#include "Krystal.Gfx/Registries/IShaderRegistry.hpp"
#include "Krystal.Gfx/Registries/ITextureRegistry.hpp"
#include "Krystal.IO/VirtualFileSystem.hpp"
#include "Krystal.Lib/Types/Expected.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/NativeHandle.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Lib/String/StringInterner.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Gfx
{
  struct ContextSettings
  {
    NativeHandle WindowHandle {nullptr};
    uint32 Width {0u};
    uint32 Height {0u};
    IO::VirtualFileSystem *VFS {nullptr};
    StringInterner *Strings {nullptr};
  };

  Expected<Unique<class IContext>> CreateContext(const ContextSettings &settings) noexcept;

  enum class API
  {
    None = 0,
    OpenGL,
    D3D11,
    D3D12,
    Vulkan,
  };

  class IContext
  {
    NO_COPY_MOVE(IContext)

  protected:
    IContext() noexcept = default;

  public:
    virtual ~IContext() noexcept = default;

    virtual void Startup() noexcept = 0;

    virtual void Shutdown() noexcept = 0;

    virtual void Render(ICamera &camera) noexcept = 0;

    virtual void Present() noexcept = 0;

    virtual void Resize(uint32 width, uint32 height) noexcept = 0;

    virtual void DPIChanged(int dpi) noexcept = 0;

    NO_DISCARD virtual IBufferRegistry &Buffers() noexcept = 0;

    NO_DISCARD virtual IImageRegistry &Images() noexcept = 0;

    NO_DISCARD virtual IImageViewRegistry &ImageViews() noexcept = 0;

    NO_DISCARD virtual ISamplerRegistry &Samplers() noexcept = 0;

    NO_DISCARD virtual ITextureRegistry &Textures() noexcept = 0;

    NO_DISCARD virtual IRenderTargetRegistry &RenderTargets() noexcept = 0;

    NO_DISCARD virtual IShaderRegistry &Shaders() noexcept = 0;

    NO_DISCARD virtual IMeshRegistry &Meshes() noexcept = 0;

    NO_DISCARD virtual IMaterialRegistry &Materials() noexcept = 0;

    NO_DISCARD virtual IFontRegistry &Fonts() noexcept = 0;

    NO_DISCARD virtual StringInterner &Strings() noexcept = 0;

    NO_DISCARD virtual API GetAPI() const noexcept = 0;
  };
}
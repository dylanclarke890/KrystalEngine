#pragma once

#include "Krystal.Gfx/ICamera.hpp"
#include "Krystal.IO/VirtualFileSystem.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/NativeHandle.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  class IContext;
  class IImageRegistry;
  class IImageViewRegistry;
  class ISamplerRegistry;
  class IShaderRegistry;
  class IMeshRegistry;
  class ITextureRegistry;
  class IMaterialRegistry;
  class IFontRegistry;

  struct ContextSettings
  {
    NativeHandle WindowHandle {nullptr};
    uint32 Width {0u};
    uint32 Height {0u};
    IO::VirtualFileSystem *VFS {nullptr};
  };

  Expected<Unique<IContext>> CreateContext(const ContextSettings& settings) noexcept;

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

    virtual void Setup() noexcept = 0;

    virtual void Render(ICamera &camera) noexcept = 0;

    virtual void Present() noexcept = 0;

    virtual void Resize(uint32 width, uint32 height) noexcept = 0;

    virtual void DPIChanged(int dpi) noexcept = 0;

    NO_DISCARD virtual IImageRegistry &Images() noexcept = 0;

    NO_DISCARD virtual IImageViewRegistry &ImageViews() noexcept = 0;

    NO_DISCARD virtual ISamplerRegistry &Samplers() noexcept = 0;

    NO_DISCARD virtual ITextureRegistry &Textures() noexcept = 0;

    NO_DISCARD virtual IShaderRegistry &Shaders() noexcept = 0;

    NO_DISCARD virtual IMeshRegistry &Meshes() noexcept = 0;

    NO_DISCARD virtual IMaterialRegistry &Materials() noexcept = 0;

    NO_DISCARD virtual IFontRegistry &Fonts() noexcept = 0;

    NO_DISCARD virtual API GetAPI() const noexcept = 0;
  };
}
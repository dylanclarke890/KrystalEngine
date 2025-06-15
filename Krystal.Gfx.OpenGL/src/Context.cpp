#include "Krystal.Gfx.OpenGL/Context.hpp"

#include "Krystal.Core/Core.hpp"
#include "Krystal.Core/Detection.hpp"
#include "Krystal.Gfx/IContext.hpp"

#ifdef KRYS_PLATFORM_WINDOWS
  #include "Krystal.Gfx.OpenGL/Win32/Impl.hpp"
#else
  #error "Unsupported platform for OpenGL context creation."
#endif

namespace Krys::Gfx
{
  Unique<IContext> CreateContext(NativeHandle windowHandle) noexcept
  {
    return Unique<OpenGLContext>(new OpenGLContext(windowHandle));
  }

  OpenGLContext::OpenGLContext(NativeHandle windowHandle) noexcept
      : IContext(), _windowHandle(windowHandle), _impl(CreateUnique<Impl>(windowHandle))
  {
  }

  void OpenGLContext::Initialise() noexcept
  {
    _impl->Initialise();
  }

  void OpenGLContext::Present() noexcept
  {
    _impl->Present();
  }
}

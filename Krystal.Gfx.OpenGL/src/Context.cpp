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
  Expected<Unique<IContext>> CreateContext(NativeHandle windowHandle) noexcept
  {
    try
    {
      return Expected<Unique<IContext>>(CreateUnique<OpenGLContext>(windowHandle));
    }
    catch (const std::exception &e)
    {
      return Unexpected(e.what());
    }
  }

  OpenGLContext::OpenGLContext(NativeHandle windowHandle)
      : _windowHandle(windowHandle), _impl(CreateUnique<Impl>(windowHandle))
  {
  }

  void OpenGLContext::Present() noexcept
  {
    _impl->Present();
  }
}

#include "Krystal.Gfx.OpenGL/UIRenderer.hpp"
#include "Krystal.Gfx.OpenGL/Context.hpp"

namespace Krys::Gfx
{
  Expected<Unique<IUIRenderer>> CreateUIRenderer(IContext &context) noexcept
  {
    try
    {
      return CreateUnique<OpenGL::UIRenderer>(context);
    }
    catch (const std::exception &e)
    {
      return Unexpected("Exception during OpenGL UI renderer creation: " + string(e.what()));
    }
  }
}

namespace Krys::Gfx::OpenGL
{
  UIRenderer::UIRenderer(IContext &context) noexcept : _context(static_cast<Context &>(context))
  {
  }
}
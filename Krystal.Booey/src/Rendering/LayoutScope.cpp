#include "Krystal.Booey/Rendering/LayoutScope.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"

namespace krys::boo
{
  LayoutScope::LayoutScope(RenderElement &renderer) noexcept : _renderer(renderer)
  {
  }

  LayoutScope::~LayoutScope() noexcept
  {
    if (auto *block = DynamicDowncast<RenderBlock>(_renderer))
    {
      block->UpdateScrollInfoAfterLayout();
    }
    _renderer.ClearNeedsLayout();
  }
}

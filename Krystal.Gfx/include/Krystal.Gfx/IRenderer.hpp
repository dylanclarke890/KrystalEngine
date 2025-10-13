#pragma once

#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx
{
  class IRenderer
  {
  public:
    IRenderer() noexcept = default;
    virtual ~IRenderer() noexcept = default;

    virtual void DrawRectangle(const Maths::Vec2 &position, const Maths::Vec2 &size,
                               const Colour &colour) = 0;
  };
}
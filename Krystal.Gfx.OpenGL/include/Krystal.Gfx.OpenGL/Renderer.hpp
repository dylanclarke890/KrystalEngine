#pragma once

#include "Krystal.Gfx/IRenderer.hpp"

namespace Krys::Gfx::OpenGL
{
  class Renderer : public IRenderer
  {
    ~Renderer() noexcept override = default;

    void DrawRectangle(const Maths::Vec2 &position, const Maths::Vec2 &size, const Colour &colour) override
    {
    }
  };
}
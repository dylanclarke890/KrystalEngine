#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx
{
  struct LightColour
  {
    Colourf Ambient;
    Colourf Diffuse;
    Colourf Specular;

    constexpr LightColour() noexcept
        : Ambient(Colourf(0.2f, 0.2f, 0.2f)), Diffuse(Colourf(0.5f, 0.5f, 0.5f)), Specular(Colourf(1.0f, 1.0f, 1.0f))
    {
    }

    constexpr LightColour(const Colourf &ambient, const Colourf &diffuse, const Colourf &specular) noexcept
        : Ambient(ambient), Diffuse(diffuse), Specular(specular)
    {
    }
  };

  struct LightAttenuation
  {
    float Constant;
    float Linear;
    float Quadratic;
    constexpr LightAttenuation() noexcept : Constant(1.0f), Linear(0.0f), Quadratic(0.0f)
    {
    }
    constexpr LightAttenuation(float constant, float linear, float quadratic) noexcept
        : Constant(constant), Linear(linear), Quadratic(quadratic)
    {
    }
  };

  struct DirectionalLight
  {
    Maths::Vec3 Direction;
    LightColour Colour;
  };

  struct PointLight
  {
    Maths::Vec3 Position;
    LightColour Colour;
    LightAttenuation Attenuation;
  };

  struct SpotLight
  {
    Maths::Vec3 Position;
    Maths::Vec3 Direction;
    LightColour Colour;
    LightAttenuation Attenuation;
    float CutOffRadians;
    float OuterCutOffRadians;
  };
}
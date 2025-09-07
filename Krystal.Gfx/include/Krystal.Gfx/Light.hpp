#pragma once

#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx
{
  struct LightColour
  {
    Colour Ambient;
    Colour Diffuse;
    Colour Specular;

    constexpr LightColour() noexcept
        : Ambient(Colour(0.2f, 0.2f, 0.2f)), Diffuse(Colour(0.5f, 0.5f, 0.5f)), Specular(Colour(1.0f, 1.0f, 1.0f))
    {
    }

    constexpr LightColour(const Colour &ambient, const Colour &diffuse, const Colour &specular) noexcept
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
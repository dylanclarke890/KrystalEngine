#pragma once

#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <variant>

namespace Krys::Gfx::OpenGL
{
  using MaterialParameterValue =
    std::variant<bool, int32, uint32, float, Maths::Vec2, Maths::Vec3, Maths::Vec4, TextureHandle>;

  enum class MaterialParameterType
  {
    None = 0,
    Bool,
    Int,
    UInt,
    Float,
    Vec2,
    Vec3,
    Vec4,
    Texture,
  };

  struct MaterialParameter
  {
    string Name;
    MaterialParameterType Type {MaterialParameterType::None};
    MaterialParameterValue Value;
  };

  struct Material : NonCopyable<Material>
  {
    string Name;
    ShaderHandle Shader;
    List<MaterialParameter> Parameters;

    Material(const string &name, ShaderHandle shader, const List<MaterialParameter> &parameters) noexcept
        : Name(name), Shader(shader), Parameters(parameters)
    {
    }

    Material(Material &&other) noexcept
        : Name(std::move(other.Name)), Shader(std::exchange(other.Shader, {})),
          Parameters(std::move(other.Parameters))
    {
    }

    Material &operator=(Material &&other) noexcept
    {
      if (this != &other)
      {
        Name = std::move(other.Name);
        Shader = std::exchange(other.Shader, {});
        Parameters = std::move(other.Parameters);
      }
      return *this;
    }
  };
}
#pragma once

#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Maths/Vector.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include <variant>

namespace krys::Gfx::OpenGL
{
  using MaterialParameterValue = std::variant<bool, int32, uint32, float, Vec2, Vec3, Vec4, TextureHandle>;

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

  struct Material
  {
    KRYS_NON_COPYABLE(Material);

  public:
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
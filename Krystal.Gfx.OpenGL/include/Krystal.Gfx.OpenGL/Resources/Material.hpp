#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
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

  struct Material
  {
    NO_COPY(Material)
    MOVE_SWAP(Material)

    Material(const string &name, ShaderHandle shader, const List<MaterialParameter> &parameters) noexcept
        : Name(name), Shader(shader), Parameters(parameters)
    {
    }

    string Name;
    ShaderHandle Shader;
    List<MaterialParameter> Parameters;

  private:
    void Swap(Material &other) noexcept
    {
      std::swap(Name, other.Name);
      std::swap(Shader, other.Shader);
      std::swap(Parameters, other.Parameters);
    }
  };

  class FlatColourMaterial
  {
  public:
    Colour Ambient {0.0f};
    Colour Diffuse {0.0f};
    Colour Specular {0.0f};
    float Shininess {32.0f};

    constexpr FlatColourMaterial() noexcept = default;

    constexpr FlatColourMaterial(const Colour &ambient, const Colour &diffuse, const Colour &specular,
                                 float shininess) noexcept
        : Ambient(ambient), Diffuse(diffuse), Specular(specular), Shininess(shininess)
    {
    }
  };

  namespace FlatColourMaterials
  {
    static FlatColourMaterial Emerald {Colour {0.0215f, 0.1745f, 0.0215f},
                                       Colour {0.07568f, 0.61424f, 0.07568f},
                                       Colour {0.633f, 0.727811f, 0.633f}, 0.6f * 128};

    static FlatColourMaterial Jade {Colour {0.135f, 0.2225f, 0.1575f}, Colour {0.54f, 0.89f, 0.63f},
                                    Colour {0.316228f, 0.316228f, 0.316228f}, 0.1f * 128};

    static FlatColourMaterial Obsidian {Colour {0.05375f, 0.05f, 0.06625f},
                                        Colour {0.18275f, 0.17f, 0.22525f},
                                        Colour {0.332741f, 0.328634f, 0.346435f}, 0.3f * 128};

    static FlatColourMaterial Pearl {Colour {0.25f, 0.20725f, 0.20725f}, Colour {1.0f, 0.829f, 0.829f},
                                     Colour {0.296648f, 0.296648f, 0.296648f}, 0.088f * 128};

    static FlatColourMaterial Ruby {Colour {0.1745f, 0.01175f, 0.01175f},
                                    Colour {0.61424f, 0.04136f, 0.04136f},
                                    Colour {0.727811f, 0.626959f, 0.626959f}, 0.6f * 128};

    static FlatColourMaterial Turquoise {Colour {0.1f, 0.18725f, 0.1745f},
                                         Colour {0.396f, 0.74151f, 0.69102f},
                                         Colour {0.297254f, 0.30829f, 0.306678f}, 0.1f * 128};

    static FlatColourMaterial Brass {Colour {0.329412f, 0.223529f, 0.027451f},
                                     Colour {0.780392f, 0.568627f, 0.113725f},
                                     Colour {0.992157f, 0.941176f, 0.807843f}, 0.21794872f * 128};

    static FlatColourMaterial Bronze {Colour {0.2125f, 0.1275f, 0.054f}, Colour {0.714f, 0.4284f, 0.18144f},
                                      Colour {0.393548f, 0.271906f, 0.166721f}, 0.2f * 128};

    static FlatColourMaterial Chrome {Colour {0.25f, 0.25f, 0.25f}, Colour {0.4f, 0.4f, 0.4f},
                                      Colour {0.774597f, 0.774597f, 0.774597f}, 0.6f * 128};

    static FlatColourMaterial Copper {Colour {0.19125f, 0.0735f, 0.0225f},
                                      Colour {0.7038f, 0.27048f, 0.0828f},
                                      Colour {0.256777f, 0.137622f, 0.086014f}, 0.1f * 128};

    static FlatColourMaterial Gold {Colour {0.24725f, 0.1995f, 0.0745f},
                                    Colour {0.75164f, 0.60648f, 0.22648f},
                                    Colour {0.628281f, 0.555802f, 0.366065f}, 0.4f * 128};

    static FlatColourMaterial Silver {Colour {0.19225f, 0.19225f, 0.19225f},
                                      Colour {0.50754f, 0.50754f, 0.50754f},
                                      Colour {0.508273f, 0.508273f, 0.508273f}, 0.4f * 128};

    static FlatColourMaterial BlackPlastic {Colour {0.0f, 0.0f, 0.0f}, Colour {0.01f, 0.01f, 0.01f},
                                            Colour {0.50f, 0.50f, 0.50f}, 0.25f * 128};

    static FlatColourMaterial CyanPlastic {Colour {0.0f, 0.1f, 0.06f},
                                           Colour {0.0f, 0.50980392f, 0.50980392f},
                                           Colour {0.50196078f, 0.50196078f, 0.50196078f}, 0.25f * 128};

    static FlatColourMaterial GreenPlastic {Colour {0.0f, 0.0f, 0.0f}, Colour {0.1f, 0.35f, 0.1f},
                                            Colour {0.45f, 0.55f, 0.45f}, 0.25f * 128};

    static FlatColourMaterial RedPlastic {Colour {0.0f, 0.0f, 0.0f}, Colour {0.5f, 0.0f, 0.0f},
                                          Colour {0.7f, 0.6f, 0.6f}, 0.25f * 128};

    static FlatColourMaterial WhitePlastic {Colour {0.0f, 0.0f, 0.0f}, Colour {0.55f, 0.55f, 0.55f},
                                            Colour {0.70f, 0.70f, 0.70f}, 0.25f * 128};

    static FlatColourMaterial YellowPlastic {Colour {0.0f, 0.0f, 0.0f}, Colour {0.5f, 0.5f, 0.0f},
                                             Colour {0.60f, 0.60f, 0.50f}, 0.25f * 128};

    static FlatColourMaterial BlackRubber {Colour {0.02f, 0.02f, 0.02f}, Colour {0.01f, 0.01f, 0.01f},
                                           Colour {0.4f, 0.4f, 0.4f}, 0.078125f * 128};

    static FlatColourMaterial CyanRubber {Colour {0.0f, 0.05f, 0.05f}, Colour {0.4f, 0.5f, 0.5f},
                                          Colour {0.04f, 0.7f, 0.7f}, 0.078125f * 128};

    static FlatColourMaterial GreenRubber {Colour {0.0f, 0.05f, 0.0f}, Colour {0.4f, 0.5f, 0.4f},
                                           Colour {0.04f, 0.7f, 0.04f}, 0.078125f * 128};

    static FlatColourMaterial RedRubber {Colour {0.05f, 0.0f, 0.0f}, Colour {0.5f, 0.4f, 0.4f},
                                         Colour {0.7f, 0.04f, 0.04f}, 0.078125f * 128};

    static FlatColourMaterial WhiteRubber {Colour {0.05f, 0.05f, 0.05f}, Colour {0.5f, 0.5f, 0.5f},
                                           Colour {0.7f, 0.7f, 0.7f}, 0.078125f * 128};

    static FlatColourMaterial YellowRubber {Colour {0.05f, 0.05f, 0.0f}, Colour {0.5f, 0.5f, 0.4f},
                                            Colour {0.7f, 0.7f, 0.04f}, 0.078125f * 128};
  }
}
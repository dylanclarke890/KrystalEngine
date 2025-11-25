#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Gfx/Handle.hpp"

namespace Krys::Gfx
{
  struct PBRMaterialDesc
  {
    Colourf AlbedoFactor {1.f, 1.f, 1.f, 1.f};
    float MetallicFactor {0.f};
    float RoughnessFactor {1.f};
    float AmbientOcclusionFactor {1.f};
    float NormalScale {1.f};
    Colourf EmissiveFactor {0.f};

    TextureHandle AlbedoMap {};
    TextureHandle MetallicMap {};
    TextureHandle RoughnessMap {};
    TextureHandle AmbientOcclusionMap {};
    TextureHandle NormalMap {};
    TextureHandle EmissiveMap {};
  };

  class FlatColourMaterial
  {
  public:
    Colourf Ambient {0.0f};
    Colourf Diffuse {0.0f};
    Colourf Specular {0.0f};
    float Shininess {32.0f};

    constexpr FlatColourMaterial() noexcept = default;

    constexpr FlatColourMaterial(const Colourf &ambient, const Colourf &diffuse, const Colourf &specular,
                                 float shininess) noexcept
        : Ambient(ambient), Diffuse(diffuse), Specular(specular), Shininess(shininess)
    {
    }
  };

  namespace FlatColourMaterials
  {
    static FlatColourMaterial Emerald {Colourf {0.0215f, 0.1745f, 0.0215f},
                                       Colourf {0.07568f, 0.61424f, 0.07568f},
                                       Colourf {0.633f, 0.727811f, 0.633f}, 0.6f * 128};

    static FlatColourMaterial Jade {Colourf {0.135f, 0.2225f, 0.1575f}, Colourf {0.54f, 0.89f, 0.63f},
                                    Colourf {0.316228f, 0.316228f, 0.316228f}, 0.1f * 128};

    static FlatColourMaterial Obsidian {Colourf {0.05375f, 0.05f, 0.06625f},
                                        Colourf {0.18275f, 0.17f, 0.22525f},
                                        Colourf {0.332741f, 0.328634f, 0.346435f}, 0.3f * 128};

    static FlatColourMaterial Pearl {Colourf {0.25f, 0.20725f, 0.20725f}, Colourf {1.0f, 0.829f, 0.829f},
                                     Colourf {0.296648f, 0.296648f, 0.296648f}, 0.088f * 128};

    static FlatColourMaterial Ruby {Colourf {0.1745f, 0.01175f, 0.01175f},
                                    Colourf {0.61424f, 0.04136f, 0.04136f},
                                    Colourf {0.727811f, 0.626959f, 0.626959f}, 0.6f * 128};

    static FlatColourMaterial Turquoise {Colourf {0.1f, 0.18725f, 0.1745f},
                                         Colourf {0.396f, 0.74151f, 0.69102f},
                                         Colourf {0.297254f, 0.30829f, 0.306678f}, 0.1f * 128};

    static FlatColourMaterial Brass {Colourf {0.329412f, 0.223529f, 0.027451f},
                                     Colourf {0.780392f, 0.568627f, 0.113725f},
                                     Colourf {0.992157f, 0.941176f, 0.807843f}, 0.21794872f * 128};

    static FlatColourMaterial Bronze {Colourf {0.2125f, 0.1275f, 0.054f}, Colourf {0.714f, 0.4284f, 0.18144f},
                                      Colourf {0.393548f, 0.271906f, 0.166721f}, 0.2f * 128};

    static FlatColourMaterial Chrome {Colourf {0.25f, 0.25f, 0.25f}, Colourf {0.4f, 0.4f, 0.4f},
                                      Colourf {0.774597f, 0.774597f, 0.774597f}, 0.6f * 128};

    static FlatColourMaterial Copper {Colourf {0.19125f, 0.0735f, 0.0225f},
                                      Colourf {0.7038f, 0.27048f, 0.0828f},
                                      Colourf {0.256777f, 0.137622f, 0.086014f}, 0.1f * 128};

    static FlatColourMaterial Gold {Colourf {0.24725f, 0.1995f, 0.0745f},
                                    Colourf {0.75164f, 0.60648f, 0.22648f},
                                    Colourf {0.628281f, 0.555802f, 0.366065f}, 0.4f * 128};

    static FlatColourMaterial Silver {Colourf {0.19225f, 0.19225f, 0.19225f},
                                      Colourf {0.50754f, 0.50754f, 0.50754f},
                                      Colourf {0.508273f, 0.508273f, 0.508273f}, 0.4f * 128};

    static FlatColourMaterial BlackPlastic {Colourf {0.0f, 0.0f, 0.0f}, Colourf {0.01f, 0.01f, 0.01f},
                                            Colourf {0.50f, 0.50f, 0.50f}, 0.25f * 128};

    static FlatColourMaterial CyanPlastic {Colourf {0.0f, 0.1f, 0.06f},
                                           Colourf {0.0f, 0.50980392f, 0.50980392f},
                                           Colourf {0.50196078f, 0.50196078f, 0.50196078f}, 0.25f * 128};

    static FlatColourMaterial GreenPlastic {Colourf {0.0f, 0.0f, 0.0f}, Colourf {0.1f, 0.35f, 0.1f},
                                            Colourf {0.45f, 0.55f, 0.45f}, 0.25f * 128};

    static FlatColourMaterial RedPlastic {Colourf {0.0f, 0.0f, 0.0f}, Colourf {0.5f, 0.0f, 0.0f},
                                          Colourf {0.7f, 0.6f, 0.6f}, 0.25f * 128};

    static FlatColourMaterial WhitePlastic {Colourf {0.0f, 0.0f, 0.0f}, Colourf {0.55f, 0.55f, 0.55f},
                                            Colourf {0.70f, 0.70f, 0.70f}, 0.25f * 128};

    static FlatColourMaterial YellowPlastic {Colourf {0.0f, 0.0f, 0.0f}, Colourf {0.5f, 0.5f, 0.0f},
                                             Colourf {0.60f, 0.60f, 0.50f}, 0.25f * 128};

    static FlatColourMaterial BlackRubber {Colourf {0.02f, 0.02f, 0.02f}, Colourf {0.01f, 0.01f, 0.01f},
                                           Colourf {0.4f, 0.4f, 0.4f}, 0.078125f * 128};

    static FlatColourMaterial CyanRubber {Colourf {0.0f, 0.05f, 0.05f}, Colourf {0.4f, 0.5f, 0.5f},
                                          Colourf {0.04f, 0.7f, 0.7f}, 0.078125f * 128};

    static FlatColourMaterial GreenRubber {Colourf {0.0f, 0.05f, 0.0f}, Colourf {0.4f, 0.5f, 0.4f},
                                           Colourf {0.04f, 0.7f, 0.04f}, 0.078125f * 128};

    static FlatColourMaterial RedRubber {Colourf {0.05f, 0.0f, 0.0f}, Colourf {0.5f, 0.4f, 0.4f},
                                         Colourf {0.7f, 0.04f, 0.04f}, 0.078125f * 128};

    static FlatColourMaterial WhiteRubber {Colourf {0.05f, 0.05f, 0.05f}, Colourf {0.5f, 0.5f, 0.5f},
                                           Colourf {0.7f, 0.7f, 0.7f}, 0.078125f * 128};

    static FlatColourMaterial YellowRubber {Colourf {0.05f, 0.05f, 0.0f}, Colourf {0.5f, 0.5f, 0.4f},
                                            Colourf {0.7f, 0.7f, 0.04f}, 0.078125f * 128};
  }
}
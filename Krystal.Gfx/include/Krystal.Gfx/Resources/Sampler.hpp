#pragma once

#include "Krystal.Lib/HashUtils.hpp"
#include "Krystal.Lib/Types.hpp"
#include <compare>
#include <type_traits>

namespace Krys::Gfx
{
  enum class FilterMode : uint8
  {
    Nearest = 0,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear,
  };

  enum class WrapMode : uint8
  {
    Repeat = 0,
    ClampToEdge,
    ClampToBorder,
  };

  struct SamplerDesc
  {
    FilterMode MinFilter {FilterMode::Linear};
    FilterMode MagFilter {FilterMode::Linear};
    WrapMode WrapS {WrapMode::Repeat};
    WrapMode WrapT {WrapMode::Repeat};
    WrapMode WrapR {WrapMode::Repeat};
    float AnisotropicLevel {1.0f};

    NO_DISCARD auto operator<=>(const SamplerDesc &other) const noexcept = default;

    constexpr static float DefaultAnisotropicLevel = 4.0f;

    constexpr static SamplerDesc MipmapLinearRepeat(float anisotropy = DefaultAnisotropicLevel)
    {
      return {
        .MinFilter = FilterMode::LinearMipmapLinear,
        .MagFilter = FilterMode::Linear,
        .WrapS = WrapMode::Repeat,
        .WrapT = WrapMode::Repeat,
        .WrapR = WrapMode::Repeat,
        .AnisotropicLevel = anisotropy,
      };
    }

    constexpr static SamplerDesc LinearRepeat(float anisotropy = DefaultAnisotropicLevel)
    {
      return {
        .MinFilter = FilterMode::Linear,
        .MagFilter = FilterMode::Linear,
        .WrapS = WrapMode::Repeat,
        .WrapT = WrapMode::Repeat,
        .WrapR = WrapMode::Repeat,
        .AnisotropicLevel = anisotropy,
      };
    }

    constexpr static SamplerDesc MipmapLinearClampToEdge(float anisotropy = DefaultAnisotropicLevel)
    {
      return {
        .MinFilter = FilterMode::LinearMipmapLinear,
        .MagFilter = FilterMode::Linear,
        .WrapS = WrapMode::ClampToEdge,
        .WrapT = WrapMode::ClampToEdge,
        .WrapR = WrapMode::ClampToEdge,
        .AnisotropicLevel = anisotropy,
      };
    }

    constexpr static SamplerDesc LinearClampToEdge(float anisotropy = DefaultAnisotropicLevel)
    {
      return {
        .MinFilter = FilterMode::Linear,
        .MagFilter = FilterMode::Linear,
        .WrapS = WrapMode::ClampToEdge,
        .WrapT = WrapMode::ClampToEdge,
        .WrapR = WrapMode::ClampToEdge,
        .AnisotropicLevel = anisotropy,
      };
    }

    constexpr static SamplerDesc MipmapNearestRepeat(float anisotropy = DefaultAnisotropicLevel)
    {
      return {
        .MinFilter = FilterMode::NearestMipmapNearest,
        .MagFilter = FilterMode::Nearest,
        .WrapS = WrapMode::Repeat,
        .WrapT = WrapMode::Repeat,
        .WrapR = WrapMode::Repeat,
        .AnisotropicLevel = anisotropy,
      };
    }

    constexpr static SamplerDesc NearestRepeat(float anisotropy = DefaultAnisotropicLevel)
    {
      return {
        .MinFilter = FilterMode::Nearest,
        .MagFilter = FilterMode::Nearest,
        .WrapS = WrapMode::Repeat,
        .WrapT = WrapMode::Repeat,
        .WrapR = WrapMode::Repeat,
        .AnisotropicLevel = anisotropy,
      };
    }

    constexpr static SamplerDesc MipmapNearestClampToEdge(float anisotropy = DefaultAnisotropicLevel)
    {
      return {
        .MinFilter = FilterMode::NearestMipmapNearest,
        .MagFilter = FilterMode::Nearest,
        .WrapS = WrapMode::ClampToEdge,
        .WrapT = WrapMode::ClampToEdge,
        .WrapR = WrapMode::ClampToEdge,
        .AnisotropicLevel = anisotropy,
      };
    }

    constexpr static SamplerDesc NearestClampToEdge(float anisotropy = DefaultAnisotropicLevel)
    {
      return {
        .MinFilter = FilterMode::Nearest,
        .MagFilter = FilterMode::Nearest,
        .WrapS = WrapMode::ClampToEdge,
        .WrapT = WrapMode::ClampToEdge,
        .WrapR = WrapMode::ClampToEdge,
        .AnisotropicLevel = anisotropy,
      };
    }
  };
}

namespace std
{
  template <>
  struct hash<Krys::Gfx::SamplerDesc>
  {
    size_t operator()(const Krys::Gfx::SamplerDesc &desc) const noexcept
    {
      return Krys::HashUtils::HashCombine(desc.MinFilter, desc.MagFilter, desc.WrapS, desc.WrapT, desc.WrapR,
                                    desc.AnisotropicLevel);
    }
  };
}
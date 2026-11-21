#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hashing/FastHash.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Gfx/CompareOp.hpp"
#include <functional>

namespace krys::gfx
{
  enum class SamplerFilter : uint8
  {
    Nearest,
    Linear,
  };

  enum class SamplerMipmapMode : uint8
  {
    /// @brief Only the base level is sampled.
    None,
    Nearest,
    Linear,
  };

  enum class SamplerAddressMode : uint8
  {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
  };

  enum class SamplerBorderColour : uint8
  {
    TransparentBlack,
    OpaqueBlack,
    OpaqueWhite,
  };

  /// @brief How a texture is sampled. Samplers are immutable; the device returns one sampler for equal
  /// descriptions.
  struct SamplerDesc
  {
    SamplerFilter MinFilter {SamplerFilter::Linear};
    SamplerFilter MagFilter {SamplerFilter::Linear};
    SamplerMipmapMode MipmapMode {SamplerMipmapMode::None};
    SamplerAddressMode AddressU {SamplerAddressMode::ClampToEdge};
    SamplerAddressMode AddressV {SamplerAddressMode::ClampToEdge};
    SamplerAddressMode AddressW {SamplerAddressMode::ClampToEdge};
    SamplerBorderColour BorderColour {SamplerBorderColour::TransparentBlack};
    /// @brief 1 disables anisotropic filtering.
    float MaxAnisotropy {1.f};
    float MinLod {0.f};
    float MaxLod {1000.f};
    float LodBias {0.f};
    /// @brief Makes this a comparison sampler (shadow maps): sampling returns the result of comparing the
    /// reference value with the texel using `Compare`.
    bool CompareEnabled {false};
    CompareOp Compare {CompareOp::LessOrEqual};

    constexpr bool operator==(const SamplerDesc &) const noexcept = default;

    KRYS_NODISCARD constexpr static SamplerDesc LinearClamp() noexcept
    {
      return {};
    }

    KRYS_NODISCARD constexpr static SamplerDesc NearestClamp() noexcept
    {
      return {.MinFilter = SamplerFilter::Nearest, .MagFilter = SamplerFilter::Nearest};
    }

    KRYS_NODISCARD constexpr static SamplerDesc LinearMipmappedRepeat(float maxAnisotropy = 1.f) noexcept
    {
      return {.MipmapMode = SamplerMipmapMode::Linear,
              .AddressU = SamplerAddressMode::Repeat,
              .AddressV = SamplerAddressMode::Repeat,
              .AddressW = SamplerAddressMode::Repeat,
              .MaxAnisotropy = maxAnisotropy};
    }
  };

  /// @brief Hashes a `SamplerDesc`, for deduplicating samplers.
  struct SamplerDescHash
  {
    KRYS_NODISCARD size_t operator()(const SamplerDesc &desc) const noexcept
    {
      return CombineFastHashes(desc.MinFilter, desc.MagFilter, desc.MipmapMode, desc.AddressU, desc.AddressV,
                               desc.AddressW, desc.BorderColour, desc.MaxAnisotropy, desc.MinLod, desc.MaxLod,
                               desc.LodBias, desc.CompareEnabled, desc.Compare);
    }
  };
}

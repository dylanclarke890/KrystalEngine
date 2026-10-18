#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Gfx/Formats.hpp"

namespace krys::gfx
{
  enum class TextureDimension : uint8
  {
    Texture1D,
    Texture2D,
    Texture2DArray,
    Texture3D,
    Cube,
  };

  /// @brief What a texture can be used as.
  enum class TextureUsage : uint8
  {
    None = 0,
    Sampled = 1 << 0,
    RenderTarget = 1 << 1,
    DepthStencil = 1 << 2,
    Storage = 1 << 3,
  };

  struct TextureDesc
  {
    TextureDimension Dimension {TextureDimension::Texture2D};
    TextureFormat Format {TextureFormat::RGBA8Unorm};
    uint32 Width {1u};
    uint32 Height {1u};
    /// @brief The depth of a 3D texture; 1 otherwise.
    uint32 Depth {1u};
    /// @brief The layers of an array texture; 6 for a cube; 1 otherwise.
    uint32 Layers {1u};
    uint32 MipLevels {1u};
    uint32 Samples {1u};
    TextureUsage Usage {TextureUsage::Sampled};
    utf8_stringview DebugName {};
  };

  /// @brief A box of texels in one mip level of a texture.
  struct TextureRegion
  {
    uint32 X {0u};
    uint32 Y {0u};
    /// @brief The first slice: a depth slice of a 3D texture, an array layer, or a cube face.
    uint32 Z {0u};
    uint32 Width {0u};
    uint32 Height {0u};
    /// @brief The number of slices.
    uint32 Depth {1u};
    uint32 MipLevel {0u};

    /// @brief The whole of a 2D mip level.
    KRYS_NODISCARD constexpr static TextureRegion Whole2D(uint32 width, uint32 height,
                                                          uint32 mipLevel = 0u) noexcept
    {
      return {.Width = width, .Height = height, .MipLevel = mipLevel};
    }
  };

  /// @brief The number of mip levels in a full chain for a texture of this size.
  KRYS_NODISCARD constexpr uint32 FullMipChainLength(uint32 width, uint32 height) noexcept
  {
    uint32 levels = 1u;
    uint32 size = width > height ? width : height;
    while (size > 1u)
    {
      size >>= 1u;
      ++levels;
    }
    return levels;
  }
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::gfx::TextureUsage, 4uz)

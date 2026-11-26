#include "Krystal.Gfx/Formats.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Gfx/Texture.hpp"

namespace krys::gfx::tests
{
  TEST_CASE("Formats: sizes and kinds", "[Gfx][Formats]")
  {
    STATIC_CHECK(BytesPerTexel(TextureFormat::R8Unorm) == 1u);
    STATIC_CHECK(BytesPerTexel(TextureFormat::BGRA8Srgb) == 4u);
    STATIC_CHECK(BytesPerTexel(TextureFormat::RGBA16Float) == 8u);
    STATIC_CHECK(BytesPerTexel(TextureFormat::RGBA32Float) == 16u);
    STATIC_CHECK(BytesPerTexel(TextureFormat::Depth24UnormStencil8) == 4u);

    STATIC_CHECK(IsDepthFormat(TextureFormat::Depth32Float));
    STATIC_CHECK_FALSE(IsDepthFormat(TextureFormat::R32Float));
    STATIC_CHECK(HasStencil(TextureFormat::Depth24UnormStencil8));
    STATIC_CHECK_FALSE(HasStencil(TextureFormat::Depth16Unorm));
    STATIC_CHECK(IsSrgbFormat(TextureFormat::RGBA8Srgb));
    STATIC_CHECK_FALSE(IsSrgbFormat(TextureFormat::RGBA8Unorm));
    STATIC_CHECK(IsIntegerFormat(TextureFormat::R32Uint));

    STATIC_CHECK(IndexSize(IndexType::UInt16) == 2u);
    STATIC_CHECK(IndexSize(IndexType::UInt32) == 4u);
    STATIC_CHECK(VertexFormatSize(VertexFormat::Float3) == 12u);
    STATIC_CHECK(VertexFormatSize(VertexFormat::UByte4Norm) == 4u);
  }

  TEST_CASE("Texture: mip chain length", "[Gfx][Texture]")
  {
    STATIC_CHECK(FullMipChainLength(1u, 1u) == 1u);
    STATIC_CHECK(FullMipChainLength(2u, 2u) == 2u);
    STATIC_CHECK(FullMipChainLength(256u, 64u) == 9u);
    STATIC_CHECK(FullMipChainLength(300u, 5u) == 9u);
  }
}

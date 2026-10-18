#include "Krystal.Gfx/Device.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Log/ILogger.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Utils/ByteUtils.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include <cstring>

namespace krys::gfx::tests
{
  namespace
  {
    /// @brief A triangle covering the target, with UVs where (0, 0) is the top left.
    constexpr const char8_t *FullscreenVertexSource = u8R"(#version 450
layout(binding = 1, std140) uniform Draw
{
  vec4 u_Colour;
  float u_Depth;
};

layout(location = 0) out vec2 v_UV;

void main()
{
  vec2 positions[3] = vec2[](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));
  vec2 position = positions[gl_VertexID];
  gl_Position = vec4(position, u_Depth, 1.0);
  v_UV = vec2(position.x * 0.5 + 0.5, 0.5 - position.y * 0.5);
}
)";

    constexpr const char8_t *PositionVertexSource = u8R"(#version 450
layout(location = 0) in vec2 a_Position;

void main()
{
  gl_Position = vec4(a_Position, 0.0, 1.0);
}
)";

    constexpr const char8_t *ColourFragmentSource = u8R"(#version 450
layout(binding = 1, std140) uniform Draw
{
  vec4 u_Colour;
  float u_Depth;
};

layout(location = 0) out vec4 o_Colour;

void main()
{
  o_Colour = u_Colour;
}
)";

    constexpr const char8_t *UniformFragmentSource = u8R"(#version 450
layout(binding = 0, std140) uniform Material
{
  vec4 u_MaterialColour;
};

layout(location = 0) out vec4 o_Colour;

void main()
{
  o_Colour = u_MaterialColour;
}
)";

    constexpr const char8_t *TextureFragmentSource = u8R"(#version 450
layout(binding = 0) uniform sampler2D u_Texture;
layout(location = 0) in vec2 v_UV;
layout(location = 0) out vec4 o_Colour;

void main()
{
  o_Colour = texture(u_Texture, v_UV);
}
)";

    struct DrawConstants
    {
      float Colour[4];
      float Depth;
      float Padding[3];
    };

    using Pixel = Array<uint8, 4>;

    struct PipelineOptions
    {
      TextureFormat Format {TextureFormat::RGBA8Unorm};
      BlendState Blend {};
      DepthStencilState DepthStencil {};
      TextureFormat DepthFormat {TextureFormat::Undefined};
      bool ScissorTest {false};
      bool PositionInput {false};
    };

    class DeviceFixture
    {
    protected:
      UniquePtr<Device> _device;
      List<BufferHandle> _buffers;
      List<TextureHandle> _textures;
      List<SamplerHandle> _samplers;
      List<ShaderHandle> _shaders;
      List<PipelineHandle> _pipelines;

    public:
      DeviceFixture()
      {
        // The device logs its debug output and leaks through the global logger.
        static const auto logger = []
        {
          auto created = log::CreateLogger({.Name = "Gfx.Tests", .FilePath = {}, .Level = log::Level::Warn});
          REQUIRE(created.has_value());
          log::SetGlobalLogger(*created);
          return krys::move(*created);
        }();

        auto device = Device::Create({.EnableValidation = true, .TransientMemoryPerFrame = 64u * 1'024u});
        if (!device)
        {
          FAIL("No OpenGL 4.5 device: " << device.error());
        }
        _device = krys::move(*device);
      }

      ~DeviceFixture()
      {
        for (auto pipeline : _pipelines)
        {
          _device->Destroy(pipeline);
        }
        for (auto shader : _shaders)
        {
          _device->Destroy(shader);
        }
        for (auto sampler : _samplers)
        {
          _device->Destroy(sampler);
        }
        for (auto texture : _textures)
        {
          _device->Destroy(texture);
        }
        for (auto buffer : _buffers)
        {
          _device->Destroy(buffer);
        }
      }

      /// @brief Fails the test if creation failed; otherwise destroys the resource when the test ends.
      template <typename THandle>
      KRYS_NODISCARD THandle Track(const Expected<THandle> &created)
      {
        if (!created)
        {
          FAIL(created.error());
        }
        if constexpr (SameType<THandle, BufferHandle>)
        {
          _buffers.push_back(*created);
        }
        else if constexpr (SameType<THandle, TextureHandle>)
        {
          _textures.push_back(*created);
        }
        else if constexpr (SameType<THandle, SamplerHandle>)
        {
          _samplers.push_back(*created);
        }
        else if constexpr (SameType<THandle, ShaderHandle>)
        {
          _shaders.push_back(*created);
        }
        else
        {
          _pipelines.push_back(*created);
        }
        return *created;
      }

      KRYS_NODISCARD ShaderHandle Shader(ShaderStage stage, const char8_t *source)
      {
        return Track(_device->CreateShader({.Stage = stage, .Source = source, .DebugName = u8"Test"}));
      }

      KRYS_NODISCARD PipelineHandle Pipeline(const char8_t *vertex, const char8_t *fragment,
                                             const PipelineOptions &options = {})
      {
        PipelineDesc desc {
          .VertexShader = Shader(ShaderStage::Vertex, vertex),
          .FragmentShader = Shader(ShaderStage::Fragment, fragment),
          .Blend = options.Blend,
          .DepthStencil = options.DepthStencil,
          .Raster = {.ScissorTest = options.ScissorTest},
          .ColourFormats = {options.Format},
          .DepthStencilFormat = options.DepthFormat,
          .DebugName = u8"Test",
        };
        if (options.PositionInput)
        {
          desc.VertexBuffers.push_back({.Stride = 8u, .Attributes = {{0u, VertexFormat::Float2, 0u}}});
        }
        return Track(_device->CreatePipeline(desc));
      }

      KRYS_NODISCARD TextureHandle Target(uint32 width, uint32 height,
                                          TextureFormat format = TextureFormat::RGBA8Unorm)
      {
        return Track(_device->CreateTexture({.Format = format,
                                             .Width = width,
                                             .Height = height,
                                             .Usage = TextureUsage::RenderTarget | TextureUsage::Sampled,
                                             .DebugName = u8"Target"}));
      }

      template <typename TFunc>
      void Frame(TFunc &&record)
      {
        _device->BeginFrame();
        auto &commands = _device->GetCommandBuffer();
        record(commands);
        _device->Submit(commands);
        _device->EndFrame();
      }

      /// @brief Copies the draw's constants into transient memory and binds them to the `Draw` block's slot.
      void Bind(CommandBuffer &commands, const DrawConstants &constants)
      {
        auto allocation = _device->AllocateTransient(sizeof(DrawConstants));
        REQUIRE(allocation.IsValid());
        std::memcpy(allocation.Data.data(), &constants, sizeof(DrawConstants));
        commands.BindUniformBuffer(1u, allocation.Buffer, allocation.Offset, sizeof(DrawConstants));
      }

      KRYS_NODISCARD List<Pixel> Read(TextureHandle texture, uint32 width, uint32 height,
                                      uint32 mipLevel = 0u)
      {
        List<Pixel> pixels(width * height);
        _device->ReadTexture(
          texture, TextureRegion::Whole2D(width, height, mipLevel),
          Span<byte>(reinterpret_cast<byte *>(pixels.data()), pixels.size() * sizeof(Pixel)));
        return pixels;
      }

      KRYS_NODISCARD static RenderPassDesc Pass(TextureHandle target,
                                                Array<float, 4> clear = {0.f, 0.f, 0.f, 0.f})
      {
        return RenderPassDesc::SingleColour({.Texture = target, .ClearColour = clear});
      }
    };

    KRYS_NODISCARD bool Near(const Pixel &pixel, const Pixel &expected, int tolerance = 1)
    {
      for (size_t i = 0u; i < 4u; ++i)
      {
        int difference = int(pixel[i]) - int(expected[i]);
        if (difference > tolerance || difference < -tolerance)
        {
          return false;
        }
      }
      return true;
    }

    constexpr Pixel Red {255, 0, 0, 255};
    constexpr Pixel Green {0, 255, 0, 255};
    constexpr Pixel Blue {0, 0, 255, 255};
    constexpr Pixel White {255, 255, 255, 255};
    constexpr Pixel Clear {0, 0, 0, 0};
  }

  TEST_CASE_METHOD(DeviceFixture, "Device: clears and reads back a target", "[Gfx][Device][GPU]")
  {
    auto target = Target(4u, 4u);
    Frame(
      [&](CommandBuffer &commands)
      {
        commands.BeginRenderPass(Pass(target, {1.f, 0.f, 0.f, 1.f}));
        commands.EndRenderPass();
      });

    for (const auto &pixel : Read(target, 4u, 4u))
    {
      CHECK(pixel == Red);
    }

    // sRGB targets encode the linear clear value.
    auto srgb = Target(2u, 2u, TextureFormat::RGBA8Srgb);
    Frame(
      [&](CommandBuffer &commands)
      {
        commands.BeginRenderPass(Pass(srgb, {0.5f, 0.5f, 0.5f, 1.f}));
        commands.EndRenderPass();
      });
    CHECK(Near(Read(srgb, 2u, 2u)[0], {188, 188, 188, 255}));
  }

  TEST_CASE_METHOD(DeviceFixture, "Device: draws with per-draw uniform data", "[Gfx][Device][GPU]")
  {
    auto target = Target(4u, 4u);
    auto pipeline = Pipeline(FullscreenVertexSource, ColourFragmentSource);
    Frame(
      [&](CommandBuffer &commands)
      {
        commands.BeginRenderPass(Pass(target));
        commands.BindPipeline(pipeline);
        Bind(commands, DrawConstants {.Colour = {0.f, 1.f, 0.f, 1.f}, .Depth = 0.f, .Padding = {}});
        commands.Draw(3u);
        // The binding stays set for later draws.
        commands.Draw(3u);
        commands.EndRenderPass();
      });

    for (const auto &pixel : Read(target, 4u, 4u))
    {
      CHECK(pixel == Green);
    }
  }

  TEST_CASE_METHOD(DeviceFixture, "Device: y points up in clip space and texture rows start at the top",
                   "[Gfx][Device][GPU]")
  {
    // A quad over the top half of clip space (y from 0 to 1).
    constexpr Array<float, 12> topHalf {-1.f, 0.f, 1.f, 0.f, 1.f, 1.f, -1.f, 0.f, 1.f, 1.f, -1.f, 1.f};
    auto vertices =
      Track(_device->CreateBuffer({.Size = sizeof(topHalf), .InitialData = ByteUtils::AsBytesView(topHalf)}));

    auto target = Target(4u, 4u);
    auto pipeline = Pipeline(PositionVertexSource, ColourFragmentSource, {.PositionInput = true});
    Frame(
      [&](CommandBuffer &commands)
      {
        commands.BeginRenderPass(Pass(target));
        commands.BindPipeline(pipeline);
        commands.BindVertexBuffer(0u, vertices);
        Bind(commands, DrawConstants {.Colour = {1.f, 0.f, 0.f, 1.f}, .Depth = 0.f, .Padding = {}});
        commands.Draw(6u);
        commands.EndRenderPass();
      });

    auto pixels = Read(target, 4u, 4u);
    CHECK(pixels[0] == Red);
    CHECK(pixels[1 * 4 + 3] == Red);
    CHECK(pixels[2 * 4 + 0] == Clear);
    CHECK(pixels[3 * 4 + 3] == Clear);
  }

  TEST_CASE_METHOD(DeviceFixture, "Device: samples textures with UV (0, 0) at the top left",
                   "[Gfx][Device][GPU]")
  {
    // Rows start at the top: red, green over blue, white.
    constexpr Array<Pixel, 4> texels {Red, Green, Blue, White};
    auto source = Track(_device->CreateTexture({.Width = 2u, .Height = 2u, .DebugName = u8"Source"}));
    _device->WriteTexture(source, TextureRegion::Whole2D(2u, 2u), ByteUtils::AsBytesView(texels));

    auto sampler = Track(_device->CreateSampler(SamplerDesc::NearestClamp()));

    // Rendering into a texture and reading it back keeps the image the right way up.
    auto target = Target(2u, 2u);
    auto pipeline = Pipeline(FullscreenVertexSource, TextureFragmentSource);
    Frame(
      [&](CommandBuffer &commands)
      {
        commands.BeginRenderPass(Pass(target));
        commands.BindPipeline(pipeline);
        Bind(commands, DrawConstants {});
        commands.BindTexture(0u, source, sampler);
        commands.Draw(3u);
        commands.EndRenderPass();
      });

    auto pixels = Read(target, 2u, 2u);
    CHECK(pixels[0] == Red);
    CHECK(pixels[1] == Green);
    CHECK(pixels[2] == Blue);
    CHECK(pixels[3] == White);
  }

  TEST_CASE_METHOD(DeviceFixture, "Device: binds uniform buffers from transient memory", "[Gfx][Device][GPU]")
  {
    auto target = Target(2u, 2u);
    auto pipeline = Pipeline(FullscreenVertexSource, UniformFragmentSource);
    Frame(
      [&](CommandBuffer &commands)
      {
        auto first = _device->AllocateTransient(16u);
        auto second = _device->AllocateTransient(16u);
        REQUIRE(first.IsValid());
        REQUIRE(second.IsValid());
        CHECK(second.Offset - first.Offset == 256u);

        constexpr Array<float, 4> colour {0.f, 0.f, 1.f, 1.f};
        std::memcpy(second.Data.data(), colour.data(), sizeof(colour));
        commands.BeginRenderPass(Pass(target));
        commands.BindPipeline(pipeline);
        Bind(commands, DrawConstants {});
        commands.BindUniformBuffer(0u, second.Buffer, second.Offset, 16u);
        commands.Draw(3u);
        commands.EndRenderPass();
      });

    CHECK(Read(target, 2u, 2u)[3] == Blue);
  }

  TEST_CASE_METHOD(DeviceFixture, "Device: draws indexed geometry", "[Gfx][Device][GPU]")
  {
    constexpr Array<float, 8> corners {-1.f, -1.f, 1.f, -1.f, 1.f, 1.f, -1.f, 1.f};
    constexpr Array<uint16, 6> indices {0u, 1u, 2u, 2u, 3u, 0u};
    auto vertices =
      Track(_device->CreateBuffer({.Size = sizeof(corners), .InitialData = ByteUtils::AsBytesView(corners)}));
    auto indexBuffer = Track(_device->CreateBuffer({.Size = sizeof(indices),
                                                    .Usage = BufferUsage::Index,
                                                    .InitialData = ByteUtils::AsBytesView(indices)}));

    auto target = Target(4u, 4u);
    auto pipeline = Pipeline(PositionVertexSource, ColourFragmentSource, {.PositionInput = true});
    Frame(
      [&](CommandBuffer &commands)
      {
        commands.BeginRenderPass(Pass(target));
        commands.BindPipeline(pipeline);
        commands.BindVertexBuffer(0u, vertices);
        commands.BindIndexBuffer(indexBuffer, IndexType::UInt16);
        Bind(commands, DrawConstants {.Colour = {1.f, 1.f, 1.f, 1.f}, .Depth = 0.f, .Padding = {}});
        commands.DrawIndexed(6u);
        commands.EndRenderPass();
      });

    for (const auto &pixel : Read(target, 4u, 4u))
    {
      CHECK(pixel == White);
    }
  }

  TEST_CASE_METHOD(DeviceFixture, "Device: scissor rectangles have a top-left origin", "[Gfx][Device][GPU]")
  {
    auto target = Target(4u, 4u);
    auto pipeline = Pipeline(FullscreenVertexSource, ColourFragmentSource, {.ScissorTest = true});
    Frame(
      [&](CommandBuffer &commands)
      {
        commands.BeginRenderPass(Pass(target));
        commands.BindPipeline(pipeline);
        commands.SetScissor(1, 0, 2u, 1u);
        Bind(commands, DrawConstants {.Colour = {1.f, 0.f, 0.f, 1.f}, .Depth = 0.f, .Padding = {}});
        commands.Draw(3u);
        commands.EndRenderPass();
      });

    auto pixels = Read(target, 4u, 4u);
    CHECK(pixels[0] == Clear);
    CHECK(pixels[1] == Red);
    CHECK(pixels[2] == Red);
    CHECK(pixels[3] == Clear);
    CHECK(pixels[4 + 1] == Clear);
  }

  TEST_CASE_METHOD(DeviceFixture, "Device: blends premultiplied colours", "[Gfx][Device][GPU]")
  {
    auto target = Target(2u, 2u);
    auto pipeline =
      Pipeline(FullscreenVertexSource, ColourFragmentSource, {.Blend = BlendState::PremultipliedAlpha()});
    Frame(
      [&](CommandBuffer &commands)
      {
        commands.BeginRenderPass(Pass(target, {0.f, 0.f, 1.f, 1.f}));
        commands.BindPipeline(pipeline);
        Bind(commands, DrawConstants {.Colour = {0.5f, 0.f, 0.f, 0.5f}, .Depth = 0.f, .Padding = {}});
        commands.Draw(3u);
        commands.EndRenderPass();
      });

    CHECK(Near(Read(target, 2u, 2u)[0], {128, 0, 128, 255}));
  }

  TEST_CASE_METHOD(DeviceFixture, "Device: tests depth", "[Gfx][Device][GPU]")
  {
    auto target = Target(2u, 2u);
    auto depth = Track(_device->CreateTexture({.Format = TextureFormat::Depth32Float,
                                               .Width = 2u,
                                               .Height = 2u,
                                               .Usage = TextureUsage::DepthStencil,
                                               .DebugName = u8"Depth"}));
    auto pipeline =
      Pipeline(FullscreenVertexSource, ColourFragmentSource,
               {.DepthStencil = {.DepthTest = true, .DepthWrite = true, .DepthCompare = CompareOp::Less},
                .DepthFormat = TextureFormat::Depth32Float});

    Frame(
      [&](CommandBuffer &commands)
      {
        auto pass = Pass(target);
        pass.DepthStencil = {.Texture = depth, .ClearDepth = 1.f};
        commands.BeginRenderPass(pass);
        commands.BindPipeline(pipeline);
        Bind(commands, DrawConstants {.Colour = {1.f, 0.f, 0.f, 1.f}, .Depth = 0.25f, .Padding = {}});
        commands.Draw(3u);
        // Further away, so it fails the test.
        Bind(commands, DrawConstants {.Colour = {0.f, 1.f, 0.f, 1.f}, .Depth = 0.75f, .Padding = {}});
        commands.Draw(3u);
        commands.EndRenderPass();
      });

    CHECK(Read(target, 2u, 2u)[0] == Red);
  }

  TEST_CASE_METHOD(DeviceFixture, "Device: copies buffers to textures and blits", "[Gfx][Device][GPU]")
  {
    constexpr Array<Pixel, 4> texels {Red, Green, Blue, White};
    auto staging = Track(_device->CreateBuffer({.Size = sizeof(texels),
                                                .Usage = BufferUsage::CopySource,
                                                .InitialData = ByteUtils::AsBytesView(texels)}));
    auto small = Target(2u, 2u);
    auto large = Target(4u, 4u);

    Frame(
      [&](CommandBuffer &commands)
      {
        commands.CopyBufferToTexture(staging, 0u, small, TextureRegion::Whole2D(2u, 2u));
        commands.BlitTexture(small, TextureRegion::Whole2D(2u, 2u), large, TextureRegion::Whole2D(4u, 4u),
                             SamplerFilter::Nearest);
      });

    auto copied = Read(small, 2u, 2u);
    CHECK(copied[0] == Red);
    CHECK(copied[3] == White);

    auto blitted = Read(large, 4u, 4u);
    CHECK(blitted[0] == Red);
    CHECK(blitted[1] == Red);
    CHECK(blitted[3] == Green);
    CHECK(blitted[3 * 4 + 0] == Blue);
    CHECK(blitted[3 * 4 + 3] == White);
  }

  TEST_CASE_METHOD(DeviceFixture, "Device: generates mipmaps", "[Gfx][Device][GPU]")
  {
    auto texture =
      Track(_device->CreateTexture({.Width = 4u, .Height = 4u, .MipLevels = 3u, .DebugName = u8"Mipped"}));
    Array<Pixel, 16> texels {};
    texels.fill(Green);
    _device->WriteTexture(texture, TextureRegion::Whole2D(4u, 4u), ByteUtils::AsBytesView(texels));
    _device->GenerateMipmaps(texture);
    CHECK(Read(texture, 1u, 1u, 2u)[0] == Green);
  }

  TEST_CASE_METHOD(DeviceFixture, "Device: reports errors", "[Gfx][Device][GPU]")
  {
    auto broken = _device->CreateShader({.Stage = ShaderStage::Fragment,
                                         .Source = u8"#version 450\nvoid main() { oops; }\n",
                                         .DebugName = u8"Broken"});
    REQUIRE_FALSE(broken.has_value());
    CHECK(broken.error().starts_with("Broken: "));
    CHECK(broken.error().size() > 8u);

    auto noVersion = _device->CreateShader(
      {.Stage = ShaderStage::Vertex, .Source = u8"void main() {}\n", .DebugName = u8"NoVersion"});
    CHECK_FALSE(noVersion.has_value());

    auto fragment = Shader(ShaderStage::Fragment, ColourFragmentSource);
    CHECK_FALSE(_device->CreatePipeline({.VertexShader = fragment, .FragmentShader = fragment}).has_value());

    CHECK_FALSE(_device->CreateBuffer({.Size = 0u}).has_value());
    CHECK_FALSE(_device->CreateTexture({.Width = 0u}).has_value());
    CHECK_FALSE(_device->CreateTexture({.Width = 4u, .Height = 4u, .MipLevels = 4u}).has_value());
    CHECK_FALSE(
      _device->CreateTexture({.Dimension = TextureDimension::Cube, .Width = 4u, .Height = 2u, .Layers = 6u})
        .has_value());
  }

  TEST_CASE_METHOD(DeviceFixture, "Device: shares samplers and expires handles", "[Gfx][Device][GPU]")
  {
    auto a = _device->CreateSampler(SamplerDesc::LinearClamp());
    auto b = _device->CreateSampler(SamplerDesc::LinearClamp());
    auto c = _device->CreateSampler(SamplerDesc::NearestClamp());
    REQUIRE((a.has_value() && b.has_value() && c.has_value()));
    CHECK(*a == *b);
    CHECK(*a != *c);

    // The shared sampler lives until both references are released.
    _device->Destroy(*a);
    auto again = _device->CreateSampler(SamplerDesc::LinearClamp());
    CHECK(*again == *a);
    _device->Destroy(*again);
    _device->Destroy(*b);
    auto fresh = _device->CreateSampler(SamplerDesc::LinearClamp());
    CHECK(*fresh != *a);
    _device->Destroy(*fresh);
    _device->Destroy(*c);

    auto texture = _device->CreateTexture({.Width = 2u, .Height = 2u});
    REQUIRE(texture.has_value());
    REQUIRE(_device->GetTextureDesc(*texture).has_value());
    CHECK(_device->GetTextureDesc(*texture)->Width == 2u);
    _device->Destroy(*texture);
    CHECK_FALSE(_device->GetTextureDesc(*texture).has_value());
  }
}

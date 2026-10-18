#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"

namespace krys::gfx::tests
{
  namespace
  {
    using Pixel = Array<uint8, 4>;

    constexpr Pixel Red {255, 0, 0, 255};
    constexpr Pixel Green {0, 255, 0, 255};
    constexpr Pixel Clear {0, 0, 0, 0};

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

    class Renderer2DFixture
    {
    protected:
      UniquePtr<Device> _device;
      UniquePtr<Renderer2D> _renderer;
      TextureHandle _target {};
      uint32 _width {4u};
      uint32 _height {4u};
      TextureFormat _format {TextureFormat::RGBA8Unorm};

    public:
      Renderer2DFixture() : _device(CreateTestDevice())
      {
        auto renderer = Renderer2D::Create(*_device);
        if (!renderer)
        {
          FAIL(renderer.error());
        }
        _renderer = krys::move(*renderer);
      }

      ~Renderer2DFixture()
      {
        _renderer = nullptr;
        if (_target.IsValid())
        {
          _device->Destroy(_target);
        }
      }

      void UseTarget(uint32 width, uint32 height, TextureFormat format = TextureFormat::RGBA8Unorm)
      {
        if (_target.IsValid())
        {
          _device->Destroy(_target);
        }
        auto target = _device->CreateTexture({.Format = format,
                                              .Width = width,
                                              .Height = height,
                                              .Usage = TextureUsage::RenderTarget | TextureUsage::Sampled,
                                              .DebugName = u8"Target"});
        REQUIRE(target.has_value());
        _target = *target;
        _width = width;
        _height = height;
        _format = format;
      }

      /// @brief Draws one 2D frame into the target; it's cleared first when `clear` is set.
      template <typename TFunc>
      void Draw(TFunc &&draw, Maybe<Array<float, 4>> clear = Array<float, 4> {0.f, 0.f, 0.f, 0.f})
      {
        _device->BeginFrame();
        auto &commands = _device->GetCommandBuffer();
        _renderer->Begin(
          commands,
          {.Texture = _target, .Width = _width, .Height = _height, .Format = _format, .ClearColour = clear});
        draw(*_renderer);
        _renderer->End();
        _device->Submit(commands);
        _device->EndFrame();
      }

      KRYS_NODISCARD List<Pixel> Read()
      {
        List<Pixel> pixels(_width * _height);
        _device->ReadTexture(
          _target, TextureRegion::Whole2D(_width, _height),
          Span<byte>(reinterpret_cast<byte *>(pixels.data()), pixels.size() * sizeof(Pixel)));
        return pixels;
      }

      KRYS_NODISCARD Pixel At(const List<Pixel> &pixels, uint32 x, uint32 y) const
      {
        return pixels[y * _width + x];
      }
    };
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: fills rects in pixels from the top left",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(4u, 4u);
    Draw([](Renderer2D &renderer) { renderer.FillRect({1.f, 0.f, 2.f, 1.f}, SRGBA<uint8> {255, 0, 0}); });

    auto pixels = Read();
    CHECK(At(pixels, 0u, 0u) == Clear);
    CHECK(At(pixels, 1u, 0u) == Red);
    CHECK(At(pixels, 2u, 0u) == Red);
    CHECK(At(pixels, 3u, 0u) == Clear);
    CHECK(At(pixels, 1u, 1u) == Clear);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: draws in painter's order", "[Gfx][Render2D][GPU]")
  {
    UseTarget(4u, 4u);
    Draw(
      [](Renderer2D &renderer)
      {
        renderer.FillRect({0.f, 0.f, 4.f, 4.f}, SRGBA<uint8> {0, 255, 0});
        renderer.FillRect({0.f, 0.f, 2.f, 2.f}, SRGBA<uint8> {255, 0, 0});
        // Empty and fully transparent rects draw nothing.
        renderer.FillRect({0.f, 0.f, 0.f, 4.f}, SRGBA<uint8> {0, 0, 255});
        renderer.FillRect({0.f, 0.f, 4.f, 4.f}, SRGBA<uint8> {0, 0, 255, 0});
      });

    auto pixels = Read();
    CHECK(At(pixels, 0u, 0u) == Red);
    CHECK(At(pixels, 1u, 1u) == Red);
    CHECK(At(pixels, 2u, 0u) == Green);
    CHECK(At(pixels, 3u, 3u) == Green);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: blends with straight alpha colours",
                   "[Gfx][Render2D][GPU]")
  {
    // A unorm target blends the encoded values: half red over blue.
    UseTarget(2u, 2u);
    Draw([](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 2.f, 2.f}, SRGBA<uint8> {255, 0, 0, 128}); },
         Array<float, 4> {0.f, 0.f, 1.f, 1.f});
    CHECK(Near(Read()[0], {128, 0, 127, 255}));

    // An sRGB target blends in linear light: half white over black is half the light, encoded as 188.
    UseTarget(2u, 2u, TextureFormat::RGBA8Srgb);
    Draw([](Renderer2D &renderer)
         { renderer.FillRect({0.f, 0.f, 2.f, 2.f}, SRGBA<uint8> {255, 255, 255, 128}); },
         Array<float, 4> {0.f, 0.f, 0.f, 1.f});
    CHECK(Near(Read()[0], {188, 188, 188, 255}));

    // Opaque colours come back as they went in.
    Draw([](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 2.f, 2.f}, SRGBA<uint8> {200, 100, 50}); });
    CHECK(Near(Read()[0], {200, 100, 50, 255}));
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: keeps the target's contents without a clear colour",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(4u, 4u);
    Draw([](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 4.f, 4.f}, SRGBA<uint8> {0, 255, 0}); });
    Draw([](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 1.f, 1.f}, SRGBA<uint8> {255, 0, 0}); },
         Maybe<Array<float, 4>> {});

    auto pixels = Read();
    CHECK(At(pixels, 0u, 0u) == Red);
    CHECK(At(pixels, 3u, 3u) == Green);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: draws many rects in one frame", "[Gfx][Render2D][GPU]")
  {
    UseTarget(16u, 16u);
    Draw(
      [](Renderer2D &renderer)
      {
        for (uint32 y = 0u; y < 16u; ++y)
        {
          for (uint32 x = 0u; x < 16u; ++x)
          {
            const bool red = (x + y) % 2u == 0u;
            renderer.FillRect({float(x), float(y), 1.f, 1.f},
                              red ? SRGBA<uint8> {255, 0, 0} : SRGBA<uint8> {0, 255, 0});
          }
        }
      });

    auto pixels = Read();
    CHECK(At(pixels, 0u, 0u) == Red);
    CHECK(At(pixels, 1u, 0u) == Green);
    CHECK(At(pixels, 15u, 15u) == Red);
    CHECK(At(pixels, 14u, 15u) == Green);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: antialiases edges between pixels", "[Gfx][Render2D][GPU]")
  {
    // The left edge at x = 0.5 and the right edge at x = 1.5 each cover half of a pixel.
    UseTarget(4u, 4u);
    Draw([](Renderer2D &renderer) { renderer.FillRect({0.5f, 0.f, 1.f, 4.f}, SRGBA<uint8> {255, 0, 0}); });

    auto pixels = Read();
    CHECK(Near(At(pixels, 0u, 0u), {128, 0, 0, 128}, 2));
    CHECK(Near(At(pixels, 1u, 2u), {128, 0, 0, 128}, 2));
    CHECK(At(pixels, 2u, 0u) == Clear);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: fills rounded rects", "[Gfx][Render2D][GPU]")
  {
    UseTarget(8u, 8u);
    Draw(
      [](Renderer2D &renderer)
      {
        renderer.FillRoundedRect({{0.f, 0.f, 8.f, 8.f}, CornerRadii2D::Uniform(4.f)},
                                 SRGBA<uint8> {255, 0, 0});
      });

    auto circle = Read();
    // The corner pixel is outside the curve; the pixel below it is about a fifth covered.
    CHECK(At(circle, 0u, 0u) == Clear);
    CHECK(Near(At(circle, 0u, 1u), {51, 0, 0, 51}, 3));
    CHECK(At(circle, 4u, 4u) == Red);
    CHECK(At(circle, 7u, 7u) == Clear);

    SECTION("Radii that don't fit are scaled down together")
    {
      Draw(
        [](Renderer2D &renderer)
        {
          renderer.FillRoundedRect({{0.f, 0.f, 8.f, 8.f}, CornerRadii2D::Uniform(100.f)},
                                   SRGBA<uint8> {255, 0, 0});
        });
      CHECK(Read() == circle);
    }

    SECTION("Each corner has its own radius; negative radii are square")
    {
      Draw(
        [](Renderer2D &renderer)
        {
          renderer.FillRoundedRect(
            {{0.f, 0.f, 8.f, 8.f}, {.TopLeft = 4.f, .TopRight = 0.f, .BottomRight = -5.f, .BottomLeft = 0.f}},
            SRGBA<uint8> {255, 0, 0});
        });

      auto pixels = Read();
      CHECK(At(pixels, 0u, 0u) == Clear);
      CHECK(At(pixels, 7u, 0u) == Red);
      CHECK(At(pixels, 7u, 7u) == Red);
      CHECK(At(pixels, 0u, 7u) == Red);
    }
  }
}

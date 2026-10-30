#include "Krystal.Gfx/Render2D/FilterRenderer2D.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"

namespace krys::gfx::tests
{
  namespace
  {
    using FilterPixel = Array<uint8, 4>;

    constexpr FilterPixel FilterRed {255, 0, 0, 255};
    constexpr FilterPixel FilterGreen {0, 255, 0, 255};
    constexpr FilterPixel FilterWhite {255, 255, 255, 255};
    constexpr FilterPixel FilterClear {0, 0, 0, 0};

    KRYS_NODISCARD bool FilterNear(const FilterPixel &pixel, const FilterPixel &expected, int tolerance = 1)
    {
      for (size_t i = 0u; i < 4u; ++i)
      {
        const int difference = int(pixel[i]) - int(expected[i]);
        if (difference > tolerance || difference < -tolerance)
        {
          return false;
        }
      }
      return true;
    }

    class FilterRenderer2DFixture
    {
    protected:
      UniquePtr<Device> _device;
      UniquePtr<FilterRenderer2D> _renderer;
      List<TextureHandle> _images {};
      PixelRect2D _region {};

    public:
      FilterRenderer2DFixture() : _device(CreateTestDevice())
      {
        auto renderer = FilterRenderer2D::Create(*_device);
        if (!renderer)
        {
          FAIL(renderer.error());
        }
        _renderer = krys::move(*renderer);
      }

      ~FilterRenderer2DFixture()
      {
        _renderer = nullptr;
        for (auto image : _images)
        {
          _device->Destroy(image);
        }
      }

      /// @brief A source holding `texels`, rows from the top, over `region`.
      KRYS_NODISCARD FilterSource2D Source(const PixelRect2D &region, Span<const FilterPixel> texels,
                                           bool premultiplied = true)
      {
        auto image = _device->CreateTexture({.Format = TextureFormat::RGBA8Unorm,
                                             .Width = region.Width,
                                             .Height = region.Height,
                                             .DebugName = u8"Image"});
        REQUIRE(image.has_value());
        _device->WriteTexture(
          *image, TextureRegion::Whole2D(region.Width, region.Height),
          Span<const byte>(reinterpret_cast<const byte *>(texels.data()), texels.size_bytes()));
        _images.push_back(*image);
        return {.Texture = *image, .Region = region, .Premultiplied = premultiplied};
      }

      /// @brief Renders `graph` and reads its result, rows from the top.
      KRYS_NODISCARD List<FilterPixel> Run(const FilterGraph2D &graph)
      {
        _device->BeginFrame();
        auto &commands = _device->GetCommandBuffer();
        auto result = _renderer->Render(commands, graph);
        _device->Submit(commands);
        _device->EndFrame();
        REQUIRE(result.has_value());
        _region = result->Region;
        List<FilterPixel> pixels(static_cast<size_t>(_region.Width) * _region.Height);
        _device->ReadTexture(
          result->Texture, TextureRegion::Whole2D(_region.Width, _region.Height),
          Span<byte>(reinterpret_cast<byte *>(pixels.data()), pixels.size() * sizeof(FilterPixel)));
        return pixels;
      }

      KRYS_NODISCARD FilterPixel At(const List<FilterPixel> &pixels, uint32 x, uint32 y) const
      {
        return pixels[y * _region.Width + x];
      }
    };

    /// @brief A 4 by 4 image, clear but for `pixel` at `x`, `y`.
    KRYS_NODISCARD List<FilterPixel> OnePixel(uint32 x, uint32 y, const FilterPixel &pixel)
    {
      List<FilterPixel> texels(16u, FilterClear);
      texels[y * 4u + x] = pixel;
      return texels;
    }

    constexpr PixelRect2D Square {.X = 0, .Y = 0, .Width = 4u, .Height = 4u};
  }

  TEST_CASE_METHOD(FilterRenderer2DFixture, "FilterRenderer2D: an empty graph has no result",
                   "[Render2D][Filter]")
  {
    _device->BeginFrame();
    auto &commands = _device->GetCommandBuffer();
    CHECK_FALSE(_renderer->Render(commands, {}).has_value());
    _device->Submit(commands);
    _device->EndFrame();
  }

  TEST_CASE_METHOD(FilterRenderer2DFixture, "FilterRenderer2D: flood fills its region", "[Render2D][Filter]")
  {
    FilterGraph2D graph;
    graph.Nodes.push_back({.Type = FilterNode2D::Kind::Flood,
                           .Region = {.X = 5, .Y = 7, .Width = 3u, .Height = 2u},
                           .Colour = {0.f, 0.5f, 0.f, 0.5f}});
    const auto pixels = Run(graph);
    CHECK(_region.X == 5);
    CHECK(_region.Y == 7);
    for (const auto &pixel : pixels)
    {
      CHECK(FilterNear(pixel, {0, 128, 0, 128}));
    }
  }

  TEST_CASE_METHOD(FilterRenderer2DFixture, "FilterRenderer2D: offset moves the input, and reads its region",
                   "[Render2D][Filter]")
  {
    FilterGraph2D graph;
    const auto texels = OnePixel(0u, 0u, FilterRed);
    graph.Sources.push_back(Source({.X = 10, .Y = 20, .Width = 4u, .Height = 4u}, texels));

    SECTION("by whole pixels")
    {
      graph.Nodes.push_back({.Type = FilterNode2D::Kind::Offset,
                             .Inputs = {FilterInput2D::Source(0u)},
                             .Region = {.X = 10, .Y = 20, .Width = 4u, .Height = 4u},
                             .Offset = {2.f, 1.f}});
      const auto pixels = Run(graph);
      CHECK(FilterNear(At(pixels, 2u, 1u), FilterRed));
      CHECK(FilterNear(At(pixels, 0u, 0u), FilterClear));
    }
    SECTION("by half a pixel")
    {
      graph.Nodes.push_back({.Type = FilterNode2D::Kind::Offset,
                             .Inputs = {FilterInput2D::Source(0u)},
                             .Region = {.X = 10, .Y = 20, .Width = 4u, .Height = 4u},
                             .Offset = {0.5f, 0.f}});
      const auto pixels = Run(graph);
      CHECK(FilterNear(At(pixels, 0u, 0u), {128, 0, 0, 128}));
      CHECK(FilterNear(At(pixels, 1u, 0u), {128, 0, 0, 128}));
    }
    SECTION("outside the source is transparent")
    {
      graph.Nodes.push_back({.Type = FilterNode2D::Kind::Offset,
                             .Inputs = {FilterInput2D::Source(0u)},
                             .Region = {.X = 6, .Y = 20, .Width = 4u, .Height = 4u}});
      const auto pixels = Run(graph);
      for (const auto &pixel : pixels)
      {
        CHECK(FilterNear(pixel, FilterClear));
      }
    }
  }

  TEST_CASE_METHOD(FilterRenderer2DFixture, "FilterRenderer2D: colour matrix and component transfer",
                   "[Render2D][Filter]")
  {
    FilterGraph2D graph;
    const List<FilterPixel> texels(16u, {255, 0, 0, 128});
    graph.Sources.push_back(Source(Square, texels, false));

    SECTION("the matrix moves red to green on straight colours")
    {
      FilterNode2D node {
        .Type = FilterNode2D::Kind::ColourMatrix, .Inputs = {FilterInput2D::Source(0u)}, .Region = Square};
      node.Matrix = {0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f,
                     0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f};
      graph.Nodes.push_back(node);
      CHECK(FilterNear(Run(graph)[5], {0, 128, 0, 128}));
    }
    SECTION("the table inverts each channel")
    {
      FilterNode2D node {.Type = FilterNode2D::Kind::ComponentTransfer,
                         .Inputs = {FilterInput2D::Source(0u)},
                         .Region = Square};
      for (uint32 i = 0u; i < 256u; ++i)
      {
        const auto inverse = static_cast<uint8>(255u - i);
        node.Table.push_back({inverse, inverse, inverse, static_cast<uint8>(i)});
      }
      graph.Nodes.push_back(node);
      // Straight (0, 255, 255) at half alpha.
      CHECK(FilterNear(Run(graph)[5], {0, 128, 128, 128}));
    }
  }

  TEST_CASE_METHOD(FilterRenderer2DFixture, "FilterRenderer2D: inputs convert between colour spaces",
                   "[Render2D][Filter]")
  {
    FilterGraph2D graph;
    const List<FilterPixel> texels(16u, {128, 128, 128, 255});
    graph.Sources.push_back(Source(Square, texels));
    graph.Nodes.push_back({.Type = FilterNode2D::Kind::Offset,
                           .Inputs = {FilterInput2D::Source(0u)},
                           .Region = Square,
                           .ColourSpace = FilterColourSpace2D::LinearSRGB});

    SECTION("to linear")
    {
      CHECK(FilterNear(Run(graph)[0], {55, 55, 55, 255}));
    }
    SECTION("and back")
    {
      graph.Nodes.push_back(
        {.Type = FilterNode2D::Kind::Offset, .Inputs = {FilterInput2D::Node(0u)}, .Region = Square});
      // Linear 8-bit values lose some precision.
      CHECK(FilterNear(Run(graph)[0], {128, 128, 128, 255}, 3));
    }
  }

  TEST_CASE_METHOD(FilterRenderer2DFixture, "FilterRenderer2D: blend and composite", "[Render2D][Filter]")
  {
    FilterGraph2D graph;
    const List<FilterPixel> red(16u, FilterRed);
    const auto green = OnePixel(1u, 1u, FilterGreen);
    graph.Sources.push_back(Source(Square, red));
    graph.Sources.push_back(Source(Square, green));

    SECTION("multiply")
    {
      graph.Nodes.push_back({.Type = FilterNode2D::Kind::Blend,
                             .Inputs = {FilterInput2D::Source(0u), FilterInput2D::Source(1u)},
                             .Region = Square,
                             .Blend = BlendMode::Multiply});
      const auto pixels = Run(graph);
      CHECK(FilterNear(At(pixels, 1u, 1u), {0, 0, 0, 255}));
      CHECK(FilterNear(At(pixels, 0u, 0u), FilterRed));
    }
    SECTION("in")
    {
      graph.Nodes.push_back({.Type = FilterNode2D::Kind::Composite,
                             .Inputs = {FilterInput2D::Source(0u), FilterInput2D::Source(1u)},
                             .Region = Square,
                             .Operator = FilterCompositeOperator2D::In});
      const auto pixels = Run(graph);
      CHECK(FilterNear(At(pixels, 1u, 1u), FilterRed));
      CHECK(FilterNear(At(pixels, 0u, 0u), FilterClear));
    }
    SECTION("arithmetic")
    {
      graph.Nodes.push_back({.Type = FilterNode2D::Kind::Composite,
                             .Inputs = {FilterInput2D::Source(0u), FilterInput2D::Source(1u)},
                             .Region = Square,
                             .Operator = FilterCompositeOperator2D::Arithmetic,
                             .K = {0.f, 0.5f, 0.5f, 0.f}});
      const auto pixels = Run(graph);
      CHECK(FilterNear(At(pixels, 1u, 1u), {128, 128, 0, 255}));
      CHECK(FilterNear(At(pixels, 0u, 0u), {128, 0, 0, 128}));
    }
  }

  TEST_CASE_METHOD(FilterRenderer2DFixture, "FilterRenderer2D: blur spreads and keeps its total",
                   "[Render2D][Filter]")
  {
    FilterGraph2D graph;
    List<FilterPixel> texels(81u, FilterClear);
    texels[4u * 9u + 4u] = FilterWhite;
    graph.Sources.push_back(Source({.X = 0, .Y = 0, .Width = 9u, .Height = 9u}, texels));
    graph.Nodes.push_back({.Type = FilterNode2D::Kind::Blur,
                           .Inputs = {FilterInput2D::Source(0u)},
                           .Region = {.X = 0, .Y = 0, .Width = 9u, .Height = 9u},
                           .StandardDeviation = {1.f, 1.f}});
    const auto pixels = Run(graph);
    const auto centre = At(pixels, 4u, 4u);
    CHECK(centre[3] > At(pixels, 5u, 4u)[3]);
    CHECK(At(pixels, 3u, 4u) == At(pixels, 5u, 4u));
    CHECK(At(pixels, 4u, 3u) == At(pixels, 4u, 5u));
    CHECK(At(pixels, 3u, 4u) == At(pixels, 4u, 3u));
    int total = 0;
    for (const auto &pixel : pixels)
    {
      total += pixel[3];
    }
    CHECK(total > 245);
    CHECK(total < 265);
  }

  TEST_CASE_METHOD(FilterRenderer2DFixture, "FilterRenderer2D: morphology dilates and erodes",
                   "[Render2D][Filter]")
  {
    FilterGraph2D graph;
    List<FilterPixel> texels(25u, FilterClear);
    texels[2u * 5u + 2u] = FilterRed;
    const PixelRect2D region {.X = 0, .Y = 0, .Width = 5u, .Height = 5u};
    graph.Sources.push_back(Source(region, texels));
    graph.Nodes.push_back({.Type = FilterNode2D::Kind::Morphology,
                           .Inputs = {FilterInput2D::Source(0u)},
                           .Region = region,
                           .Radius = {1, 1},
                           .Dilate = true});

    SECTION("dilating grows the pixel to a square")
    {
      const auto pixels = Run(graph);
      CHECK(FilterNear(pixels[1u * 5u + 1u], FilterRed));
      CHECK(FilterNear(pixels[3u * 5u + 3u], FilterRed));
      CHECK(FilterNear(pixels[0u], FilterClear));
    }
    SECTION("eroding that square leaves the pixel")
    {
      graph.Nodes.push_back({.Type = FilterNode2D::Kind::Morphology,
                             .Inputs = {FilterInput2D::Node(0u)},
                             .Region = region,
                             .Radius = {1, 1}});
      const auto pixels = Run(graph);
      CHECK(FilterNear(pixels[2u * 5u + 2u], FilterRed));
      CHECK(FilterNear(pixels[1u * 5u + 1u], FilterClear));
    }
  }

  TEST_CASE_METHOD(FilterRenderer2DFixture, "FilterRenderer2D: convolve matrix", "[Render2D][Filter]")
  {
    FilterGraph2D graph;
    const auto texels = OnePixel(1u, 1u, FilterRed);
    graph.Sources.push_back(Source(Square, texels));
    FilterNode2D node {
      .Type = FilterNode2D::Kind::ConvolveMatrix, .Inputs = {FilterInput2D::Source(0u)}, .Region = Square};

    SECTION("an identity kernel copies")
    {
      node.Kernel = {0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f};
      graph.Nodes.push_back(node);
      const auto pixels = Run(graph);
      CHECK(FilterNear(At(pixels, 1u, 1u), FilterRed));
      CHECK(FilterNear(At(pixels, 2u, 1u), FilterClear));
    }
    SECTION("the kernel is turned half way round")
    {
      // The kernel's top-left weight reads the pixel down and right of the target.
      node.Kernel = {1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
      graph.Nodes.push_back(node);
      const auto pixels = Run(graph);
      CHECK(FilterNear(At(pixels, 0u, 0u), FilterRed));
      CHECK(FilterNear(At(pixels, 1u, 1u), FilterClear));
    }
  }

  TEST_CASE_METHOD(FilterRenderer2DFixture, "FilterRenderer2D: displacement map", "[Render2D][Filter]")
  {
    FilterGraph2D graph;
    const auto texels = OnePixel(1u, 1u, FilterRed);
    graph.Sources.push_back(Source(Square, texels));

    SECTION("a neutral map leaves the input")
    {
      const List<FilterPixel> map(16u, {128, 128, 128, 255});
      graph.Sources.push_back(Source(Square, map));
      graph.Nodes.push_back({.Type = FilterNode2D::Kind::DisplacementMap,
                             .Inputs = {FilterInput2D::Source(0u), FilterInput2D::Source(1u)},
                             .Region = Square,
                             .Offset = {2.f, 2.f},
                             .XChannel = FilterChannel2D::Red,
                             .YChannel = FilterChannel2D::Green});
      CHECK(FilterNear(At(Run(graph), 1u, 1u), FilterRed));
    }
    SECTION("a full channel reads further along")
    {
      const List<FilterPixel> map(16u, {255, 128, 0, 255});
      graph.Sources.push_back(Source(Square, map));
      graph.Nodes.push_back({.Type = FilterNode2D::Kind::DisplacementMap,
                             .Inputs = {FilterInput2D::Source(0u), FilterInput2D::Source(1u)},
                             .Region = Square,
                             .Offset = {2.f, 2.f},
                             .XChannel = FilterChannel2D::Red,
                             .YChannel = FilterChannel2D::Green});
      const auto pixels = Run(graph);
      CHECK(FilterNear(At(pixels, 0u, 1u), FilterRed));
      CHECK(FilterNear(At(pixels, 1u, 1u), FilterClear));
    }
  }

  TEST_CASE_METHOD(FilterRenderer2DFixture, "FilterRenderer2D: lighting", "[Render2D][Filter]")
  {
    FilterGraph2D graph;
    const List<FilterPixel> texels(16u, FilterWhite);
    graph.Sources.push_back(Source(Square, texels));
    FilterNode2D node {
      .Type = FilterNode2D::Kind::DiffuseLighting, .Inputs = {FilterInput2D::Source(0u)}, .Region = Square};
    node.LightingColour = {1.f, 0.5f, 0.f};

    SECTION("a distant light overhead lights a flat surface fully")
    {
      graph.Nodes.push_back(node);
      const auto pixels = Run(graph);
      CHECK(FilterNear(At(pixels, 0u, 0u), {255, 128, 0, 255}));
      CHECK(FilterNear(At(pixels, 2u, 1u), {255, 128, 0, 255}));
    }
    SECTION("a light at the horizon lights it not at all")
    {
      node.Light.Vector = {1.f, 0.f, 0.f};
      graph.Nodes.push_back(node);
      CHECK(FilterNear(At(Run(graph), 1u, 1u), {0, 0, 0, 255}));
    }
    SECTION("specular alpha is the brightest channel")
    {
      node.Type = FilterNode2D::Kind::SpecularLighting;
      graph.Nodes.push_back(node);
      CHECK(FilterNear(At(Run(graph), 1u, 1u), {255, 128, 0, 255}));
    }
  }

  TEST_CASE_METHOD(FilterRenderer2DFixture, "FilterRenderer2D: turbulence is repeatable",
                   "[Render2D][Filter]")
  {
    FilterGraph2D graph;
    const PixelRect2D region {.X = 0, .Y = 0, .Width = 16u, .Height = 16u};
    graph.Nodes.push_back({.Type = FilterNode2D::Kind::Turbulence,
                           .Region = region,
                           .FractalNoise = true,
                           .BaseFrequency = {0.1f, 0.1f},
                           .Octaves = 2u,
                           .Seed = 3});
    const auto first = Run(graph);
    const auto second = Run(graph);
    CHECK(first == second);
    bool varies = false;
    for (const auto &pixel : first)
    {
      varies = varies || pixel != first[0];
      CHECK(pixel[0] <= pixel[3]);
    }
    CHECK(varies);

    graph.Nodes[0].Seed = 4;
    CHECK(Run(graph) != first);
  }

  TEST_CASE_METHOD(FilterRenderer2DFixture, "FilterRenderer2D: tile repeats its rect", "[Render2D][Filter]")
  {
    FilterGraph2D graph;
    const auto texels = OnePixel(1u, 1u, FilterRed);
    graph.Sources.push_back(Source(Square, texels));
    graph.Nodes.push_back(
      {.Type = FilterNode2D::Kind::Offset, .Inputs = {FilterInput2D::Source(0u)}, .Region = Square});
    graph.Nodes.push_back({.Type = FilterNode2D::Kind::Tile,
                           .Inputs = {FilterInput2D::Node(0u)},
                           .Region = {.X = 0, .Y = 0, .Width = 8u, .Height = 8u},
                           .TileRect = {.X = 1, .Y = 1, .Width = 2u, .Height = 2u}});
    const auto pixels = Run(graph);
    CHECK(FilterNear(pixels[1u * 8u + 1u], FilterRed));
    CHECK(FilterNear(pixels[3u * 8u + 3u], FilterRed));
    CHECK(FilterNear(pixels[7u * 8u + 5u], FilterRed));
    CHECK(FilterNear(pixels[2u * 8u + 2u], FilterClear));
  }
}

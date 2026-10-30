#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Gfx.Tests/Golden.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.UI/Compositing/GraphicsLayerTextureMapper.hpp"
#include "Krystal.UI/Compositing/TextureMapper.hpp"
#include "Krystal.UI/FontFaceCache.hpp"
#include <string>

namespace krys::ui::tests
{
  using namespace gfx::tests;

  namespace
  {
    /// @brief Paints each layer's contents by its name: a fill of its colour, and a rect inside it.
    class TestClient final : public boo::GraphicsLayerClient
    {
    public:
      struct Contents
      {
        boo::Color Fill;
        boo::FloatRect Inner;
        boo::Color InnerFill;
      };

      HashMap<utf8_string, Contents> ContentsByName;
      /// @brief Fills alternate bands of this height in two colours, for tall layers.
      float StripeHeight {0};
      uint32 PaintCount {0};

      void PaintContents(const boo::GraphicsLayer &layer, boo::GraphicsContext &context,
                         const boo::FloatRect &clip,
                         EnumSet<boo::GraphicsLayerPaintBehavior>) noexcept override
      {
        ++PaintCount;
        if (StripeHeight > 0)
        {
          auto first = static_cast<int32>(clip.Y() / StripeHeight);
          auto last = static_cast<int32>(clip.MaxY() / StripeHeight);
          for (int32 band = first; band <= last; ++band)
          {
            context.FillRect({0, static_cast<float>(band) * StripeHeight, layer.Size().Width(), StripeHeight},
                             band % 2 ? boo::Color {SRGBA<uint8> {239, 71, 111}}
                                      : boo::Color {SRGBA<uint8> {17, 138, 178}});
          }
          return;
        }

        auto it = ContentsByName.find(layer.Name());
        if (it == ContentsByName.end())
        {
          return;
        }
        context.FillRect({boo::FloatPoint(), layer.Size()}, it->second.Fill);
        if (!it->second.Inner.IsEmpty())
        {
          context.FillRect(it->second.Inner, it->second.InnerFill);
        }
      }
    };

    KRYS_NODISCARD boo::Color Rgb(uint32 rgb) noexcept
    {
      return SRGBA<uint8> {static_cast<uint8>(rgb >> 16), static_cast<uint8>(rgb >> 8),
                           static_cast<uint8>(rgb)};
    }

    /// @brief A device, renderers and compositor, compositing a layer tree into a target.
    class CompositingFixture
    {
    public:
      UniquePtr<gfx::Device> Device;
      UniquePtr<gfx::Renderer2D> Renderer;
      FontFaceCache Fonts;
      UniquePtr<TextureMapper> Compositor;
      GraphicsLayerFactoryTextureMapper Factory;
      TestClient Client;
      uint32 Width;
      uint32 Height;

      CompositingFixture(uint32 width, uint32 height)
          : Device(CreateTestDevice(gfx::DeviceDesc {}.TransientMemoryPerFrame)), Width(width), Height(height)
      {
        auto renderer = gfx::Renderer2D::Create(*Device);
        REQUIRE(renderer.has_value());
        Renderer = krys::move(*renderer);
        auto compositor = TextureMapper::Create(*Device, Fonts);
        REQUIRE(compositor.has_value());
        Compositor = krys::move(*compositor);
      }

      KRYS_NODISCARD Ref<GraphicsLayerTextureMapper> CreateLayer(utf8_stringview name,
                                                                 const boo::FloatPoint &position,
                                                                 const boo::FloatSize &size) noexcept
      {
        Ref<boo::GraphicsLayer> layer = boo::GraphicsLayer::Create(&Factory, Client);
        layer->SetName(utf8_string(name));
        layer->SetAnchorPoint({0.5f, 0.5f, 0});
        layer->SetPosition(position);
        layer->SetSize(size);
        return ShareRef(Downcast<GraphicsLayerTextureMapper>(*layer));
      }

      /// @brief Flushes the tree, paints what's dirty of its layers and composites it.
      KRYS_NODISCARD GoldenImage Composite(GraphicsLayerTextureMapper &root)
      {
        auto target =
          Device->CreateTexture({.Format = gfx::TextureFormat::RGBA8Unorm,
                                 .Width = Width,
                                 .Height = Height,
                                 .Usage = gfx::TextureUsage::RenderTarget | gfx::TextureUsage::Sampled,
                                 .DebugName = u8"Compositing"});
        REQUIRE(target.has_value());

        const boo::IntRect viewRect(0, 0, static_cast<int32>(Width), static_cast<int32>(Height));
        root.FlushCompositingState(boo::FloatRect(viewRect));

        Device->BeginFrame();
        auto &commands = Device->GetCommandBuffer();
        Renderer->Begin(commands, {.Texture = *target,
                                   .Width = Width,
                                   .Height = Height,
                                   .Format = gfx::TextureFormat::RGBA8Unorm,
                                   .ClearColour = Array<float, 4> {0.f, 0.f, 0.f, 0.f}});
        Compositor->BeginPainting(*Renderer, viewRect);
        root.UpdateBackingStoreIncludingSubLayers(*Compositor);
        root.Layer().Paint(*Compositor);
        Compositor->EndPainting();
        Renderer->End();
        Device->Submit(commands);
        Device->EndFrame();

        GoldenImage image {.Width = Width, .Height = Height, .Pixels = {}};
        image.Pixels.resize(static_cast<size_t>(Width) * Height);
        Device->ReadTexture(
          *target, gfx::TextureRegion::Whole2D(Width, Height),
          Span<byte>(reinterpret_cast<byte *>(image.Pixels.data()), image.Pixels.size() * 4u));
        Device->Destroy(*target);
        return image;
      }
    };

    KRYS_NODISCARD Array<uint8, 4> PixelAt(const GoldenImage &image, uint32 x, uint32 y) noexcept
    {
      return image.Pixels[static_cast<size_t>(y) * image.Width + x];
    }
  }

  TEST_CASE("Compositing: a layer tree matches the page it stands for", "[UI][GPU][Golden][Compositing]")
  {
    CompositingFixture fixture(360u, 240u);
    auto &client = fixture.Client;

    auto root = fixture.CreateLayer(u8"root", {}, {360, 240});
    root->SetBackgroundColor(Rgb(0xffffff));
    root->SetMasksToBounds(true);

    auto drawn = fixture.CreateLayer(u8"drawn", {16, 16}, {100, 80});
    drawn->SetDrawsContent(true);
    client.ContentsByName[u8"drawn"] = {Rgb(0x118ab2), {10, 10, 40, 30}, Rgb(0xffd166)};

    auto group = fixture.CreateLayer(u8"group", {140, 20}, {80, 80});
    group->SetOpacity(0.5f);
    auto under = fixture.CreateLayer(u8"under", {0, 0}, {50, 50});
    under->SetBackgroundColor(Rgb(0x073b4c));
    auto over = fixture.CreateLayer(u8"over", {20, 20}, {60, 60});
    over->SetBackgroundColor(Rgb(0xef476f));
    group->AddChild(Ref<boo::GraphicsLayer> {under});
    group->AddChild(Ref<boo::GraphicsLayer> {over});

    auto turned = fixture.CreateLayer(u8"turned", {250, 30}, {80, 60});
    turned->SetDrawsContent(true);
    boo::TransformationMatrix rotation;
    rotation.Rotate(15);
    turned->SetTransform(rotation);
    client.ContentsByName[u8"turned"] = {Rgb(0x06d6a0), {10, 10, 20, 20}, Rgb(0x073b4c)};

    auto blurred = fixture.CreateLayer(u8"blurred", {30, 140}, {100, 70});
    blurred->SetDrawsContent(true);
    CHECK(blurred->SetFilters(boo::FilterOperations {{boo::BlurFilterOperation::Create(3)}}));
    client.ContentsByName[u8"blurred"] = {Rgb(0xef476f), {30, 20, 40, 30}, Rgb(0xffd166)};

    auto clipping = fixture.CreateLayer(u8"clipping", {170, 140}, {80, 70});
    clipping->SetMasksToBounds(true);
    clipping->SetBackgroundColor(Rgb(0xe9ecf2));
    auto clipped = fixture.CreateLayer(u8"clipped", {40, 30}, {80, 60});
    clipped->SetDrawsContent(true);
    client.ContentsByName[u8"clipped"] = {Rgb(0x118ab2), {}, {}};
    clipping->AddChild(Ref<boo::GraphicsLayer> {clipped});

    auto solid = fixture.CreateLayer(u8"solid", {280, 150}, {60, 50});
    solid->SetContentsToSolidColor(Rgb(0x8338ec));
    solid->SetContentsRect({0, 0, 60, 50});
    solid->SetContentsClippingRect(boo::FloatRoundedRect({0, 0, 60, 50}));

    for (auto *layer : {drawn.get(), group.get(), turned.get(), blurred.get(), clipping.get(), solid.get()})
    {
      root->AddChild(ShareRef(*layer));
    }

    const auto actual = fixture.Composite(*root);
    const auto output = GoldenOutputDirectory();
    CHECK(SavePng(output / "compositing.actual.png", actual));

    const auto ownPath = GoldenDirectory() / "compositing.png";
    const auto own = LoadPng(ownPath);
    if (UpdatingGoldens() || !own)
    {
      WARN("Writing the compositing golden; review it before committing.");
      CHECK(SavePng(ownPath, actual));
    }
    else
    {
      const auto comparison = CompareImages(actual, *own, 2u);
      INFO("against its golden: largest difference " << comparison.LargestDifference << ", "
                                                     << comparison.PixelsOver << " pixels over 2");
      CHECK(comparison.PixelsOver == 0u);
    }

    const auto chrome = LoadPng(GoldenDirectory() / "compositing.chrome.png");
    if (!chrome)
    {
      WARN(
        "No Chrome golden; run tools/golden/capture_chrome.py compositing --golden Krystal.UI.Tests/golden.");
      return;
    }
    const auto comparison = CompareImages(actual, *chrome, 24u);
    SavePng(output / "compositing.chrome-diff.png", DifferenceImage(actual, *chrome));
    WARN("against Chrome: largest difference " << comparison.LargestDifference << ", mean "
                                               << comparison.MeanDifference << ", " << comparison.PixelsOver
                                               << " of " << comparison.PixelCount << " pixels over 24");
    CHECK(comparison.PixelsOver <= comparison.PixelCount / 200u);
    CHECK(comparison.MeanDifference < 1.5);
  }

  TEST_CASE("Compositing: tiles cover what's visible of a layer", "[UI][GPU][Compositing]")
  {
    CompositingFixture fixture(360u, 240u);
    fixture.Client.StripeHeight = 100;

    auto root = fixture.CreateLayer(u8"root", {}, {360, 240});
    root->SetMasksToBounds(true);
    auto tall = fixture.CreateLayer(u8"tall", {0, -1'500}, {360, 6'000});
    tall->SetDrawsContent(true);
    root->AddChild(Ref<boo::GraphicsLayer> {tall});

    auto checkStripes = [&](const GoldenImage &image, float offset)
    {
      for (uint32 y = 5; y < 240; y += 50)
      {
        auto band = static_cast<int32>((static_cast<float>(y) + offset) / 100);
        auto expected = band % 2 ? Array<uint8, 4> {239, 71, 111, 255} : Array<uint8, 4> {17, 138, 178, 255};
        INFO("y " << y);
        CHECK(PixelAt(image, 180, y) == expected);
      }
    };

    auto image = fixture.Composite(*root);
    checkStripes(image, 1'500);

    // What's visible of the layer, in its coordinates, and the tiles near it: none far from it.
    CHECK(tall->VisibleRect() == boo::IntRect(0, 1'500, 360, 240));
    auto *backingStore = tall->BackingStore();
    REQUIRE(backingStore);
    REQUIRE_FALSE(backingStore->Tiles().empty());
    float covered = 0;
    for (const auto &tile : backingStore->Tiles())
    {
      CHECK(tile.Rect().MaxY() > 1'500 - 240 - 240);
      CHECK(tile.Rect().Y() < 1'740 + 240 + 240);
      covered += tile.Rect().Height() * tile.Rect().Width();
    }
    CHECK(covered < 360.f * 6000.f / 4);

    // Moving the layer moves what's visible: tiles follow it.
    tall->SetPosition({0, -4'000});
    image = fixture.Composite(*root);
    checkStripes(image, 4'000);
    CHECK(tall->VisibleRect() == boo::IntRect(0, 4'000, 360, 240));
    for (const auto &tile : backingStore->Tiles())
    {
      CHECK(tile.Rect().MaxY() > 4'000 - 240 - 240);
    }
  }

  TEST_CASE("Compositing: only what's dirty is painted again", "[UI][GPU][Compositing]")
  {
    CompositingFixture fixture(200u, 100u);
    auto &client = fixture.Client;

    auto root = fixture.CreateLayer(u8"root", {}, {200, 100});
    auto drawn = fixture.CreateLayer(u8"drawn", {0, 0}, {200, 100});
    drawn->SetDrawsContent(true);
    client.ContentsByName[u8"drawn"] = {Rgb(0x118ab2), {}, {}};
    root->AddChild(Ref<boo::GraphicsLayer> {drawn});

    auto image = fixture.Composite(*root);
    CHECK(PixelAt(image, 50, 50) == Array<uint8, 4> {17, 138, 178, 255});
    CHECK(PixelAt(image, 150, 50) == Array<uint8, 4> {17, 138, 178, 255});

    // Nothing changed: nothing is painted.
    auto paints = client.PaintCount;
    image = fixture.Composite(*root);
    CHECK(client.PaintCount == paints);

    // The contents change everywhere, but only the right half is dirty: the left keeps what was painted.
    client.ContentsByName[u8"drawn"] = {Rgb(0xef476f), {}, {}};
    drawn->SetNeedsDisplayInRect({100, 0, 100, 100});
    image = fixture.Composite(*root);
    CHECK(PixelAt(image, 50, 50) == Array<uint8, 4> {17, 138, 178, 255});
    CHECK(PixelAt(image, 150, 50) == Array<uint8, 4> {239, 71, 111, 255});
  }

  TEST_CASE("Compositing: a layer tree as text", "[UI][Compositing]")
  {
    TestClient client;
    GraphicsLayerFactoryTextureMapper factory;
    auto root = boo::GraphicsLayer::Create(&factory, client);
    root->SetSize({100, 100});
    auto child = boo::GraphicsLayer::Create(&factory, client);
    child->SetPosition({10, 20});
    child->SetSize({30, 40});
    child->SetDrawsContent(true);
    root->AddChild(Ref<boo::GraphicsLayer> {child});
    CHECK(root->LayerTreeAsText() == u8R"((GraphicsLayer
  (anchor 0.50 0.50)
  (bounds 100.00 100.00)
  (children 1
    (GraphicsLayer
      (position 10.00 20.00)
      (anchor 0.50 0.50)
      (bounds 30.00 40.00)
      (drawsContent 1)
    )
  )
)
)");
  }
}

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Animation/DocumentTimeline.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayer.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerBacking.hpp"
#include "Krystal.Booey/Rendering/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Gfx.Tests/Golden.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.UI/View.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace krys::ui::tests
{
  using namespace gfx::tests;
  using namespace std::chrono_literals;

  namespace
  {
    /// @brief Paints a view into a target of its size the way a host does each frame (`View::Paint`, which
    /// composites the page's layers, repainting only what changed in them), and as a full repaint of the page
    /// (`Record` and `Replay`), to compare.
    class RepaintFixture
    {
      KRYS_NON_COPY_MOVABLE(RepaintFixture);

    private:
      UniquePtr<gfx::Device> _device;
      UniquePtr<gfx::Renderer2D> _renderer;
      gfx::TextureHandle _target;
      uint32 _width;
      uint32 _height;

    public:
      RepaintFixture(uint32 width, uint32 height)
          : _device(CreateTestDevice(gfx::DeviceDesc {}.TransientMemoryPerFrame)), _width(width),
            _height(height)
      {
        auto renderer = gfx::Renderer2D::Create(*_device);
        REQUIRE(renderer.has_value());
        _renderer = krys::move(*renderer);
        auto target =
          _device->CreateTexture({.Format = gfx::TextureFormat::RGBA8Unorm,
                                  .Width = width,
                                  .Height = height,
                                  .Usage = gfx::TextureUsage::RenderTarget | gfx::TextureUsage::Sampled,
                                  .DebugName = u8"Repaint"});
        REQUIRE(target.has_value());
        _target = *target;
      }

      ~RepaintFixture() noexcept
      {
        _device->Destroy(_target);
      }

      /// @brief The page as the host shows it: its layers composited.
      KRYS_NODISCARD GoldenImage Painted(View &view)
      {
        return Draw([&] { view.Paint(*_renderer); });
      }

      /// @brief The page composited where the host draws it: `offset` into the target.
      KRYS_NODISCARD GoldenImage PaintedAt(View &view, Vec2f offset)
      {
        return Draw(
          [&]
          {
            _renderer->Translate(offset.x, offset.y);
            view.Paint(*_renderer);
          });
      }

      /// @brief The whole page painted again.
      KRYS_NODISCARD GoldenImage Repainted(View &view)
      {
        // Animations that run on layers don't update the page's style each frame: resolve it at the current
        // time, so the whole page paints what the layers show.
        if (auto *timeline = view.Document().ExistingTimeline())
        {
          for (const auto &animation : timeline->RelevantAnimations())
          {
            if (auto *effect = animation->GetKeyframeEffect())
            {
              effect->Invalidate();
            }
          }
        }
        (void)view.Document().UpdateLayout();
        auto displayList = view.Record();
        return Draw([&] { view.Replay(*displayList, *_renderer); });
      }

    private:
      template <typename TDraw>
      KRYS_NODISCARD GoldenImage Draw(TDraw &&draw)
      {
        _device->BeginFrame();
        auto &commands = _device->GetCommandBuffer();
        _renderer->Begin(commands, {.Texture = _target,
                                    .Width = _width,
                                    .Height = _height,
                                    .Format = gfx::TextureFormat::RGBA8Unorm,
                                    .ClearColour = Array<float, 4> {0.f, 0.f, 0.f, 0.f}});
        draw();
        _renderer->End();
        _device->Submit(commands);
        _device->EndFrame();

        GoldenImage image {.Width = _width, .Height = _height, .Pixels = {}};
        image.Pixels.resize(static_cast<size_t>(_width) * _height);
        _device->ReadTexture(
          _target, gfx::TextureRegion::Whole2D(_width, _height),
          Span<byte>(reinterpret_cast<byte *>(image.Pixels.data()), image.Pixels.size() * 4u));
        return image;
      }
    };

    /// @brief Checks the page the host shows after a change against the whole page painted again: no stale
    /// pixels.
    void CheckNoStalePixels(RepaintFixture &fixture, View &view, const std::string &step)
    {
      auto painted = fixture.Painted(view);
      auto repainted = fixture.Repainted(view);
      auto comparison = CompareImages(painted, repainted, 2u);
      INFO(step << ": largest difference " << comparison.LargestDifference << ", " << comparison.PixelsOver
                << " pixels over 2");
      if (comparison.PixelsOver > 0u)
      {
        (void)SavePng(GoldenOutputDirectory() / ("repaint." + step + ".painted.png"), painted);
        (void)SavePng(GoldenOutputDirectory() / ("repaint." + step + ".repainted.png"), repainted);
      }
      CHECK(comparison.PixelsOver == 0u);
    }

    /// @brief Writes a 1x1 GIF of two frames, red then blue, 100ms each, repeating forever, to @p path.
    void WriteAnimatedGif(const std::filesystem::path &path)
    {
      const unsigned char gif[] = {
        71, 73,  70, 56, 57,  97, 1,  0,  1,  0,  129, 0,  0,  255, 0,  0,  0,  0, 0, 0, 0,   0,
        0,  0,   0,  33, 255, 11, 78, 69, 84, 83, 67,  65, 80, 69,  50, 46, 48, 3, 1, 0, 0,   0,
        33, 249, 4,  0,  10,  0,  0,  0,  44, 0,  0,   0,  0,  1,   0,  1,  0,  0, 8, 4, 0,   1,
        4,  4,   0,  33, 249, 4,  1,  10, 0,  1,  0,   44, 0,  0,   0,  0,  1,  0, 1, 0, 129, 0,
        0,  255, 0,  0,  0,   0,  0,  0,  0,  0,  0,   8,  4,  0,   1,  4,  4,  0, 59};
      std::ofstream image(path, std::ios::binary);
      image.write(reinterpret_cast<const char *>(gif), sizeof(gif));
    }
  }

  TEST_CASE("View: repainting only the damage leaves no stale pixels", "[UI][GPU]")
  {
    static const auto fonts = View::AddFontDirectory(std::filesystem::path {KRYS_UI_TESTS_FONTS_DIR});
    (void)fonts;

    RepaintFixture fixture(320u, 240u);
    View view({320, 240});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>"
      u8"body { margin: 8px; font: 14px 'Liberation Sans'; background: #f4f4f4 }"
      u8"#box { width: 60px; height: 30px; background: red; border-radius: 6px; box-shadow: 2px 2px 6px "
      u8"#0008 }"
      u8"#box:hover { background: blue }"
      u8"#spin { position: absolute; left: 200px; top: 20px; width: 30px; height: 30px; background: teal }"
      u8"#scroller { width: 120px; height: 60px; overflow: auto; border: 1px solid black }"
      u8"#scroller div { height: 30px; background: linear-gradient(orange, purple) }"
      u8"#moved { position: absolute; left: 10px; top: 200px; width: 20px; height: 20px; background: green;"
      u8" outline: 2px solid black }"
      u8"</style>"
      u8"<div id=box></div><p id=text>Some text that changes.</p>"
      u8"<div id=spin></div><div id=scroller><div></div><div></div><div></div><div></div></div>"
      u8"<div id=moved></div><div id=list></div>");

    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&]
    {
      time += 16ms;
      (void)view.Update(time);
    };
    frame();
    CheckNoStalePixels(fixture, view, "first");

    auto &document = view.Document();
    auto set = [&](utf8_stringview id, utf8_stringview style)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      REQUIRE_FALSE(element->SetAttribute(u8"style", utf8_string(style)).HasException());
    };

    SECTION("changes one after another")
    {
      set(u8"box", u8"background: green");
      frame();
      CheckNoStalePixels(fixture, view, "colour");

      view.MouseMoved({20, 20});
      frame();
      CheckNoStalePixels(fixture, view, "hover");

      set(u8"box", u8"width: 120px");
      frame();
      CheckNoStalePixels(fixture, view, "wider");

      set(u8"box", u8"width: 40px; height: 50px");
      frame();
      CheckNoStalePixels(fixture, view, "narrower");

      auto text = document.GetElementById(u8"text");
      REQUIRE(text);
      REQUIRE_FALSE(
        text->TextContent(u8"Other, longer text that wraps onto a second line here.").HasException());
      frame();
      CheckNoStalePixels(fixture, view, "text");

      set(u8"spin", u8"transform: rotate(30deg)");
      frame();
      CheckNoStalePixels(fixture, view, "rotate");

      set(u8"spin", u8"transform: rotate(60deg) scale(1.5); opacity: 0.5");
      frame();
      CheckNoStalePixels(fixture, view, "rotate-again");

      set(u8"moved", u8"left: 150px; top: 180px");
      frame();
      CheckNoStalePixels(fixture, view, "moved");

      // Changes far apart in one frame are painted as separate rects.
      set(u8"moved", u8"left: 280px; top: 200px; background: purple");
      set(u8"box", u8"width: 40px; height: 50px; background: orange");
      frame();
      CheckNoStalePixels(fixture, view, "far-apart");

      set(u8"box", u8"display: none");
      frame();
      CheckNoStalePixels(fixture, view, "hidden");

      set(u8"box", u8"visibility: hidden");
      frame();
      CheckNoStalePixels(fixture, view, "shown-invisible");

      auto list = document.GetElementById(u8"list");
      REQUIRE(list);
      auto created = document.CreateElement(u8"span");
      REQUIRE_FALSE(created.HasException());
      auto inserted = created.ReleaseValue();
      REQUIRE_FALSE(inserted->TextContent(u8"inserted text").HasException());
      REQUIRE_FALSE(list->AppendChild(*inserted).HasException());
      frame();
      CheckNoStalePixels(fixture, view, "inserted");

      REQUIRE_FALSE(inserted->Remove().HasException());
      frame();
      CheckNoStalePixels(fixture, view, "removed");

      auto scroller = document.GetElementById(u8"scroller");
      REQUIRE(scroller);
      scroller->SetScrollTop(25);
      frame();
      CheckNoStalePixels(fixture, view, "scrolled");

      view.ScrollTo({0, 20});
      frame();
      CheckNoStalePixels(fixture, view, "page-scrolled");
    }

    SECTION("an animation")
    {
      set(u8"spin", u8"transition: transform 100ms linear; transform: rotate(90deg)");
      for (int32 i = 0; i < 8; ++i)
      {
        frame();
        CheckNoStalePixels(fixture, view, "animation-" + std::to_string(i));
      }
    }
  }

  TEST_CASE("View: changes inside filters leave no stale pixels", "[UI][GPU]")
  {
    RepaintFixture fixture(320u, 240u);
    View view({320, 240});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>"
      u8"body { margin: 0; background: #f4f4f4 }"
      u8"div { position: absolute }"
      u8"#blurred { left: 20px; top: 20px; width: 80px; height: 80px; background: #073b4c; filter: blur(4px) "
      u8"}"
      u8"#inner { left: 20px; top: 20px; width: 20px; height: 20px; background: #ffd166 }"
      u8"#shadowed { left: 160px; top: 20px; width: 60px; height: 60px; background: #118ab2;"
      u8" filter: drop-shadow(6px 6px 4px #000a) }"
      u8"#tinted { left: 20px; top: 140px; width: 60px; height: 60px; background: #ef476f; filter: sepia(1) }"
      u8"</style>"
      u8"<div id=blurred><div id=inner></div></div><div id=shadowed></div><div id=tinted></div>");

    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&]
    {
      time += 16ms;
      (void)view.Update(time);
    };
    frame();
    CheckNoStalePixels(fixture, view, "filters-first");

    auto &document = view.Document();
    auto set = [&](utf8_stringview id, utf8_stringview style)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      REQUIRE_FALSE(element->SetAttribute(u8"style", utf8_string(style)).HasException());
    };

    // A change inside a blur repaints as far as the blur reaches.
    set(u8"inner", u8"background: #ef476f");
    frame();
    CheckNoStalePixels(fixture, view, "filters-inside-blur");

    set(u8"inner", u8"left: 50px; top: 50px");
    frame();
    CheckNoStalePixels(fixture, view, "filters-moved-inside-blur");

    // Changing the filter itself, and the box it filters.
    set(u8"blurred", u8"filter: blur(8px)");
    frame();
    CheckNoStalePixels(fixture, view, "filters-more-blur");

    set(u8"shadowed", u8"left: 200px; top: 60px");
    frame();
    CheckNoStalePixels(fixture, view, "filters-shadow-moved");

    set(u8"shadowed", u8"filter: drop-shadow(-10px 12px 2px #ef476f)");
    frame();
    CheckNoStalePixels(fixture, view, "filters-shadow-changed");

    set(u8"tinted", u8"filter: none");
    frame();
    CheckNoStalePixels(fixture, view, "filters-removed");

    set(u8"tinted", u8"filter: blur(3px) hue-rotate(90deg)");
    frame();
    CheckNoStalePixels(fixture, view, "filters-added");

    set(u8"blurred", u8"filter: none");
    frame();
    CheckNoStalePixels(fixture, view, "filters-blur-removed");

    // A filter animating.
    set(u8"shadowed", u8"transition: filter 100ms linear; filter: drop-shadow(0 0 12px #073b4c) blur(2px)");
    for (int32 i = 0; i < 8; ++i)
    {
      frame();
      CheckNoStalePixels(fixture, view, "filters-animation-" + std::to_string(i));
    }
  }

  TEST_CASE("View: the page is composited where the host draws it", "[UI][GPU]")
  {
    RepaintFixture fixture(100u, 80u);
    View view({100, 50});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; background: #e9ecf2 }"
                  u8"div { position: absolute; left: 20px; top: 10px; width: 30px; height: 30px }</style>"
                  u8"<div style='background: #073b4c; filter: blur(2px)'></div>");
    (void)view.Update(boo::html::EventLoop::Current().Now());

    // Below a 30 pixel header: the page moves down with the host's transform, and nothing is drawn above it.
    auto atTop = fixture.Painted(view);
    auto belowHeader = fixture.PaintedAt(view, {0.f, 30.f});
    uint32 moved = 0u;
    uint32 aboveHeader = 0u;
    for (uint32 y = 0u; y < 80u; ++y)
    {
      for (uint32 x = 0u; x < 100u; ++x)
      {
        const auto &pixel = belowHeader.Pixels[y * 100u + x];
        if (y < 30u)
        {
          aboveHeader += pixel[3] != 0u ? 1u : 0u;
        }
        else if (pixel != atTop.Pixels[(y - 30u) * 100u + x])
        {
          ++moved;
        }
      }
    }
    CHECK(aboveHeader == 0u);
    CHECK(moved == 0u);
  }

  TEST_CASE("View: composited images at fractional positions paint as the whole page does", "[UI][GPU]")
  {
    const auto directory = std::filesystem::temp_directory_path() / "krystal-ui-composited-images-test";
    std::filesystem::create_directories(directory);
    std::filesystem::copy_file(GoldenDirectory() / "scenes" / "images" / "photo.jpg", directory / "photo.jpg",
                               std::filesystem::copy_options::overwrite_existing);
    {
      // Images with filters composite with the image as their layer's contents; the layers sit at whole
      // pixels and the images 0.4 pixels into them.
      std::ofstream page(directory / "page.html", std::ios::binary);
      page << "<!DOCTYPE html><style>body { margin: 0; background: white }"
              "img { position: absolute; top: 10px; width: 20px; height: 20px; filter: saturate(2) }</style>"
              "<img src=photo.jpg style='left: 10.4px'><img src=photo.jpg style='left: 40.4px; "
              "border-radius: 6px'>";
    }

    RepaintFixture fixture(80u, 40u);
    View view({80, 40});
    REQUIRE(view.LoadFile(directory / "page.html").has_value());
    (void)view.Update(boo::html::EventLoop::Current().Now());
    CheckNoStalePixels(fixture, view, "fractional-images");

    std::filesystem::remove_all(directory);
  }

  TEST_CASE("View: changes beside filters leave no stale pixels", "[UI][GPU]")
  {
    const auto directory = std::filesystem::temp_directory_path() / "krystal-ui-beside-filters-test";
    std::filesystem::create_directories(directory);
    WriteAnimatedGif(directory / "spin.gif");
    {
      // An animated image between a blurred box and a drop-shadowed one, and a box whose shadow, on hover,
      // reaches over the blur.
      std::ofstream page(directory / "page.html", std::ios::binary);
      page << "<!DOCTYPE html><style>body { margin: 0; background: #f4f4f4 } div, img { position: absolute }"
              "#blurred { left: 20px; top: 20px; width: 60px; height: 40px; background: #073b4c; filter: "
              "blur(4px) }"
              "#shadowed { left: 120px; top: 20px; width: 60px; height: 40px; background: #118ab2;"
              " filter: drop-shadow(6px 6px 4px #000a) }"
              "#gif { left: 90px; top: 30px; width: 20px; height: 20px }"
              "#hover { left: 40px; top: 70px; width: 40px; height: 20px; background: #ffd166 }"
              "#hover:hover { box-shadow: 0 -20px 12px #ef476f }</style>"
              "<div id=blurred></div><div id=shadowed></div><img id=gif src=spin.gif><div id=hover></div>";
    }

    RepaintFixture fixture(200u, 100u);
    View view({200, 100});
    REQUIRE(view.LoadFile(directory / "page.html").has_value());
    auto time = boo::html::EventLoop::Current().Now();
    auto frame = [&](MonotonicTime::duration delta)
    {
      time += delta;
      (void)view.Update(time);
      (void)fixture.Painted(view);
    };
    frame(16ms);
    frame(16ms);
    CheckNoStalePixels(fixture, view, "beside-filters-first");

    for (int32 i = 0; i < 4; ++i)
    {
      frame(100ms);
      CheckNoStalePixels(fixture, view, "beside-filters-image-" + std::to_string(i));
    }

    view.MouseMoved({60, 80});
    frame(16ms);
    CheckNoStalePixels(fixture, view, "beside-filters-hover");
    view.MouseMoved({190, 90});
    frame(16ms);
    CheckNoStalePixels(fixture, view, "beside-filters-unhover");

    std::filesystem::remove_all(directory);
  }

  namespace
  {
    /// @brief The page as composited: tracks the repaints of the view's own layer (the render view's), which
    /// holds what isn't composited by itself.
    class TrackedRepaints
    {
    private:
      View &_view;

    public:
      explicit TrackedRepaints(View &view) noexcept : _view(view)
      {
        Compositor().SetTracksRepaints(true);
      }

      ~TrackedRepaints() noexcept
      {
        Compositor().SetTracksRepaints(false);
      }

      KRYS_NODISCARD boo::RenderLayerCompositor &Compositor() const noexcept
      {
        return _view.Frame().View()->GetRenderViewForUpdate()->Compositor();
      }

      KRYS_NODISCARD boo::GraphicsLayer &ViewLayer() const noexcept
      {
        return *_view.Frame().View()->GetRenderViewForUpdate()->Layer()->Backing()->GetGraphicsLayer();
      }

      void Reset() noexcept
      {
        Compositor().ResetTrackedRepaintRects();
      }

      /// @brief The graphics layer of the composited element @p id.
      KRYS_NODISCARD boo::GraphicsLayer &LayerOf(utf8_stringview id) const noexcept
      {
        auto element = _view.Document().GetElementById(id);
        auto *layer = krys::Downcast<boo::RenderLayerModelObject>(*element->Renderer()).Layer();
        krys_assert(layer && layer->Backing());
        return *layer->Backing()->GetGraphicsLayer();
      }

      /// @brief The layer holding what the composited element @p id scrolls.
      KRYS_NODISCARD boo::GraphicsLayer &ScrolledContentsOf(utf8_stringview id) const noexcept
      {
        auto element = _view.Document().GetElementById(id);
        auto *layer = krys::Downcast<boo::RenderLayerModelObject>(*element->Renderer()).Layer();
        krys_assert(layer && layer->Backing() && layer->Backing()->ScrolledContentsLayer());
        return *layer->Backing()->ScrolledContentsLayer();
      }

      /// @brief The bounds of the view layer's repaints since the last reset, in the layer's coordinates.
      KRYS_NODISCARD boo::FloatRect Bounds() const noexcept
      {
        return Bounds(ViewLayer());
      }

      /// @brief The bounds of @p layer's repaints since the last reset, in its coordinates.
      KRYS_NODISCARD boo::FloatRect Bounds(boo::GraphicsLayer &layer) const noexcept
      {
        auto text = layer.LayerTreeAsText({boo::LayerTreeAsTextOptions::IncludeRepaintRects,
                                           boo::LayerTreeAsTextOptions::IncludeRootLayerProperties});
        // Only the layer's own rects: its children's come after "(children".
        text = text.substr(0, text.find(u8"(children"));
        boo::FloatRect bounds;
        for (size_t at = text.find(u8"(rect "); at != utf8_string::npos; at = text.find(u8"(rect ", at + 1))
        {
          float x = 0;
          float y = 0;
          float width = 0;
          float height = 0;
          std::istringstream rect(std::string(reinterpret_cast<const char *>(text.c_str()) + at + 6));
          rect >> x >> y >> width >> height;
          REQUIRE(rect);
          bounds.Unite(boo::FloatRect(x, y, width, height));
        }
        return bounds;
      }
    };
  }

  TEST_CASE("View: scrolling moves the painted page and leaves no stale pixels", "[UI][GPU]")
  {
    static const auto fonts = View::AddFontDirectory(std::filesystem::path {KRYS_UI_TESTS_FONTS_DIR});
    (void)fonts;

    RepaintFixture fixture(240u, 160u);
    View view({240, 160});
    bool fixedBackground = false;

    SECTION("a fixed header and an animation")
    {
      view.LoadHTML(
        u8"<!DOCTYPE html><style>"
        u8"body { margin: 0; font: 14px 'Liberation Sans' }"
        u8"#header { position: fixed; top: 0; left: 0; right: 0; height: 24px; background: #222c; color: "
        u8"white }"
        u8".row { height: 30px; border-bottom: 1px solid #888; background: linear-gradient(90deg, #fde, "
        u8"#def) }"
        u8"#spin { position: absolute; left: 150px; top: 100px; width: 30px; height: 30px; background: teal;"
        u8" animation: turn 1s linear infinite }"
        u8"@keyframes turn { to { transform: rotate(360deg) } }"
        u8"</style><div id=header>Header</div>"
        u8"<div class=row>1</div><div class=row>2</div><div class=row>3</div><div class=row>4</div>"
        u8"<div class=row>5</div><div class=row>6</div><div class=row>7</div><div class=row>8</div>"
        u8"<div class=row>9</div><div class=row>10</div><div class=row>11</div><div class=row>12</div>"
        u8"<div id=spin></div>");
    }

    SECTION("a fixed background")
    {
      fixedBackground = true;
      view.LoadHTML(u8"<!DOCTYPE html><style>"
                    u8"body { margin: 0; height: 600px;"
                    u8" background: linear-gradient(#f88, #88f) fixed }"
                    u8"p { margin: 40px 10px; font: 14px 'Liberation Sans' }"
                    u8"</style><p>One</p><p>Two</p><p>Three</p><p>Four</p><p>Five</p><p>Six</p>");
    }

    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&]
    {
      time += 16ms;
      (void)view.Update(time);
    };
    frame();
    CheckNoStalePixels(fixture, view, "scroll-first");
    // A fixed background moves against the contents, so scrolling repaints all of them.
    CHECK(view.Frame().View()->UseSlowRepaints() == fixedBackground);
    // A fixed header has a layer of its own, which stays put as the page's layer moves.
    if (!fixedBackground)
    {
      CHECK(view.Document().GetElementById(u8"header")->Renderer()->IsComposited());
    }

    TrackedRepaints repaints(view);
    for (int32 step = 1; step <= 6; ++step)
    {
      // Down in steps of different sizes, then back up; twice in one frame on the third step.
      auto y = step <= 4 ? step * 23 : (8 - step) * 31;
      repaints.Reset();
      view.ScrollTo({0, y});
      if (step == 3)
      {
        view.ScrollTo({0, y + 7});
      }
      frame();
      CHECK(repaints.Bounds().IsEmpty() != fixedBackground);
      CheckNoStalePixels(fixture, view, "scroll-" + std::to_string(step));
    }
  }

  TEST_CASE("View: repaints the rect that changed", "[UI][GPU]")
  {
    RepaintFixture fixture(200u, 100u);
    View view({200, 100});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0 } #box { width: 50px; height: 50px; background: red }"
      u8"#box:hover { background: blue }"
      u8"#spin { position: absolute; left: 100px; top: 20px; width: 20px; height: 20px; background: teal }"
      u8"#tall { height: 500px }</style><div id=box></div><div id=spin></div><div id=tall></div>");
    auto time = boo::html::EventLoop::Current().Now() + 1s;
    // A frame as the host runs it: an update, then a paint if the page asks for one.
    auto frame = [&]
    {
      time += 16ms;
      bool updated = view.Update(time);
      if (view.NeedsPaint())
      {
        (void)fixture.Painted(view);
      }
      return updated;
    };
    (void)frame();
    (void)frame();
    TrackedRepaints repaints(view);
    repaints.Reset();

    // A still page needs no update or paint, and repaints nothing.
    CHECK_FALSE(frame());
    CHECK_FALSE(view.NeedsPaint());
    CHECK(repaints.Bounds().IsEmpty());

    SECTION("a colour")
    {
      auto box = view.Document().GetElementById(u8"box");
      REQUIRE_FALSE(box->SetAttribute(u8"style", u8"background: green").HasException());
      CHECK(frame());
      CHECK(repaints.Bounds() == boo::FloatRect(0, 0, 50, 50));
    }

    SECTION("hovering")
    {
      view.MouseMoved({10, 10});
      CHECK(frame());
      CHECK(repaints.Bounds() == boo::FloatRect(0, 0, 50, 50));
    }

    SECTION("scrolling moves the layers and repaints nothing")
    {
      view.ScrollTo({0, 30});
      (void)frame();
      CHECK(repaints.Bounds().IsEmpty());
    }

    SECTION("a transform")
    {
      auto spin = view.Document().GetElementById(u8"spin");
      REQUIRE_FALSE(spin->SetAttribute(u8"style", u8"transform: rotate(45deg)").HasException());
      CHECK(frame());
      auto bounds = repaints.Bounds();
      // The box before and after turning: about 28 pixels across, around its centre (110, 30).
      CHECK(boo::FloatRect(94, 14, 32, 32).Contains(bounds));
      CHECK(bounds.Contains(boo::FloatRect(100, 20, 20, 20)));
    }
  }

  TEST_CASE("View: scrolling a box moves its layers and repaints nothing", "[UI][GPU]")
  {
    RepaintFixture fixture(200u, 160u);
    View view({200, 160});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0; background: #f4f4f4 }"
      u8"#scroller { margin: 10px; width: 140px; height: 100px; overflow: auto; background: white }"
      u8".row { height: 30px; border-bottom: 1px solid #888; background: linear-gradient(90deg, #fde, #def) }"
      u8"#blurred { margin: 10px; width: 40px; height: 20px; background: #073b4c; filter: blur(2px) }"
      u8"</style><div id=scroller><div class=row></div><div class=row></div><div id=blurred></div>"
      u8"<div class=row></div><div class=row></div><div class=row id=changed></div><div class=row></div>"
      u8"<div class=row></div><div class=row></div><div class=row></div><div class=row></div>"
      u8"<div class=row></div><div class=row></div><div class=row></div><div class=row></div></div>");
    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&]
    {
      time += 16ms;
      (void)view.Update(time);
      (void)fixture.Painted(view);
    };
    frame();
    frame();
    TrackedRepaints repaints(view);
    auto scroller = view.Document().GetElementById(u8"scroller");
    REQUIRE(scroller->Renderer()->IsComposited());
    CheckNoStalePixels(fixture, view, "box-scroll-first");

    // Down in steps of different sizes, past the tiles painted at first, then back up; the blurred box,
    // composited inside the scrolled contents, moves with them.
    for (int32 y : {25, 60, 140, 300, 90, 0})
    {
      repaints.Reset();
      scroller->SetScrollTop(y);
      frame();
      CHECK(repaints.Bounds().IsEmpty());
      CHECK(repaints.Bounds(repaints.LayerOf(u8"scroller")).IsEmpty());
      CHECK(repaints.Bounds(repaints.ScrolledContentsOf(u8"scroller")).IsEmpty());
      CheckNoStalePixels(fixture, view, "box-scroll-" + std::to_string(y));
    }

    // A change inside the scrolled box repaints it where it is in the scrolled contents.
    scroller->SetScrollTop(70);
    frame();
    auto changed = view.Document().GetElementById(u8"changed");
    REQUIRE_FALSE(changed->SetAttribute(u8"style", u8"background: #06d6a0").HasException());
    frame();
    CheckNoStalePixels(fixture, view, "box-scroll-changed");
  }

  TEST_CASE("View: sticky headers stay in view and leave no stale pixels", "[UI][GPU]")
  {
    RepaintFixture fixture(200u, 160u);
    View view({200, 160});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0; background: #f4f4f4 }"
      u8"h2 { position: sticky; top: 0; margin: 0; height: 20px; background: #073b4c }"
      u8"section { height: 150px; background: linear-gradient(#fde, #def) }"
      u8"#scroller { margin: 10px; width: 140px; height: 80px; overflow: auto; background: white }"
      u8"#scroller h3 { position: sticky; top: 0; margin: 0; height: 16px; background: #ef476f }"
      u8".row { height: 30px; border-bottom: 1px solid #888 }</style>"
      u8"<section><h2></h2></section>"
      u8"<div id=scroller><h3></h3><div class=row></div><div class=row></div><div class=row></div>"
      u8"<div class=row></div><div class=row></div><div class=row></div></div>"
      u8"<section><h2></h2></section><section><h2></h2></section>");
    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&]
    {
      time += 16ms;
      (void)view.Update(time);
      (void)fixture.Painted(view);
    };
    frame();
    frame();
    CheckNoStalePixels(fixture, view, "sticky-first");

    // The view: its sections' headers stick in turn (repainted, as they don't composite).
    for (int32 y : {40, 120, 200, 310, 90})
    {
      view.ScrollTo({0, y});
      frame();
      CheckNoStalePixels(fixture, view, "sticky-view-" + std::to_string(y));
    }

    // The box scrolls on layers: its header composites and stays at its top without repainting.
    view.ScrollTo({0, 0});
    frame();
    auto scroller = view.Document().GetElementById(u8"scroller");
    auto header = view.Document().QuerySelector(u8"#scroller h3").ReleaseValue();
    REQUIRE(header->Renderer()->IsComposited());
    TrackedRepaints repaints(view);
    for (int32 y : {20, 75, 10})
    {
      repaints.Reset();
      scroller->SetScrollTop(y);
      frame();
      CHECK(repaints.Bounds().IsEmpty());
      CHECK(repaints.Bounds(repaints.ScrolledContentsOf(u8"scroller")).IsEmpty());
      CheckNoStalePixels(fixture, view, "sticky-box-" + std::to_string(y));
    }
  }

  TEST_CASE("View: scrolling under a fixed backdrop repaints nothing", "[UI][GPU]")
  {
    const utf8_string page =
      u8"<!DOCTYPE html><style>body { margin: 0; height: 600px;"
      u8" background: repeating-linear-gradient(45deg, #073b4c 0 6px, #ffd166 6px 12px, #ef476f 12px 18px) }"
      u8"#glass { position: fixed; left: 20px; top: 20px; width: 64px; height: 64px; border-radius: 12px;"
      u8" background: rgba(255, 255, 255, 0.2); backdrop-filter: blur(4px) saturate(1.5) }</style>"
      u8"<div id=glass></div>";
    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&](View &view, RepaintFixture &fixture)
    {
      time += 16ms;
      (void)view.Update(time);
      return fixture.Painted(view);
    };

    RepaintFixture fixture(200u, 160u);
    View view({200, 160});
    view.LoadHTML(utf8_string(page));
    (void)frame(view, fixture);
    REQUIRE(view.Document().GetElementById(u8"glass")->Renderer()->IsComposited());
    TrackedRepaints repaints(view);

    for (int32 y : {7, 30, 95})
    {
      repaints.Reset();
      view.ScrollTo({0, y});
      auto painted = frame(view, fixture);
      CHECK(repaints.Bounds().IsEmpty());
      CHECK(repaints.Bounds(repaints.LayerOf(u8"glass")).IsEmpty());

      // The whole repaint doesn't draw backdrops, so the frame is compared with a fresh view of the page at
      // the same scroll position: the backdrop shows what's beneath it now.
      View fresh({200, 160});
      fresh.LoadHTML(utf8_string(page));
      (void)frame(fresh, fixture);
      fresh.ScrollTo({0, y});
      auto expected = frame(fresh, fixture);
      auto comparison = CompareImages(painted, expected, 2u);
      INFO("scrolled to " << y << ": largest difference " << comparison.LargestDifference << ", "
                          << comparison.PixelsOver << " pixels over 2");
      CHECK(comparison.PixelsOver == 0u);
    }
  }

  TEST_CASE("View: composited masks and clip paths paint as the whole page does", "[UI][GPU]")
  {
    RepaintFixture fixture(240u, 120u);
    View view({240, 120});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0; background: #f4f4f4 }"
      u8"div { position: absolute; width: 56px; height: 56px; will-change: transform;"
      u8" background: linear-gradient(135deg, #ef476f, #ffd166 50%, #118ab2) }"
      u8"div > div { left: 32px; top: 32px; width: 40px; height: 40px; background: #073b4c; will-change: "
      u8"auto }"
      u8"#masked { left: 16px; top: 24px; mask-image: linear-gradient(90deg, black 40%, transparent);"
      u8" transform: translateX(0) }"
      u8"#clipped { left: 96px; top: 24px; clip-path: circle(45%) }"
      u8"#both { left: 168px; top: 24px; filter: blur(1px);"
      u8" mask-image: radial-gradient(circle, black 50%, transparent 52%) }</style>"
      u8"<div id=masked><div></div></div><div id=clipped><div></div></div><div id=both><div></div></div>");
    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&]
    {
      time += 16ms;
      (void)view.Update(time);
      (void)fixture.Painted(view);
    };
    frame();
    frame();
    for (auto id : {u8"masked", u8"clipped", u8"both"})
    {
      auto *renderer = view.Document().GetElementById(id)->Renderer();
      REQUIRE(renderer->IsComposited());
      CHECK(krys::Downcast<boo::RenderLayerModelObject>(*renderer).Layer()->HasCompositedMask());
    }
    // A mask layer's edges are antialiased as the mask image or clip path painted into it, then applied to
    // the layer's (its edges' coverage counting again on a turned layer): edge pixels may differ a little
    // from the whole repaint, nothing else.
    auto checkMasked = [&](const std::string &step)
    {
      auto painted = fixture.Painted(view);
      auto repainted = fixture.Repainted(view);
      auto comparison = CompareImages(painted, repainted, 2u);
      auto edges = CompareImages(painted, repainted, 80u);
      INFO(step << ": largest difference " << comparison.LargestDifference << ", " << comparison.PixelsOver
                << " pixels over 2");
      CHECK(comparison.PixelsOver < 200u);
      CHECK(edges.PixelsOver == 0u);
    };
    checkMasked("composited-masks");

    // Moving a masked layer moves its mask with it, and repaints nothing (it had a transform, which keeps its
    // layout).
    TrackedRepaints repaints(view);
    repaints.Reset();
    auto masked = view.Document().GetElementById(u8"masked");
    REQUIRE_FALSE(
      masked->SetAttribute(u8"style", u8"transform: translate(4px, 10px) rotate(10deg)").HasException());
    frame();
    CHECK(repaints.Bounds().IsEmpty());
    CHECK(repaints.Bounds(repaints.LayerOf(u8"masked")).IsEmpty());
    checkMasked("composited-masks-moved");
  }

  TEST_CASE("View: changing a composited layer's opacity, transform or filter repaints nothing", "[UI][GPU]")
  {
    RepaintFixture fixture(200u, 100u);
    View view({200, 100});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0; background: #f4f4f4 }"
      u8"#blurred { position: absolute; left: 30px; top: 20px; width: 40px; height: 40px;"
      u8" background: #073b4c; filter: blur(2px); opacity: 0.9; transform: translateX(1px) }</style>"
      u8"<p>Text beside it</p><div id=blurred></div>");
    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&]
    {
      time += 16ms;
      (void)view.Update(time);
      (void)fixture.Painted(view);
    };
    frame();
    TrackedRepaints repaints(view);
    repaints.Reset();

    // Each change keeps the layer's stacking context and containing block, which would need a layout.
    auto blurred = view.Document().GetElementById(u8"blurred");
    auto change = [&](utf8_stringview style)
    {
      REQUIRE_FALSE(blurred->SetAttribute(u8"style", utf8_string(style)).HasException());
      frame();
      CHECK(repaints.Bounds().IsEmpty());
      CHECK(repaints.Bounds(repaints.LayerOf(u8"blurred")).IsEmpty());
      CheckNoStalePixels(fixture, view, "composited-change");
    };

    SECTION("opacity")
    {
      change(u8"opacity: 0.5");
    }

    SECTION("transform")
    {
      change(u8"transform: translateX(20px) rotate(10deg)");
    }

    SECTION("filter")
    {
      // The same blur, so the layer keeps its size.
      change(u8"filter: blur(2px) sepia(1)");
    }
  }

  TEST_CASE("View: animations of composited properties repaint nothing", "[UI][GPU]")
  {
    RepaintFixture fixture(200u, 100u);
    View view({200, 100});
    auto load = [&](utf8_stringview animation)
    {
      utf8_string markup =
        u8"<!DOCTYPE html><style>body { margin: 0 } @keyframes fade { from { opacity: 0 } to { opacity: 1 } }"
        u8"@keyframes spin { from { transform: rotate(0) } to { transform: rotate(90deg) } }"
        u8"@keyframes tint { from { filter: grayscale(0) } to { filter: grayscale(1) } }"
        u8"#box { margin: 20px; width: 30px; height: 20px; background: teal; animation: ";
      markup += animation;
      markup += u8" }</style><div id=box></div>";
      view.LoadHTML(krys::move(markup));
    };
    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&]
    {
      time += 16ms;
      bool updated = view.Update(time);
      (void)fixture.Painted(view);
      return updated;
    };
    // Each frame of an animation running on the box's layer: no rendering update, no repaints, but a paint.
    auto checkLayerAnimates = [&](const std::string &step)
    {
      (void)frame();
      (void)frame();
      TrackedRepaints repaints(view);
      REQUIRE(view.Document().GetElementById(u8"box")->Renderer()->IsComposited());
      for (int32 i = 0; i < 3; ++i)
      {
        repaints.Reset();
        CHECK_FALSE(frame());
        CHECK(view.NeedsPaint());
        CHECK(repaints.Bounds().IsEmpty());
        CHECK(repaints.Bounds(repaints.LayerOf(u8"box")).IsEmpty());
      }
      // The composited frame shows the animated value (comparing resolves the page's style, so it comes
      // last).
      CheckNoStalePixels(fixture, view, step);
    };

    SECTION("opacity")
    {
      load(u8"fade 1s linear infinite");
      checkLayerAnimates("layer-opacity");
    }

    SECTION("transform")
    {
      load(u8"spin 1s linear infinite");
      checkLayerAnimates("layer-transform");
    }

    SECTION("a filter, which layers don't animate, recomposites")
    {
      load(u8"tint 1s linear infinite");
      (void)frame();
      (void)frame();
      TrackedRepaints repaints(view);
      REQUIRE(view.Document().GetElementById(u8"box")->Renderer()->IsComposited());
      for (int32 i = 0; i < 3; ++i)
      {
        repaints.Reset();
        CHECK(frame());
        CHECK(repaints.Bounds().IsEmpty());
        CHECK(repaints.Bounds(repaints.LayerOf(u8"box")).IsEmpty());
        CheckNoStalePixels(fixture, view, "page-filter-" + std::to_string(i));
      }
    }

    SECTION("it ends")
    {
      load(u8"fade 100ms linear");
      for (int32 i = 0; i < 12; ++i)
      {
        (void)frame();
      }
      CHECK_FALSE(view.NeedsPaint());
      CheckNoStalePixels(fixture, view, "layer-animation-ended");
    }
  }

  TEST_CASE("View: an animated image asks for a rendering update for each frame", "[UI][GPU]")
  {
    const auto directory = std::filesystem::temp_directory_path() / "krystal-ui-animation-test";
    std::filesystem::create_directories(directory);
    {
      WriteAnimatedGif(directory / "spin.gif");
      std::ofstream page(directory / "page.html", std::ios::binary);
      page << "<!DOCTYPE html><img id=img src=spin.gif>";
    }

    RepaintFixture fixture(200u, 100u);
    View view({200, 100});
    REQUIRE(view.LoadFile(directory / "page.html").has_value());
    view.UpdateRendering();
    auto now = boo::html::EventLoop::Current().Now();
    // The load completes; painting the image starts its animation. The host paints after each update.
    CHECK(view.Update(now));
    (void)fixture.Painted(view);
    CHECK_FALSE(view.Update(now + 50ms));
    // Each new frame asks for a rendering update.
    CHECK(view.Update(now + 100ms));
    (void)fixture.Painted(view);
    CHECK_FALSE(view.Update(now + 150ms));
    CHECK(view.Update(now + 200ms));

    std::filesystem::remove_all(directory);
  }
}

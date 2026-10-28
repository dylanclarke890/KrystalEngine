#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Animation/DocumentTimeline.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/DOM/DOMRect.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayer.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2D.hpp"
#include "Krystal.Booey/HTML/Canvas/EngineTextureRenderingContext.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerBacking.hpp"
#include "Krystal.Booey/Rendering/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Editor/PageCanvas2D.hpp"
#include "Krystal.Editor/PageCube.hpp"
#include "Krystal.Gfx.Tests/Golden.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.UI.Tests/RepaintFixture.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/EngineTexture.hpp"
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
    WaitForLoad(view);

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
    WaitForLoad(view);

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
    WaitForLoad(view);
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
    WaitForLoad(view);
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
    WaitForLoad(view);
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
      WaitForLoad(view);
    }

    SECTION("a fixed background")
    {
      fixedBackground = true;
      view.LoadHTML(u8"<!DOCTYPE html><style>"
                    u8"body { margin: 0; height: 600px;"
                    u8" background: linear-gradient(#f88, #88f) fixed }"
                    u8"p { margin: 40px 10px; font: 14px 'Liberation Sans' }"
                    u8"</style><p>One</p><p>Two</p><p>Three</p><p>Four</p><p>Five</p><p>Six</p>");
      WaitForLoad(view);
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
    WaitForLoad(view);
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
    WaitForLoad(view);
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
    WaitForLoad(view);
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
    WaitForLoad(view);
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
      WaitForLoad(fresh);
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
    WaitForLoad(view);
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
    WaitForLoad(view);
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
      WaitForLoad(view);
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
    WaitForCommit(view);
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

  TEST_CASE("View: dragging a selection leaves no stale pixels", "[UI][GPU]")
  {
    static const auto fonts = View::AddFontDirectory(std::filesystem::path {KRYS_UI_TESTS_FONTS_DIR});
    (void)fonts;

    RepaintFixture fixture(240u, 160u);
    View view({240, 160});
    view.LoadHTML(u8"<!DOCTYPE html><style>"
                  u8"body { margin: 8px; font: 14px/18px 'Liberation Sans' }"
                  u8"p { margin: 0 0 6px } .box { margin: 0 20px; padding: 4px; background: #def }"
                  u8"::selection { background: #f80; color: white }"
                  u8"</style><p>The first paragraph wraps onto a second line here.</p>"
                  u8"<div class=box><p>A nested block, with its own gaps.</p></div><p>The last one.</p>");
    WaitForLoad(view);

    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&]
    {
      time += 16ms;
      (void)view.Update(time);
    };
    frame();
    CheckNoStalePixels(fixture, view, "selection-first");

    TrackedRepaints repaints(view);
    view.MousePressed({10, 14}, pal::MouseButton::Left);
    frame();
    // Down through the lines, then back up, each move repainting what the selection gained or lost.
    const boo::IntPoint points[] = {{120, 14}, {60, 32}, {100, 60}, {40, 90}, {200, 90}, {80, 32}, {30, 14}};
    int32 step = 0;
    for (const auto &point : points)
    {
      repaints.Reset();
      view.MouseMoved(point);
      frame();
      CHECK_FALSE(repaints.Bounds().IsEmpty());
      CheckNoStalePixels(fixture, view, "selection-drag-" + std::to_string(++step));
    }
    view.MouseMoved({200, 90});
    view.MouseReleased({200, 90}, pal::MouseButton::Left);
    frame();
    CHECK(view.Document().Selection().IsRange());
    CheckNoStalePixels(fixture, view, "selection-released");

    // Losing focus repaints the selection in the inactive colours, and a click clears it.
    repaints.Reset();
    view.SetFocused(false);
    frame();
    CheckNoStalePixels(fixture, view, "selection-inactive");
    view.SetFocused(true);
    // Inside the selection, and far enough from the press that started it not to count as a double click.
    view.MousePressed({60, 32}, pal::MouseButton::Left);
    view.MouseReleased({60, 32}, pal::MouseButton::Left);
    frame();
    CHECK_FALSE(view.Document().Selection().IsRange());
    CheckNoStalePixels(fixture, view, "selection-cleared");
  }

  TEST_CASE("View: a blinking caret repaints only itself and leaves no stale pixels", "[UI][GPU]")
  {
    static const auto fonts = View::AddFontDirectory(std::filesystem::path {KRYS_UI_TESTS_FONTS_DIR});
    (void)fonts;

    RepaintFixture fixture(240u, 120u);
    View view({240, 120});
    view.LoadHTML(u8"<!DOCTYPE html><style>"
                  u8"body { margin: 8px; font: 14px/18px 'Liberation Sans' }"
                  u8"#e { padding: 4px; background: #def; caret-color: #c00 }"
                  u8"</style><div id=e contenteditable>Click to place the caret here.</div>");
    WaitForLoad(view);

    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&](MonotonicTime::duration delta)
    {
      time += delta;
      (void)view.Update(time);
    };
    frame(16ms);
    CheckNoStalePixels(fixture, view, "caret-first");

    // A click places the caret and focuses the editing host.
    view.MousePressed({40, 20}, pal::MouseButton::Left);
    view.MouseReleased({40, 20}, pal::MouseButton::Left);
    frame(16ms);
    auto &selection = view.Document().Selection();
    REQUIRE(selection.IsCaret());
    CHECK(view.Document().FocusedElement() == view.Document().GetElementById(u8"e").get());
    CheckNoStalePixels(fixture, view, "caret-placed");

    // Each blink repaints the caret's rect alone.
    auto caret = boo::FloatRect(selection.AbsoluteCaretBounds());
    TrackedRepaints repaints(view);
    for (int32 blink = 1; blink <= 3; ++blink)
    {
      repaints.Reset();
      frame(550ms);
      auto bounds = repaints.Bounds();
      CHECK_FALSE(bounds.IsEmpty());
      CHECK(bounds.Width() <= caret.Width() + 2);
      CHECK(bounds.Height() <= caret.Height() + 2);
      CHECK(bounds.Intersects(caret));
      CheckNoStalePixels(fixture, view, "caret-blink-" + std::to_string(blink));
    }

    // The arrows move it, repainting where it was and where it is.
    repaints.Reset();
    CHECK(view.KeyPressed(pal::Key::RightArrow, {}));
    frame(16ms);
    CHECK(repaints.Bounds().Width() > caret.Width() + 2);
    CHECK(boo::FloatRect(selection.AbsoluteCaretBounds()).X() > caret.X());
    CheckNoStalePixels(fixture, view, "caret-moved");
  }

  TEST_CASE("View: dragging a selection past the edge scrolls", "[UI][GPU]")
  {
    static const auto fonts = View::AddFontDirectory(std::filesystem::path {KRYS_UI_TESTS_FONTS_DIR});
    (void)fonts;

    View view({240, 160});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; font: 14px/20px 'Liberation Sans' }</style>"
                  u8"<p>1</p><p>2</p><p>3</p><p>4</p><p>5</p><p>6</p><p>7</p><p>8</p><p>9</p><p>10</p>"
                  u8"<p>11</p><p>12</p><p>13</p><p>14</p><p>15</p><p>16</p><p>17</p><p>18</p>");
    WaitForLoad(view);
    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&](std::chrono::milliseconds step)
    {
      time += step;
      (void)view.Update(time);
    };
    frame(16ms);

    view.MousePressed({5, 30}, pal::MouseButton::Left);
    view.MouseMoved({5, 150});
    view.MouseMoved({5, 200});
    // The autoscroll timer scrolls toward the mouse every 50ms while it's held below the view.
    for (int32 i = 0; i < 10; ++i)
    {
      frame(50ms);
    }
    CHECK(view.ScrollPosition().Y() > 0);
    auto scrolled = view.ScrollPosition().Y();

    // Releasing stops it.
    view.MouseReleased({5, 200}, pal::MouseButton::Left);
    for (int32 i = 0; i < 4; ++i)
    {
      frame(50ms);
    }
    CHECK(view.ScrollPosition().Y() == scrolled);
    CHECK(view.Document().Selection().IsRange());
  }

  TEST_CASE("View: a texture canvas redraws its texture without repainting the page", "[UI][GPU]")
  {
    RepaintFixture fixture(200u, 100u);
    View view({200, 100});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0; background: #f4f4f4 }</style><p>Text beside it</p>"
      u8"<canvas id=canvas width=4 height=2 style='width: 80px; height: 40px'></canvas>");
    WaitForLoad(view);
    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&]
    {
      time += 16ms;
      (void)view.Update(time);
      return fixture.Painted(view);
    };
    (void)frame();

    // A 4 by 2 texture of one colour, which the host draws into.
    auto &device = fixture.Device();
    auto texture = device.CreateTexture({.Format = gfx::TextureFormat::BGRA8Unorm,
                                         .Width = 4u,
                                         .Height = 2u,
                                         .Usage = gfx::TextureUsage::Sampled,
                                         .DebugName = u8"Repaint.EngineTexture"});
    REQUIRE(texture.has_value());
    auto fill = [&](uint32 argb)
    {
      List<uint32> pixels(8u, argb);
      device.WriteTexture(*texture, gfx::TextureRegion::Whole2D(4u, 2u),
                          {reinterpret_cast<const byte *>(pixels.data()), pixels.size() * sizeof(uint32)});
    };
    fill(0xFF'33'66'99u);

    auto canvas = view.Document().GetElementById(u8"canvas");
    auto context = Downcast<boo::html::HTMLCanvasElement>(*canvas).GetContext(
      boo::html::EngineTextureRenderingContext::ContextType);
    REQUIRE(context);
    auto &textureContext = Downcast<boo::html::EngineTextureRenderingContext>(*context);
    textureContext.SetTexture(EngineTexture::Create(*texture, {4, 2}));
    auto before = frame();
    // Composited and painted the whole page again, which draws the texture as an image, the page is the
    // same.
    CheckNoStalePixels(fixture, view, "texture-canvas");

    TrackedRepaints repaints(view);
    repaints.Reset();

    // The host draws into the texture and says so: only the canvas's contents are redrawn.
    fill(0xFF'EE'77'33u);
    textureContext.TextureChanged();
    auto after = frame();
    CHECK(repaints.Bounds().IsEmpty());
    auto &canvasLayer = repaints.LayerOf(u8"canvas");
    CHECK_FALSE(canvasLayer.DrawsContent());
    CHECK(repaints.Bounds(canvasLayer) == canvasLayer.ContentsRect());
    auto changed = CompareImages(before, after, 2u);
    CHECK(changed.PixelsOver == 80u * 40u);
    CheckNoStalePixels(fixture, view, "texture-canvas-changed");

    textureContext.SetTexture(nullptr);
    (void)frame();
    device.Destroy(*texture);
  }

  TEST_CASE("View: a 2D canvas redraws its contents without repainting the page", "[UI][GPU]")
  {
    RepaintFixture fixture(200u, 100u);
    View view({200, 100});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0; background: #f4f4f4 }</style><p>Text beside it</p>"
      u8"<canvas id=canvas width=40 height=20 style='width: 80px; height: 40px'></canvas>");
    WaitForLoad(view);
    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&]
    {
      time += 16ms;
      (void)view.Update(time);
      return fixture.Painted(view);
    };
    (void)frame();

    auto &canvas = Downcast<boo::html::HTMLCanvasElement>(*view.Document().GetElementById(u8"canvas"));
    auto context = canvas.GetContext2d();
    REQUIRE(context);
    context->SetFillStyle(boo::dom::DOMString(u8"#336699"));
    context->FillRect(0, 0, 40, 20);
    auto before = frame();
    // Composited and painted the whole page again, which draws the canvas's texture as an image, the page is
    // the same.
    CheckNoStalePixels(fixture, view, "2d-canvas");

    TrackedRepaints repaints(view);
    repaints.Reset();

    // Drawing redraws only the canvas's contents.
    context->SetFillStyle(boo::dom::DOMString(u8"#ee7733"));
    context->FillRect(0, 0, 40, 20);
    auto after = frame();
    CHECK(repaints.Bounds().IsEmpty());
    auto &canvasLayer = repaints.LayerOf(u8"canvas");
    CHECK_FALSE(canvasLayer.DrawsContent());
    CHECK(repaints.Bounds(canvasLayer) == canvasLayer.ContentsRect());
    auto changed = CompareImages(before, after, 2u);
    CHECK(changed.PixelsOver == 80u * 40u);
    CheckNoStalePixels(fixture, view, "2d-canvas-changed");
  }

  TEST_CASE("View: the editor page's live 3D view", "[UI][GPU]")
  {
    RepaintFixture fixture(1'200u, 600u);
    View view({1'200, 600});
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    WaitForLoad(view);
    auto &document = view.Document();
    // As the editor makes it once the page has loaded, on the device it paints with.
    editor::PageCube cube(document, fixture.Device());

    auto canvas = document.GetElementById(u8"cube");
    REQUIRE(canvas);
    auto rect = canvas->GetBoundingClientRect();
    view.ScrollTo(boo::IntPoint(0, view.ScrollPosition().Y() + static_cast<int32>(rect->Top()) - 40));
    view.UpdateRendering();
    rect = canvas->GetBoundingClientRect();
    REQUIRE(rect->Width() == 240);
    REQUIRE(rect->Height() == 240);
    const auto left = static_cast<uint32>(rect->Left());
    const auto top = static_cast<uint32>(rect->Top());
    const auto centre = boo::IntPoint(static_cast<int32>(left + 120u), static_cast<int32>(top + 120u));

    // Each frame, the cube is drawn into its texture before the page is painted.
    auto frame = [&]
    {
      return fixture.PaintedAfter(view, [&](gfx::CommandBuffer &commands) { cube.Render(commands); });
    };
    auto pixel = [](const GoldenImage &image, uint32 x, uint32 y)
    {
      return image.Pixels[y * image.Width + x];
    };
    // #073b4c, the canvas's background, opaque.
    constexpr Array<uint8, 4> canvasBackground {0x07, 0x3B, 0x4C, 0xFF};

    auto before = frame();
    // The cube is in the middle of the canvas, its background around it, and the corners are rounded off.
    CHECK(pixel(before, left + 120u, top + 120u) != canvasBackground);
    CHECK(pixel(before, left + 6u, top + 120u) == canvasBackground);
    CHECK(pixel(before, left + 1u, top + 1u) != canvasBackground);
    // The same texture fills the wide canvas too.
    auto strip = document.GetElementById(u8"cube-strip")->GetBoundingClientRect();
    CHECK(pixel(before, static_cast<uint32>(strip->Left() + strip->Width() / 2),
                static_cast<uint32>(strip->Top() + strip->Height() / 2))
          != canvasBackground);

    // Dragging it turns the cube.
    view.MouseMoved(centre);
    view.MousePressed(centre, pal::MouseButton::Left);
    view.MouseMoved(boo::IntPoint(centre.X() + 30, centre.Y()));
    view.MouseMoved(boo::IntPoint(centre.X() + 60, centre.Y() + 20));
    view.MouseReleased(boo::IntPoint(centre.X() + 60, centre.Y() + 20), pal::MouseButton::Left);
    auto label = document.GetElementById(u8"cube-angle")->TextContent().value_or(boo::dom::DOMString {});
    CHECK(std::string(label.begin(), label.end()).starts_with("Turned "));
    auto after = frame();
    CHECK(CompareImages(before, after, 2u).PixelsOver > 1'000u);

    // Painted again whole, which draws the texture as an image, the canvases are as composited. (The page's
    // frosted badge, a backdrop filter, isn't compared: its blur differs a little between the two.)
    auto repainted = fixture.Repainted(view);
    auto sameWithin = [&](const boo::dom::DOMRectReadOnly &area)
    {
      uint32 differing = 0u;
      for (auto y = static_cast<uint32>(area.Top()); y < static_cast<uint32>(area.Bottom()); ++y)
      {
        for (auto x = static_cast<uint32>(area.Left()); x < static_cast<uint32>(area.Right()); ++x)
        {
          const auto &a = pixel(after, x, y);
          const auto &b = pixel(repainted, x, y);
          for (size_t channel = 0; channel < 4u; ++channel)
          {
            if (std::abs(int32(a[channel]) - int32(b[channel])) > 2)
            {
              ++differing;
              break;
            }
          }
        }
      }
      return differing;
    };
    CHECK(sameWithin(*canvas->GetBoundingClientRect()) == 0u);
    CHECK(sameWithin(*document.GetElementById(u8"cube-strip")->GetBoundingClientRect()) == 0u);
  }

  TEST_CASE("View: the editor page's canvas 2D sample", "[UI][GPU]")
  {
    RepaintFixture fixture(1'200u, 600u);
    View view({1'200, 600});
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    WaitForLoad(view);
    auto &document = view.Document();
    // As the editor makes it once the page has loaded; its readbacks need the device the view paints with.
    view.SetDevice(fixture.Device());
    editor::PageCanvas2D sample(document);

    auto chart = document.GetElementById(u8"chart");
    REQUIRE(chart);
    auto rect = chart->GetBoundingClientRect();
    view.ScrollTo(boo::IntPoint(0, view.ScrollPosition().Y() + static_cast<int32>(rect->Top()) - 40));
    view.UpdateRendering();
    rect = chart->GetBoundingClientRect();
    REQUIRE(rect->Width() == 320);
    REQUIRE(rect->Height() == 160);

    auto pixel = [](const GoldenImage &image, double x, double y)
    {
      return image.Pixels[static_cast<uint32>(y) * image.Width + static_cast<uint32>(x)];
    };
    constexpr Array<uint8, 4> white {0xFF, 0xFF, 0xFF, 0xFF};

    // The chart: the newest value's bar is filled with its pattern, and it's redrawn as values come in.
    view.UpdateRendering();
    auto before = fixture.Painted(view);
    const auto barX = rect->Left() + 296;
    const auto barY = rect->Top() + 139;
    CHECK(pixel(before, barX, barY) != white);
    CHECK(pixel(before, rect->Left() + 300, rect->Top() + 6) == white);
    const auto values = sample.Values();
    sample.Update(1.f);
    CHECK(sample.Values() != values);
    CHECK(sample.Values()[0] == values[1]);
    view.UpdateRendering();
    auto after = fixture.Painted(view);
    CHECK(CompareImages(before, after, 2u).PixelsOver > 100u);

    // The pad keeps what's drawn on it.
    auto pad = document.GetElementById(u8"pad")->GetBoundingClientRect();
    const auto from =
      boo::IntPoint(static_cast<int32>(pad->Left() + 40), static_cast<int32>(pad->Top() + 100));
    const auto to =
      boo::IntPoint(static_cast<int32>(pad->Left() + 200), static_cast<int32>(pad->Top() + 100));
    view.MouseMoved(from);
    view.MousePressed(from, pal::MouseButton::Left);
    view.MouseMoved(boo::IntPoint(from.X() + 80, from.Y()));
    view.MouseMoved(to);
    view.MouseReleased(to, pal::MouseButton::Left);
    view.UpdateRendering();
    auto drawn = fixture.Painted(view);
    // #1a2240, the default colour.
    CHECK(pixel(drawn, pad->Left() + 120, pad->Top() + 100) == Array<uint8, 4> {0x1A, 0x22, 0x40, 0xFF});
    CHECK(pixel(drawn, pad->Left() + 120, pad->Top() + 40) == white);

    // A colour picked from the palette's pixels draws the next stroke.
    auto palette = document.GetElementById(u8"pad-palette")->GetBoundingClientRect();
    const auto red =
      boo::IntPoint(static_cast<int32>(palette->Left() + 2), static_cast<int32>(palette->Top() + 14));
    view.MouseMoved(red);
    view.MousePressed(red, pal::MouseButton::Left);
    view.MouseReleased(red, pal::MouseButton::Left);
    auto swatch = document.GetElementById(u8"pad-colour")
                    ->GetAttribute(boo::dom::DOMStringAtom(u8"style"))
                    .value_or(boo::dom::DOMString {});
    CHECK(std::string(swatch.begin(), swatch.end()).starts_with("background: rgb("));
    const auto below = boo::IntPoint(from.X(), from.Y() + 40);
    view.MouseMoved(below);
    view.MousePressed(below, pal::MouseButton::Left);
    view.MouseMoved(boo::IntPoint(to.X(), below.Y()));
    view.MouseReleased(boo::IntPoint(to.X(), below.Y()), pal::MouseButton::Left);
    view.UpdateRendering();
    auto picked = fixture.Painted(view);
    const auto stroke = pixel(picked, pad->Left() + 120, pad->Top() + 140);
    CHECK(stroke[0] > 200);
    CHECK(stroke[2] < 80);

    // Clearing empties it.
    auto clear = document.GetElementById(u8"pad-clear")->GetBoundingClientRect();
    const auto button = boo::IntPoint(static_cast<int32>(clear->Left() + clear->Width() / 2),
                                      static_cast<int32>(clear->Top() + clear->Height() / 2));
    view.MouseMoved(button);
    view.MousePressed(button, pal::MouseButton::Left);
    view.MouseReleased(button, pal::MouseButton::Left);
    view.UpdateRendering();
    auto cleared = fixture.Painted(view);
    CHECK(pixel(cleared, pad->Left() + 120, pad->Top() + 100) == white);
    CHECK(pixel(cleared, pad->Left() + 120, pad->Top() + 140) == white);
  }
}

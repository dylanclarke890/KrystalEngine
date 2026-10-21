#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Gfx.Tests/Golden.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.UI/View.hpp"
#include <chrono>
#include <string>

namespace krys::ui::tests
{
  using namespace gfx::tests;
  using namespace std::chrono_literals;

  namespace
  {
    /// @brief Paints a view into a target of its size the way a host does each frame (`View::Paint`, which
    /// repaints only the damage), and as a full repaint of the page (`Record` and `Replay`), to compare.
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

      /// @brief The page as the host shows it: the page texture, repainted where it was damaged.
      KRYS_NODISCARD GoldenImage Painted(View &view)
      {
        return Draw([&] { view.Paint(*_renderer); });
      }

      /// @brief The whole page painted again.
      KRYS_NODISCARD GoldenImage Repainted(View &view)
      {
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

    for (int32 step = 1; step <= 6; ++step)
    {
      // Down in steps of different sizes, then back up; twice in one frame on the third step.
      auto y = step <= 4 ? step * 23 : (8 - step) * 31;
      view.ScrollTo({0, y});
      if (step == 3)
      {
        view.ScrollTo({0, y + 7});
      }
      frame();
      CheckNoStalePixels(fixture, view, "scroll-" + std::to_string(step));
    }
  }

  TEST_CASE("View: damage is the rect that changed", "[UI]")
  {
    View view({200, 100});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0 } #box { width: 50px; height: 50px; background: red }"
      u8"#box:hover { background: blue }"
      u8"#spin { position: absolute; left: 100px; top: 20px; width: 20px; height: 20px; background: teal }"
      u8"</style><div id=box></div><div id=spin></div>");
    auto &frameView = *view.Frame().View();
    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&]
    {
      time += 16ms;
      (void)view.Update(time);
      (void)view.Document().UpdateLayout();
      return frameView.TakeDamage();
    };
    (void)frame();
    REQUIRE(frame().IsEmpty());

    SECTION("a colour")
    {
      auto box = view.Document().GetElementById(u8"box");
      REQUIRE_FALSE(box->SetAttribute(u8"style", u8"background: green").HasException());
      CHECK(frame().Bounds() == boo::IntRect(0, 0, 50, 50));
    }

    SECTION("hovering")
    {
      view.MouseMoved({10, 10});
      CHECK(frame().Bounds() == boo::IntRect(0, 0, 50, 50));
    }

    SECTION("scrolling moves the painted contents; only what comes into view is damaged")
    {
      auto tall = view.Document().GetElementById(u8"box");
      REQUIRE_FALSE(tall->SetAttribute(u8"style", u8"height: 500px").HasException());
      (void)frame();
      view.ScrollTo({0, 30});
      (void)view.Document().UpdateLayout();
      auto scrolled = frameView.TakeScrolledContents();
      REQUIRE(scrolled.has_value());
      CHECK(scrolled->Delta == boo::IntSize(0, -30));
      // The bottom 30 pixels come into view, and the scrollbar's thumb moves; what was in view isn't damaged.
      auto damage = frameView.TakeDamage();
      CHECK(damage.Contains(boo::Region(boo::IntRect(0, 70, 150, 30))));
      CHECK_FALSE(damage.Contains(boo::IntPoint(10, 10)));
      CHECK_FALSE(damage.Contains(boo::IntPoint(100, 60)));
    }

    SECTION("a transform")
    {
      auto spin = view.Document().GetElementById(u8"spin");
      REQUIRE_FALSE(spin->SetAttribute(u8"style", u8"transform: rotate(45deg)").HasException());
      auto damage = frame().Bounds();
      // The box before and after turning: about 28 pixels across, around its centre (110, 30).
      CHECK(boo::IntRect(94, 14, 32, 32).Contains(damage));
      CHECK(damage.Contains(boo::IntRect(100, 20, 20, 20)));
    }
  }
}

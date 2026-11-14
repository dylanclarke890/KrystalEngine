#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Media/MediaQueryList.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.PAL/Display.hpp"
#include "Krystal.PAL/Win32/DisplayColour.hpp"
#include "Krystal.UI.Tests/RepaintFixture.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/View.hpp"
#include <chrono>
#include <cstdlib>
#include <string>
#ifndef NOMINMAX
  #define NOMINMAX
#endif
#include <windows.h>

namespace krys::ui::tests
{
  using namespace std::chrono_literals;

  namespace
  {
    using Pixel = Array<uint8, 4>;

    KRYS_NODISCARD std::string ToString(const Pixel &pixel)
    {
      return std::to_string(pixel[0]) + "," + std::to_string(pixel[1]) + "," + std::to_string(pixel[2]) + ","
             + std::to_string(pixel[3]);
    }

    /// @brief Checks the pixel at @p x, @p y of @p image is @p expected, within 2 a channel.
    void CheckPixel(const GoldenImage &image, uint32 x, uint32 y, const Pixel &expected)
    {
      const auto actual = image.Pixels[static_cast<size_t>(y) * image.Width + x];
      INFO("pixel " << x << "," << y << " is " << ToString(actual) << ", expected " << ToString(expected));
      bool close = true;
      for (size_t channel = 0; channel < 4u; ++channel)
      {
        close = close && std::abs(static_cast<int32>(actual[channel]) - expected[channel]) <= 2;
      }
      CHECK(close);
    }

    /// @brief @p html painted in a 40 by 20 view.
    KRYS_NODISCARD GoldenImage Paint(std::u8string html)
    {
      RepaintFixture fixture(40u, 20u);
      View view({40, 20});
      view.LoadHTML(krys::move(html));
      WaitForLoad(view);
      (void)view.Update(boo::html::EventLoop::Current().Now() + 1s);
      return fixture.Painted(view);
    }
  }

  TEST_CASE("SVG masks: a luminance mask's luminance is in linear light with color-interpolation: linearRGB",
            "[UI][GPU][SVG][ColorManagement]")
  {
    // White masked by mid grey, over black: half of white in sRGB, about a fifth (mid grey's light) in linear
    // light.
    auto image = Paint(u8"<!DOCTYPE html><style>html { background: black } body { margin: 0 }"
                       u8" svg { display: block }</style>"
                       u8"<svg width=40 height=20>"
                       u8"<mask id=srgb><rect width=20 height=20 fill=rgb(128,128,128) /></mask>"
                       u8"<mask id=linear style='color-interpolation: linearRGB'>"
                       u8"<rect x=20 width=20 height=20 fill=rgb(128,128,128) /></mask>"
                       u8"<rect width=20 height=20 fill=white mask=url(#srgb) />"
                       u8"<rect x=20 width=20 height=20 fill=white mask=url(#linear) />"
                       u8"</svg>");
    CheckPixel(image, 10, 10, {128, 128, 128, 255});
    CheckPixel(image, 30, 10, {55, 55, 55, 255});
  }

  TEST_CASE("Display colour: a display's gamut by the area its primaries span", "[UI][ColorManagement]")
  {
    using pal::DisplayGamut;
    using pal::GamutOfPrimaries;
    CHECK(GamutOfPrimaries({0.64f, 0.33f}, {0.30f, 0.60f}, {0.15f, 0.06f}) == DisplayGamut::SRGB);
    // A display narrower than sRGB (72% of NTSC's gamut) still counts as sRGB.
    CHECK(GamutOfPrimaries({0.63f, 0.34f}, {0.31f, 0.595f}, {0.155f, 0.07f}) == DisplayGamut::SRGB);
    CHECK(GamutOfPrimaries({0.68f, 0.32f}, {0.265f, 0.69f}, {0.15f, 0.06f}) == DisplayGamut::DisplayP3);
    // A laptop panel covering most of Display P3, and Adobe RGB, which spans about as much.
    CHECK(GamutOfPrimaries({0.676f, 0.318f}, {0.27f, 0.68f}, {0.151f, 0.059f}) == DisplayGamut::DisplayP3);
    CHECK(GamutOfPrimaries({0.64f, 0.33f}, {0.21f, 0.71f}, {0.15f, 0.06f}) == DisplayGamut::DisplayP3);
    CHECK(GamutOfPrimaries({0.708f, 0.292f}, {0.17f, 0.797f}, {0.131f, 0.046f}) == DisplayGamut::Rec2020);
  }

  TEST_CASE("View: the display's colours are the page's media features", "[UI][GPU][ColorManagement]")
  {
    RepaintFixture fixture(40u, 20u);
    View view({40, 20});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 20px; background: red }"
                  u8" @media (color-gamut: p3) { div { background: lime } }</style><div></div>");
    WaitForLoad(view);
    auto time = boo::html::EventLoop::Current().Now() + 1s;
    auto frame = [&]
    {
      time += 16ms;
      (void)view.Update(time);
      return fixture.Painted(view);
    };

    auto &window = *view.Document().Window();
    auto p3 = window.MatchMedia(u8"(color-gamut: p3)");
    REQUIRE(p3);
    int32 changes = 0;
    auto onChange =
      p3->AddEventListener(boo::dom::DOMStringAtom(u8"change"),
                           Func<void(boo::dom::Event &)>([&](boo::dom::Event &) { ++changes; }));
    CHECK_FALSE(p3->Matches());
    CHECK(window.MatchMedia(u8"(color: 8)")->Matches());
    CheckPixel(frame(), 20, 10, {255, 0, 0, 255});

    // The host says the display shows Display P3 at 10 bits a component.
    view.SetDisplayColour({.Gamut = pal::DisplayGamut::DisplayP3, .BitsPerComponent = 10u});
    CheckPixel(frame(), 20, 10, {0, 255, 0, 255});
    CHECK(p3->Matches());
    CHECK(changes == 1);
    CHECK(window.MatchMedia(u8"(color: 10)")->Matches());
    CHECK_FALSE(window.MatchMedia(u8"(color-gamut: rec2020)")->Matches());
    CHECK(window.MatchMedia(u8"(dynamic-range: standard)")->Matches());
    CHECK_FALSE(window.MatchMedia(u8"(dynamic-range: high)")->Matches());

    // Saying the same again changes nothing.
    view.SetDisplayColour({.Gamut = pal::DisplayGamut::DisplayP3, .BitsPerComponent = 10u});
    (void)frame();
    CHECK(changes == 1);

    view.SetDisplayColour(
      {.Gamut = pal::DisplayGamut::Rec2020, .HighDynamicRange = true, .BitsPerComponent = 10u});
    (void)frame();
    CHECK(window.MatchMedia(u8"(color-gamut: rec2020)")->Matches());
    CHECK(window.MatchMedia(u8"(dynamic-range: high)")->Matches());
    CHECK(p3->Matches());
  }

  TEST_CASE("Display colour: reads the primary display's colours from the system", "[UI][ColorManagement]")
  {
    const auto colour = pal::win32::ReadDisplayColour(::MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY));
    INFO("gamut " << static_cast<int>(colour.Gamut) << ", HDR " << colour.HighDynamicRange << ", "
                  << colour.BitsPerComponent << " bits, peak " << colour.MaxLuminance << " nits, SDR white "
                  << colour.SDRWhiteLevel << " nits");
    CHECK(colour.BitsPerComponent >= 8u);
    CHECK(colour.SDRWhiteLevel > 0.f);
    // Showing high dynamic range counts as showing Rec. 2020's gamut.
    CHECK((!colour.HighDynamicRange || colour.Gamut == pal::DisplayGamut::Rec2020));
    WARN("gamut " << static_cast<int>(colour.Gamut) << ", HDR " << colour.HighDynamicRange << ", "
                  << colour.BitsPerComponent << " bits, peak " << colour.MaxLuminance << " nits, SDR white "
                  << colour.SDRWhiteLevel << " nits");
  }
}

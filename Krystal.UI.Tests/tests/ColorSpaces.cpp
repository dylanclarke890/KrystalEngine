#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.UI.Tests/RepaintFixture.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/View.hpp"
#include <chrono>
#include <cstdlib>
#include <string>

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
}

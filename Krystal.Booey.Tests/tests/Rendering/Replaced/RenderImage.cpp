#include "Krystal.Booey/Rendering/Replaced/RenderImage.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Images/BitmapImage.hpp"
#include "Krystal.Booey/Graphics/Images/Image.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Threading/JobSystem.hpp"
#include "Krystal.Threading/ManualWorkQueue.hpp"
#include <chrono>
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    /// @brief @p text as a `std::string`, which Catch can print.
    KRYS_NODISCARD std::string Narrow(utf8_stringview text) noexcept
    {
      return std::string(text.begin(), text.end());
    }

    // 400x400 decodes to 640KB, over the 500KB that makes a still image large for decoding.
    constexpr uint8 LargePNG[] = {
      137, 80,  78,  71,  13,  10,  26,  10,  0,   0,   0,   13,  73,  72,  68,  82,  0,   0,   1,   144, 0,
      0,   1,   144, 8,   6,   0,   0,   0,   128, 191, 54,  204, 0,   0,   4,   129, 73,  68,  65,  84,  120,
      218, 237, 213, 49,  1,   0,   0,   8,   195, 176, 73,  199, 249, 208, 192, 77,  142, 24,  232, 211, 100,
      90,  0,   56,  19,  1,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   12,  4,   0,   3,
      1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,
      12,  4,   0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   12,  4,   0,   3,
      1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,
      12,  4,   0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   12,  4,   0,   3,
      1,   192, 64,  0,   48,  16,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,
      48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   48,  16,  0,   12,  4,   0,   3,
      1,   192, 64,  0,   48,  16,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,
      48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   48,  16,  0,   12,  4,   0,   3,
      1,   192, 64,  0,   48,  16,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  17,
      0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   192, 64,  0,   48,  16,  0,   12,  4,   0,
      3,   1,   192, 64,  0,   192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   192, 64,
      0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   192, 64,  0,   48,  16,  0,   12,  4,   0,
      3,   1,   192, 64,  0,   192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   192, 64,
      0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   192, 64,  0,   48,  16,  0,   12,  4,   0,
      3,   1,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   0,   3,   1,   192, 64,
      0,   48,  16,  0,   12,  4,   0,   3,   1,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,   0,
      3,   1,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   0,   3,   1,   192, 64,
      0,   48,  16,  0,   12,  4,   0,   3,   1,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,   0,
      3,   1,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   17,  1,   0,   3,   1,   192,
      64,  0,   48,  16,  0,   12,  4,   0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,
      0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   12,  4,   0,   3,   1,   192,
      64,  0,   48,  16,  0,   12,  4,   0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,
      0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   12,  4,   0,   3,   1,   192,
      64,  0,   48,  16,  0,   12,  4,   0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   48,  16,
      0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192,
      64,  0,   48,  16,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   48,  16,
      0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192,
      64,  0,   48,  16,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   48,  16,
      0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  17,  0,   48,  16,  0,   12,  4,   0,   3,   1,
      192, 64,  0,   192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   192, 64,  0,   48,
      16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,
      192, 64,  0,   192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   192, 64,  0,   48,
      16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,
      192, 64,  0,   192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   0,   17,  0,   48,  16,  0,   12,
      4,   0,   3,   1,   192, 64,  0,   192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,
      192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   192, 64,  0,   48,  16,  0,   12,
      4,   0,   3,   1,   192, 64,  0,   192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,
      192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   192, 64,  0,   48,  16,  0,   12,
      4,   0,   3,   1,   192, 64,  0,   192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   0,   3,   1,
      192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,
      4,   0,   3,   1,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   0,   3,   1,
      192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,
      4,   0,   3,   1,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   1,   0,   3,   1,
      192, 64,  0,   48,  16,  0,   12,  4,   0,   3,   17,  1,   0,   3,   1,   192, 64,  0,   48,  16,  0,
      12,  4,   0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   12,  4,   0,   3,
      1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,
      12,  4,   0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   12,  4,   0,   3,
      1,   192, 64,  0,   48,  16,  0,   12,  4,   0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,
      12,  4,   0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   48,  16,  0,   12,  4,   0,   3,
      1,   192, 64,  0,   48,  16,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,
      48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,   48,  16,  0,   12,  4,   0,   3,
      1,   192, 64,  0,   48,  16,  0,   48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   48,  16,  0,
      48,  16,  0,   12,  4,   0,   3,   1,   192, 64,  0,   248, 106, 1,   210, 36,  249, 60,  169, 120, 12,
      62,  0,   0,   0,   0,   73,  69,  78,  68,  174, 66,  96,  130};
    constexpr uint8 SmallPNG[] = {
      137, 80,  78,  71,  13,  10,  26,  10,  0,   0,  0,  13, 73, 72, 68, 82,  0,   0,   0,   20,  0,   0,
      0,   20,  8,   6,   0,   0,   0,   141, 137, 29, 13, 0,  0,  0,  29, 73,  68,  65,  84,  120, 218, 99,
      96,  104, 248, 255, 159, 170, 120, 212, 192, 81, 3,  71, 13, 28, 53, 112, 212, 192, 145, 106, 32,  0,
      50,  228, 229, 14,  187, 233, 42,  66,  0,   0,  0,  0,  73, 69, 78, 68,  174, 66,  96,  130};

    // Two 200x200 frames of 100ms: 160KB decoded, over the 100KB that makes an animation large for
    // decoding.
    constexpr uint8 AnimatedPNG[] = {
      137, 80,  78,  71,  13,  10,  26,  10,  0,   0,   0,   13,  73,  72,  68,  82,  0,   0,   0,   200, 0,
      0,   0,   200, 8,   6,   0,   0,   0,   173, 88,  174, 158, 0,   0,   0,   8,   97,  99,  84,  76,  0,
      0,   0,   2,   0,   0,   0,   0,   243, 141, 147, 112, 0,   0,   0,   26,  102, 99,  84,  76,  0,   0,
      0,   0,   0,   0,   0,   200, 0,   0,   0,   200, 0,   0,   0,   0,   0,   0,   0,   0,   0,   100, 3,
      232, 0,   0,   72,  42,  86,  193, 0,   0,   1,   158, 73,  68,  65,  84,  120, 218, 237, 211, 49,  13,
      0,   0,   8,   192, 176, 249, 55,  13,  6,   80,  64,  122, 212, 192, 146, 53,  53,  192, 45,  17,  192,
      32,  96,  16,  48,  8,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  128, 65,  192, 32,  96,
      16,  48,  8,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  66,  128, 65,  192, 32,  96,  16,
      48,  8,   24,  4,   12,  2,   6,   1,   131, 128, 65,  0,   131, 128, 65,  192, 32,  96,  16,  48,  8,
      24,  4,   12,  2,   6,   1,   131, 128, 65,  0,   131, 128, 65,  192, 32,  96,  16,  48,  8,   24,  4,
      12,  2,   6,   1,   131, 0,   6,   1,   131, 128, 65,  192, 32,  96,  16,  48,  8,   24,  4,   12,  2,
      6,   1,   131, 0,   6,   1,   131, 128, 65,  192, 32,  96,  16,  48,  8,   24,  4,   12,  2,   6,   1,
      12,  2,   6,   1,   131, 128, 65,  192, 32,  96,  16,  48,  8,   24,  4,   12,  2,   6,   1,   12,  2,
      6,   1,   131, 128, 65,  192, 32,  96,  16,  48,  8,   24,  4,   12,  2,   6,   17,  1,   12,  2,   6,
      1,   131, 128, 65,  192, 32,  96,  16,  48,  8,   24,  4,   12,  2,   24,  4,   12,  2,   6,   1,   131,
      128, 65,  192, 32,  96,  16,  48,  8,   24,  4,   12,  34,  4,   24,  4,   12,  2,   6,   1,   131, 128,
      65,  192, 32,  96,  16,  48,  8,   24,  4,   48,  8,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192,
      32,  96,  16,  48,  8,   24,  4,   48,  8,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  96,
      16,  48,  8,   96,  16,  48,  8,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  96,  16,  48,
      8,   96,  16,  48,  8,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  96,  16,  192, 32,  96,
      16,  48,  8,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  96,  16,  192, 32,  96,  16,  48,
      8,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  96,  16,  17,  192, 32,  96,  16,  48,  8,
      24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  128, 65,  192, 32,  96,  16,  48,  8,   24,  4,
      12,  2,   6,   1,   131, 128, 65,  192, 32,  66,  128, 65,  192, 32,  96,  16,  48,  8,   24,  4,   12,
      2,   6,   129, 143, 22,  63,  250, 89,  186, 188, 28,  21,  196, 0,   0,   0,   26,  102, 99,  84,  76,
      0,   0,   0,   1,   0,   0,   0,   200, 0,   0,   0,   200, 0,   0,   0,   0,   0,   0,   0,   0,   0,
      100, 3,   232, 0,   0,   211, 89,  188, 21,  0,   0,   1,   157, 102, 100, 65,  84,  0,   0,   0,   2,
      120, 218, 237, 211, 49,  13,  0,   0,   12,  195, 176, 242, 39,  221, 33,  40,  129, 201, 135, 9,   68,
      74,  146, 22,  88,  68,  0,   131, 128, 65,  192, 32,  96,  16,  48,  8,   24,  4,   12,  2,   6,   1,
      131, 0,   6,   1,   131, 128, 65,  192, 32,  96,  16,  48,  8,   24,  4,   12,  2,   6,   1,   131, 0,
      6,   1,   131, 128, 65,  192, 32,  96,  16,  48,  8,   24,  4,   12,  2,   6,   1,   12,  2,   6,   1,
      131, 128, 65,  192, 32,  96,  16,  48,  8,   24,  4,   12,  2,   6,   1,   12,  2,   6,   1,   131, 128,
      65,  192, 32,  96,  16,  48,  8,   24,  4,   12,  2,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192,
      32,  96,  16,  48,  8,   24,  4,   12,  2,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  96,
      16,  48,  8,   24,  4,   48,  8,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  96,  16,  48,
      8,   24,  4,   48,  8,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  96,  16,  48,  8,   32,
      2,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  96,  16,  48,  8,   24,  4,   48,  8,   24,
      4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  96,  16,  48,  8,   24,  4,   48,  8,   24,  4,   12,
      2,   6,   1,   131, 128, 65,  192, 32,  96,  16,  48,  8,   96,  16,  48,  8,   24,  4,   12,  2,   6,
      1,   131, 128, 65,  192, 32,  96,  16,  48,  8,   96,  16,  48,  8,   24,  4,   12,  2,   6,   1,   131,
      128, 65,  192, 32,  96,  16,  192, 32,  96,  16,  48,  8,   24,  4,   12,  2,   6,   1,   131, 128, 65,
      192, 32,  96,  16,  192, 32,  96,  16,  48,  8,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,
      128, 65,  192, 32,  96,  16,  48,  8,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  128, 65,
      192, 32,  96,  16,  48,  8,   24,  4,   12,  2,   6,   1,   131, 128, 65,  0,   17,  192, 32,  96,  16,
      48,  8,   24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  128, 65,  192, 32,  96,  16,  48,  8,
      24,  4,   12,  2,   6,   1,   131, 128, 65,  192, 32,  128, 65,  192, 32,  96,  16,  48,  8,   24,  4,
      12,  2,   6,   129, 135, 14,  230, 50,  89,  186, 234, 60,  254, 56,  0,   0,   0,   0,   73,  69,  78,
      68,  174, 66,  96,  130};

    /// @brief Gives Booey @p workQueue as the host's, for as long as it lives.
    struct HostWorkQueueScope
    {
      explicit HostWorkQueueScope(WorkQueue &workQueue) noexcept
      {
        PlatformStrategies::Singleton().SetWorkQueue(&workQueue);
      }

      ~HostWorkQueueScope() noexcept
      {
        PlatformStrategies::Singleton().SetWorkQueue(nullptr);
      }
    };

    /// @brief A page showing a large or a small image, loaded.
    struct ImagePage
    {
      LoaderFixture Loader;
      UniquePtr<PageInFrameFixture> Fixture;

      explicit ImagePage(Span<const uint8> png, utf8_stringview attributes = {}) noexcept
      {
        Loader.Strategy.Resources[u8"file:///image.png"] = png;
        Fixture =
          CreateUnique<PageInFrameFixture>(u8"<style>body{margin:0}</style><img id=i "
                                             + utf8_string(attributes) + u8" src='file:///image.png'>",
                                           IntSize(500, 500));
        (void)Fixture->Document->UpdateLayout();
        RunNetworkingTasks();
      }

      KRYS_NODISCARD html::HTMLImageElement &Image() noexcept
      {
        return Downcast<html::HTMLImageElement>(Fixture->ById(u8"i"));
      }
    };

    KRYS_NODISCARD bool DrawsImage(PageInFrameFixture &fixture) noexcept
    {
      return fixture.Painted().find(u8"draw-native-image") != utf8_string::npos;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "RenderImage: alt text of both directions is drawn in visual order",
                   "[Rendering][RenderImage]")
  {
    // "ab", then Hebrew shin and lamed: the Hebrew is drawn right to left, lamed first.
    PaintFixture fixture(u8"<!DOCTYPE html><img alt='ab\u05E9\u05DC' style='width:150px;height:40px'>");
    CHECK(DrawnText(*fixture.PaintedDisplayList(), u8"\u05E9\u05DC") == u8"\u05DC\u05E9");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "RenderImage: with dir=auto, alt text takes its first strong character's direction",
                   "[Rendering][RenderImage][html:directionality-of-the-attribute]")
  {
    // Hebrew shin and lamed, then "ab" (drawn as two runs): right to left, "ab" is on the left.
    auto drawn = [](const char8_t *attributes)
    {
      PaintFixture fixture(utf8_string(u8"<!DOCTYPE html><img ") + attributes
                           + u8" style='width:150px;height:40px'>");
      return Narrow(DrawnText(*fixture.PaintedDisplayList(), u8"ab\u05E9\u05DC"));
    };
    CHECK(drawn(u8"dir=auto alt='\u05E9\u05DCab'") == Narrow(u8"ab|\u05DC\u05E9"));
    CHECK(drawn(u8"dir=auto title='\u05E9\u05DCab'") == Narrow(u8"ab|\u05DC\u05E9"));
    // Otherwise the image's direction.
    CHECK(drawn(u8"alt='\u05E9\u05DCab'") == Narrow(u8"\u05DC\u05E9|ab"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "RenderImage: a large image decodes on the work queue when first painted",
                   "[Rendering][RenderImage][html:image-decoding-hint]")
  {
    ManualWorkQueue queue;
    HostWorkQueueScope scope(queue);
    ImagePage page(LargePNG);
    auto &fixture = page.Fixture;
    auto &image = page.Image();
    REQUIRE(image.GetCachedImage());
    REQUIRE(image.GetCachedImage()->HasImage());

    SECTION("it draws nothing until the decoded frame arrives, which repaints it")
    {
      CHECK_FALSE(DrawsImage(*fixture));
      CHECK(queue.PendingCount() == 1);
      // Painting again while it decodes doesn't ask again.
      CHECK_FALSE(DrawsImage(*fixture));
      CHECK(queue.PendingCount() == 1);

      CHECK(queue.RunAll() == 1);
      // The frame waits in the event loop's inbox.
      CHECK_FALSE(DrawsImage(*fixture));
      (void)fixture->Frame->View()->TakeDamage();

      html::EventLoop::Current().RunUntilIdle();
      CHECK(fixture->Frame->View()->HasDamage());
      CHECK(DrawsImage(*fixture));
      CHECK(queue.PendingCount() == 0);
      CHECK(image.HasEverPaintedImages());
    }

    SECTION("an image that's gone when its frame arrives drops it")
    {
      CHECK_FALSE(DrawsImage(*fixture));
      CHECK(queue.RunAll() == 1);
      image.Remove();
      fixture.reset();
      html::EventLoop::Current().RunUntilIdle();
    }

    SECTION("decoding=sync decodes it at once")
    {
      REQUIRE_FALSE(image.SetAttribute(u8"decoding", u8"sync").HasException());
      CHECK(DrawsImage(*fixture));
      CHECK(queue.PendingCount() == 0);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "RenderImage: small images, and any without a work queue, decode at once",
                   "[Rendering][RenderImage][html:image-decoding-hint]")
  {
    SECTION("small, with a work queue")
    {
      ManualWorkQueue queue;
      HostWorkQueueScope scope(queue);
      ImagePage page(SmallPNG);
      CHECK(DrawsImage(*page.Fixture));
      CHECK(queue.PendingCount() == 0);
    }

    SECTION("large, without one")
    {
      ImagePage page(LargePNG);
      CHECK(DrawsImage(*page.Fixture));
    }

    SECTION("large with decoding=async, without one")
    {
      ImagePage page(LargePNG, u8"decoding=async");
      CHECK(DrawsImage(*page.Fixture));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderImage: a large image decodes on the job system's workers",
                   "[Rendering][RenderImage][html:image-decoding-hint]")
  {
    UniquePtr<ImagePage> page;
    {
      JobSystem jobs(2);
      HostWorkQueueScope scope(jobs);
      page = CreateUnique<ImagePage>(Span<const uint8>(LargePNG));
      CHECK_FALSE(DrawsImage(*page->Fixture));
      // The workers finish the decode as the system goes.
    }
    html::EventLoop::Current().RunUntilIdle();
    CHECK(DrawsImage(*page->Fixture));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "RenderImage: an animation decodes its next frame ahead, on the work queue",
                   "[Rendering][RenderImage]")
  {
    using namespace std::chrono_literals;
    ManualWorkQueue queue;
    HostWorkQueueScope scope(queue);
    ImagePage page(AnimatedPNG);
    auto &loop = html::EventLoop::Current();
    auto &image = Downcast<BitmapImage>(*page.Image().GetCachedImage()->GetImage());
    REQUIRE(image.FrameCount() == 2);

    // The first frame decodes as it's drawn, and asks for the second.
    CHECK(DrawsImage(*page.Fixture));
    CHECK(image.CurrentFrameIndex() == 0);
    CHECK(queue.PendingCount() == 1);

    SECTION("decoded before its time, the frame shows when it's due")
    {
      CHECK(queue.RunAll() == 1);
      loop.RunUntilIdle();
      CHECK(image.CurrentFrameIndex() == 0);
      loop.Run(loop.Now() + 100ms);
      CHECK(image.CurrentFrameIndex() == 1);
    }

    SECTION("still decoding when it's due, the animation waits for it")
    {
      loop.Run(loop.Now() + 100ms);
      CHECK(image.CurrentFrameIndex() == 0);
      CHECK(queue.PendingCount() == 1);
    }
  }
}

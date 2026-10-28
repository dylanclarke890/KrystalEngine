#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Graphics/TestAcceleratedImageBuffers.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerContentsDisplayDelegate.hpp"

namespace krys::boo::tests
{
  TEST_CASE("ImageBuffer: rendering modes and the host's backends", "[Graphics][ImageBuffer]")
  {
    SECTION("without a host, every mode records into a display list")
    {
      for (auto mode : {RenderingMode::Unaccelerated, RenderingMode::Accelerated, RenderingMode::DisplayList})
      {
        auto buffer =
          ImageBuffer::Create({10, 20}, mode, RenderingPurpose::Canvas, 2, DestinationColorSpace::SRGB());
        REQUIRE(buffer);
        CHECK(buffer->GetRenderingMode() == RenderingMode::DisplayList);
        CHECK(buffer->Purpose() == RenderingPurpose::Canvas);
        CHECK(buffer->BackendSize() == IntSize(20, 40));
        CHECK_FALSE(buffer->LayerContentsDisplayDelegate());
      }
      CHECK(ImageBuffer::Create({10, 20}, 1)->Purpose() == RenderingPurpose::Unspecified);
      CHECK_FALSE(ImageBuffer::Create({0, 20}, RenderingMode::Accelerated, RenderingPurpose::Canvas, 1,
                                      DestinationColorSpace::SRGB()));
    }

    SECTION("the host makes the modes it has backends for")
    {
      AcceleratingChromeClient client;
      auto accelerated = ImageBuffer::Create({10, 20}, RenderingMode::Accelerated, RenderingPurpose::Canvas,
                                             1, DestinationColorSpace::SRGB(), &client);
      REQUIRE(accelerated);
      CHECK(client.BuffersMade == 1);
      CHECK(accelerated->GetRenderingMode() == RenderingMode::Accelerated);
      CHECK(accelerated->Purpose() == RenderingPurpose::Canvas);

      // Preparing it for display flushes it first.
      auto &backend = static_cast<TestAcceleratedBackend &>(*accelerated->Backend());
      accelerated->PrepareForDisplay();
      CHECK(backend.Flushes == 1);
      CHECK(backend.Preparations == 1);
      CHECK(accelerated->FlushDrawingContextAsync());
      CHECK(backend.Flushes == 2);

      // The host has no unaccelerated backend: the page's display list is used.
      auto unaccelerated = ImageBuffer::Create({10, 20}, RenderingMode::Unaccelerated, RenderingPurpose::DOM,
                                               1, DestinationColorSpace::SRGB(), &client);
      REQUIRE(unaccelerated);
      CHECK(client.BuffersMade == 1);
      CHECK(unaccelerated->GetRenderingMode() == RenderingMode::DisplayList);
    }
  }
}

#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/ImageOrientation.hpp"

namespace krys::boo::tests
{
  TEST_CASE("GradientColorStops", "[Graphics][Gradient]")
  {
    GradientColorStops stops;
    CHECK(stops.IsEmpty());

    stops.AddColorStop({0.5f, Color::white});
    stops.AddColorStop({0.25f, Color::black});
    stops.AddColorStop({1, Color::white});
    REQUIRE(stops.size() == 3);

    // Stops are sorted on demand, keeping the order of equal offsets.
    const auto &sorted = stops.GetSorted();
    CHECK(sorted.Stops()[0].Offset == 0.25f);
    CHECK(sorted.Stops()[1].Offset == 0.5f);
    CHECK(sorted.Stops()[2].Offset == 1);

    auto mapped = sorted.MapColors([](const Color &) { return Color {Color::black}; });
    CHECK(mapped.Stops()[1].Color == Color::black);
    CHECK(mapped.Stops()[1].Offset == 0.5f);
  }

  TEST_CASE("Gradient", "[Graphics][Gradient]")
  {
    auto method =
      ColourInterpolationMethod {ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied};

    auto linear = Gradient::Create(Gradient::LinearData {{0, 0}, {0, 100}}, method);
    CHECK_FALSE(linear->IsZeroSize());
    CHECK(linear->SpreadMethod() == GradientSpreadMethod::Pad);
    CHECK(linear->IsTransient());
    linear->AddColorStop({0, Color::black});
    CHECK(linear->Stops().size() == 1);

    CHECK(Gradient::Create(Gradient::LinearData {{10, 10}, {10, 10}}, method)->IsZeroSize());
    CHECK(Gradient::Create(Gradient::RadialData {{5, 5}, {5, 5}, 2, 2, 1}, method)->IsZeroSize());
    CHECK_FALSE(Gradient::Create(Gradient::RadialData {{5, 5}, {5, 5}, 0, 2, 1}, method)->IsZeroSize());
    CHECK_FALSE(Gradient::Create(Gradient::ConicData {{5, 5}, 0}, method)->IsZeroSize());

    SECTION("tiled drawing")
    {
      // A vertical gradient only needs a one pixel wide tile.
      FloatSize size {100, 100};
      FloatRect srcRect {10, 10, 50, 50};
      linear->AdjustParametersForTiledDrawing(size, srcRect, {});
      CHECK(size == FloatSize {1, 100});
      CHECK(srcRect == FloatRect {0, 10, 1, 50});

      // ... unless the tiles are spaced.
      FloatSize spacedSize {100, 100};
      FloatRect spacedRect {10, 10, 50, 50};
      linear->AdjustParametersForTiledDrawing(spacedSize, spacedRect, {2, 2});
      CHECK(spacedSize == FloatSize {100, 100});

      auto horizontal = Gradient::Create(Gradient::LinearData {{0, 0}, {100, 0}}, method);
      FloatSize horizontalSize {100, 100};
      FloatRect horizontalRect {10, 10, 50, 50};
      horizontal->AdjustParametersForTiledDrawing(horizontalSize, horizontalRect, {});
      CHECK(horizontalSize == FloatSize {100, 1});
      CHECK(horizontalRect == FloatRect {10, 0, 50, 1});
    }
  }

  TEST_CASE("ImageOrientation", "[Graphics][ImageOrientation]")
  {
    using Orientation = ImageOrientation::Orientation;

    CHECK(ImageOrientation {} == Orientation::None);
    CHECK(ImageOrientation::FromEXIFValue(6) == Orientation::OriginRightTop);
    CHECK(ImageOrientation::FromEXIFValue(0) == Orientation::None);
    CHECK(ImageOrientation::FromEXIFValue(9) == Orientation::None);

    CHECK(ImageOrientation {Orientation::OriginRightTop}.UsesWidthAsHeight());
    CHECK_FALSE(ImageOrientation {Orientation::OriginBottomRight}.UsesWidthAsHeight());

    CHECK(ImageOrientation {Orientation::OriginTopLeft}.WithFlippedY() == Orientation::OriginBottomLeft);
    CHECK(ImageOrientation {Orientation::OriginLeftBottom}.WithFlippedY() == Orientation::OriginLeftTop);

    // 90 degrees clockwise: x' = -y + w, y' = x.
    auto transform = ImageOrientation {Orientation::OriginRightTop}.TransformFromDefault({20, 10});
    CHECK(transform.A() == 0);
    CHECK(transform.B() == 1);
    CHECK(transform.C() == -1);
    CHECK(transform.D() == 0);
    CHECK(transform.E() == 20);
    CHECK(transform.F() == 0);
    CHECK(ImageOrientation {Orientation::OriginTopLeft}.TransformFromDefault({20, 10}).IsIdentity());
  }
}

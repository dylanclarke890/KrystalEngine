#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Image/ImageOrientation.hpp"

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
      SizeF size {100, 100};
      RectF srcRect {10, 10, 50, 50};
      linear->AdjustParametersForTiledDrawing(size, srcRect, {});
      CHECK(size == SizeF {1, 100});
      CHECK(srcRect == RectF {0, 10, 1, 50});

      // ... unless the tiles are spaced.
      SizeF spacedSize {100, 100};
      RectF spacedRect {10, 10, 50, 50};
      linear->AdjustParametersForTiledDrawing(spacedSize, spacedRect, {2, 2});
      CHECK(spacedSize == SizeF {100, 100});

      auto horizontal = Gradient::Create(Gradient::LinearData {{0, 0}, {100, 0}}, method);
      SizeF horizontalSize {100, 100};
      RectF horizontalRect {10, 10, 50, 50};
      horizontal->AdjustParametersForTiledDrawing(horizontalSize, horizontalRect, {});
      CHECK(horizontalSize == SizeF {100, 1});
      CHECK(horizontalRect == RectF {10, 0, 50, 1});
    }
  }

  TEST_CASE("ImageOrientation", "[Graphics][img::ImageOrientation]")
  {
    using Orientation = img::ImageOrientation::Orientation;

    CHECK(img::ImageOrientation {} == Orientation::None);
    CHECK(img::ImageOrientation::FromEXIFValue(6) == Orientation::OriginRightTop);
    CHECK(img::ImageOrientation::FromEXIFValue(0) == Orientation::None);
    CHECK(img::ImageOrientation::FromEXIFValue(9) == Orientation::None);

    CHECK(img::ImageOrientation {Orientation::OriginRightTop}.UsesWidthAsHeight());
    CHECK_FALSE(img::ImageOrientation {Orientation::OriginBottomRight}.UsesWidthAsHeight());

    CHECK(img::ImageOrientation {Orientation::OriginTopLeft}.WithFlippedY() == Orientation::OriginBottomLeft);
    CHECK(img::ImageOrientation {Orientation::OriginLeftBottom}.WithFlippedY() == Orientation::OriginLeftTop);

    // 90 degrees clockwise: x' = -y + w, y' = x.
    auto transform = img::ImageOrientation {Orientation::OriginRightTop}.TransformFromDefault({20, 10});
    CHECK(transform.A() == 0);
    CHECK(transform.B() == 1);
    CHECK(transform.C() == -1);
    CHECK(transform.D() == 0);
    CHECK(transform.E() == 20);
    CHECK(transform.F() == 0);
    CHECK(img::ImageOrientation {Orientation::OriginTopLeft}.TransformFromDefault({20, 10}).IsIdentity());
  }
}

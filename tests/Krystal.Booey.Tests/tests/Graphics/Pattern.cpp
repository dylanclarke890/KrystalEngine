#include "Krystal.Booey/Graphics/Pattern.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/Images/NativeImage.hpp"
#include "Krystal.Booey/Graphics/SourceBrush.hpp"
#include "Krystal.Core/Strings/TextStream.hpp"

namespace krys::boo::tests
{
  TEST_CASE("Pattern: tiles and brushes", "[Graphics][Pattern]")
  {
    auto buffer = ImageBuffer::Create({10, 20}, 2);
    REQUIRE(buffer);
    Ref<ImageBuffer> tile = buffer;

    // A pattern's tile image is the buffer it was made with, its size in the buffer's pixels.
    auto pattern = Pattern::Create(SourceImage {Ref<ImageBuffer>(tile)},
                                   {true, false, AffineTransform::CreateTranslation({1, 2})});
    CHECK(pattern->TileImageBuffer() == tile.get());
    CHECK(pattern->TileNativeImage() == nullptr);
    CHECK(pattern->TileImage().Size() == SizeI(20, 40));
    CHECK(pattern->RepeatX());
    CHECK_FALSE(pattern->RepeatY());
    pattern->SetPatternSpaceTransform(AffineTransform::CreateScale({3, 3}));
    CHECK(pattern->PatternSpaceTransform() == AffineTransform::CreateScale({3, 3}));

    // A brush paints with a pattern in place of a gradient; brushes with patterns are equal only with the
    // same pattern.
    SourceBrush brush;
    brush.SetPattern(Ref<Pattern>(pattern));
    CHECK(brush.GetPattern() == pattern.get());
    CHECK(brush.GetGradient() == nullptr);
    CHECK(brush.HasPatternOrGradient());
    CHECK_FALSE(brush.PackedColor());

    SourceBrush same;
    same.SetPattern(Ref<Pattern>(pattern));
    CHECK(brush == same);
    SourceBrush other;
    other.SetPattern(Pattern::Create(SourceImage {Ref<ImageBuffer>(tile)}));
    CHECK_FALSE(brush == other);

    TextStream ts;
    ts << brush;
    auto dump = ts.Release();
    CAPTURE(css::tests::ToNarrow(dump));
    CHECK(dump.contains(u8"(tile-size width=20 height=40)"));
    CHECK(dump.contains(u8"(pattern-space-transform {m=((3.00,0.00)(0.00,3.00)) t=(0.00,0.00)})"));

    // The graphics context's fill and stroke patterns are its brushes'.
    auto &context = tile->Context();
    context.SetStrokePattern(Ref<Pattern>(pattern));
    CHECK(context.StrokePattern() == pattern.get());
    CHECK(context.FillPattern() == nullptr);
  }
}

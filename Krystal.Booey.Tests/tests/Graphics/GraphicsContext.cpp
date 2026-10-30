#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/FloatSegment.hpp"
#include "Krystal.Booey/Graphics/NullGraphicsContext.hpp"
#include "Krystal.Core/Colour/ColourTextStream.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A recorder that exposes how text lines are split into rects.
    class LinesRecorder : public display_list::RecorderImpl
    {
    public:
      LinesRecorder() noexcept : RecorderImpl(FloatSize {100, 100})
      {
      }

      using GraphicsContext::ComputeRectsAndStrokeColorForLinesForText;
    };

    template <typename T>
    KRYS_NODISCARD utf8_string ToText(const T &value)
    {
      TextStream ts;
      ts << value;
      return ts.Release();
    }
  }

  TEST_CASE("GraphicsContextState: records what changes", "[Graphics][GraphicsContext]")
  {
    using Change = GraphicsContextState::Change;
    GraphicsContextState state;
    CHECK(state.Changes().IsEmpty());

    SECTION("Setting a property to its value isn't a change")
    {
      state.SetFillColor(Color::black);
      state.SetAlpha(1);
      CHECK(state.Changes().IsEmpty());
      state.SetAlpha(0.5f);
      state.SetStrokeThickness(2);
      CHECK(state.Changes() == GraphicsContextState::ChangeFlags {Change::Alpha, Change::StrokeThickness});
      state.DidApplyChanges();
      CHECK(state.Changes().IsEmpty());
    }

    SECTION("Merging all changes copies every property that differs")
    {
      GraphicsContextState other;
      other.SetTextDrawingMode(TextDrawingMode::Stroke);
      other.SetImageInterpolationQuality(InterpolationQuality::High);
      other.SetShadowsIgnoreTransforms(true);
      state.MergeAllChanges(other);
      CHECK(state.GetTextDrawingMode() == TextDrawingModeFlags {TextDrawingMode::Stroke});
      CHECK(state.ImageInterpolationQuality() == InterpolationQuality::High);
      CHECK(state.ShadowsIgnoreTransforms());
      CHECK(state.Changes()
            == GraphicsContextState::ChangeFlags {Change::TextDrawingMode, Change::ImageInterpolationQuality,
                                                  Change::ShadowsIgnoreTransforms});
    }

    SECTION("Merging the last changes copies only what changed")
    {
      GraphicsContextState other;
      other.SetAlpha(0.25f);
      state.SetStrokeThickness(4);
      state.DidApplyChanges();
      state.MergeLastChanges(other);
      CHECK(state.Alpha() == 0.25f);
      CHECK(state.StrokeThickness() == 4);
      CHECK(state.Changes() == GraphicsContextState::ChangeFlags {Change::Alpha});
    }

    SECTION("A change back to the last drawing state is no change")
    {
      GraphicsContextState lastDrawn;
      GraphicsContextState other;
      state.SetAlpha(0.5f);
      other.SetAlpha(0.25f);
      other.SetAlpha(1);
      state.MergeLastChanges(other, lastDrawn);
      CHECK(state.Alpha() == 1);
      CHECK(state.Changes().IsEmpty());
    }

    SECTION("Repurposing as initial forgets the changes")
    {
      state.SetAlpha(0.5f);
      auto clone = state.Clone(GraphicsContextState::Purpose::SaveRestore);
      CHECK(clone.GetPurpose() == GraphicsContextState::Purpose::SaveRestore);
      CHECK(!clone.Changes().IsEmpty());
      clone.Repurpose(GraphicsContextState::Purpose::Initial);
      CHECK(clone.Changes().IsEmpty());
      CHECK(clone.Alpha() == 0.5f);
    }
  }

  TEST_CASE("GraphicsContext: splits text lines into rects", "[Graphics][GraphicsContext]")
  {
    LinesRecorder context;
    const FloatSegment segment[] {{0, 10}};

    SECTION("A solid line is a rect, its top rounded down to the next device pixel")
    {
      auto result = context.ComputeRectsAndStrokeColorForLinesForText({0, 10.3f}, 1, segment, false, false,
                                                                      StrokeStyle::SolidStroke);
      REQUIRE(result.Rects.size() == 1u);
      CHECK(result.Rects[0] == FloatRect(0, 11, 10, 1));
      CHECK(result.StrokeColor == Color::black);
    }

    SECTION("A dashed line has dashes twice its thickness")
    {
      auto result = context.ComputeRectsAndStrokeColorForLinesForText({0, 10}, 1, segment, false, false,
                                                                      StrokeStyle::DashedStroke);
      REQUIRE(result.Rects.size() == 3u);
      CHECK(result.Rects[0] == FloatRect(0, 10, 2, 1));
      CHECK(result.Rects[1] == FloatRect(4, 10, 2, 1));
      CHECK(result.Rects[2] == FloatRect(8, 10, 2, 1));
    }

    SECTION("A double line has a second line a line's height below")
    {
      auto result = context.ComputeRectsAndStrokeColorForLinesForText({0, 10}, 2, segment, false, true,
                                                                      StrokeStyle::SolidStroke);
      REQUIRE(result.Rects.size() == 2u);
      CHECK(result.Rects[1] == FloatRect(0, 14, 10, 2));
    }

    SECTION("Scaled down, the line fades")
    {
      context.Scale(0.5f);
      auto result = context.ComputeRectsAndStrokeColorForLinesForText({0, 10}, 1, segment, false, false,
                                                                      StrokeStyle::SolidStroke);
      CHECK(result.StrokeColor.AlphaAsFloat() == Catch::Approx(0.5f).margin(0.01f));
    }
  }

  TEST_CASE("GraphicsContext: state savers", "[Graphics][GraphicsContext]")
  {
    display_list::RecorderImpl recorder({100, 100});
    GraphicsContext &context = recorder;
    {
      GraphicsContextStateSaver saver(context);
      context.SetAlpha(0.5f);
      CHECK(context.StackSize() == 1u);
      {
        TransparencyLayerScope layer(context, 0.5f);
        CHECK(context.IsInTransparencyLayer());
      }
      CHECK(!context.IsInTransparencyLayer());
      {
        InterpolationQualityMaintainer quality(context, InterpolationQuality::High);
        CHECK(context.ImageInterpolationQuality() == InterpolationQuality::High);
      }
      CHECK(context.ImageInterpolationQuality() == InterpolationQuality::Default);
    }
    CHECK(context.StackSize() == 0u);
    CHECK(context.Alpha() == 1);
  }

  TEST_CASE("NullGraphicsContext: draws nothing", "[Graphics][GraphicsContext]")
  {
    NullGraphicsContext nullContext;
    GraphicsContext &context = nullContext;
    CHECK(context.PaintingDisabled());
    CHECK(!context.PerformingPaintInvalidation());
    context.Translate(10, 10);
    context.FillRect({0, 0, 10, 10}, Color::red);
    CHECK(context.GetCTM().IsIdentity());
    CHECK(context.ClipBounds().IsEmpty());

    NullGraphicsContext detecting(NullGraphicsContext::PaintInvalidationReasons::DetectingContentfulPaint);
    CHECK(static_cast<GraphicsContext &>(detecting).DetectingContentfulPaint());
    CHECK(static_cast<GraphicsContext &>(detecting).PerformingPaintInvalidation());
  }

  TEST_CASE("FloatSegment: subtracts dilated segments", "[Graphics][GraphicsContext]")
  {
    auto result = DifferenceWithDilation({0, 100}, {{50, 60}, {10, 20}}, 2);
    CHECK(result == List<FloatSegment> {{0, 8}, {22, 48}, {62, 100}});

    // Gaps no longer than the dilation are dropped.
    result = DifferenceWithDilation({0, 100}, {{3, 50}}, 2);
    CHECK(result == List<FloatSegment> {{52, 100}});
  }

  TEST_CASE("Graphics types: write themselves", "[Graphics][GraphicsContext]")
  {
    CHECK(ToText(AffineTransform {}) == u8"identity");
    CHECK(ToText(CompositeOperator::DestinationOut) == u8"destination-out");
    CHECK(ToText(BlendMode::ColorDodge) == u8"color-dodge");
    CHECK(ToText(LineCap::Square) == u8"SQUARE");
    CHECK(ToText(StrokeStyle::DashedStroke) == u8"dashed-stroke");
    CHECK(ToText(Color(Color::red)) == u8"#FF0000");
    CHECK(ToText(GraphicsContextState::Change::DropShadow) == u8"drop-shadow");
  }
}

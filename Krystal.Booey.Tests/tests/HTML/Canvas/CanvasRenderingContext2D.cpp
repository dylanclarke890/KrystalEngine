#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2D.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Graphics/TestPathStrokeStrategy.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey/CSS/DOMMatrix.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayList.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/ImageBufferDisplayListBackend.hpp"
#include "Krystal.Booey/Graphics/SourceImage.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasGradient.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasPattern.hpp"
#include "Krystal.Booey/HTML/Canvas/Path2D.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include <numbers>

namespace krys::boo::tests
{
  namespace
  {
    using html::CanvasRenderingContext2D;

    struct Canvas2DFixture : PageInFrameFixture
    {
      using PageInFrameFixture::PageInFrameFixture;

      KRYS_NODISCARD html::HTMLCanvasElement &Canvas(dom::DOMStringView id = u8"canvas") noexcept
      {
        return Downcast<html::HTMLCanvasElement>(ById(id));
      }

      KRYS_NODISCARD CanvasRenderingContext2D &Context(dom::DOMStringView id = u8"canvas") noexcept
      {
        auto context = Canvas(id).GetContext2d();
        REQUIRE(context);
        return *context;
      }

      /// @brief The names of what's been drawn into @p id's buffer.
      KRYS_NODISCARD List<std::string_view> Drawn(dom::DOMStringView id = u8"canvas") noexcept
      {
        auto *buffer = Canvas(id).Buffer();
        REQUIRE(buffer);
        auto *backend = static_cast<ImageBufferDisplayListBackend *>(buffer->Backend());
        List<std::string_view> names;
        for (const auto &item : backend->CopyDisplayList()->Items())
        {
          names.push_back(std::visit([]<typename TItem>(const TItem &) { return TItem::Name; }, item));
        }
        return names;
      }
    };

    KRYS_NODISCARD size_t Count(const List<std::string_view> &names, std::string_view name) noexcept
    {
      return static_cast<size_t>(std::ranges::count(names, name));
    }

    KRYS_NODISCARD dom::DOMString
      StyleString(const html::CanvasRenderingContext2DBase::StyleVariant &style) noexcept
    {
      const auto *string = std::get_if<dom::DOMString>(&style);
      return string ? *string : dom::DOMString(u8"(not a colour)");
    }
  }

  TEST_CASE("CanvasRenderingContext2D: made by the canvas", "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    Canvas2DFixture fixture(u8"<canvas id=canvas></canvas><canvas id=other></canvas>");
    auto &canvas = fixture.Canvas();

    auto context = canvas.GetContext(u8"2d");
    REQUIRE(context);
    CHECK(context->Is2d());
    CHECK_FALSE(context->IsGPUBased());
    CHECK(Is<CanvasRenderingContext2D>(*context));
    CHECK(canvas.GetContext2d().get() == context.get());
    CHECK(&Downcast<CanvasRenderingContext2D>(*context).Canvas() == &canvas);
    CHECK(canvas.GetContext(u8"krystal-texture") == nullptr);

    // The settings it was made with are its attributes.
    auto &other = fixture.Canvas(u8"other");
    auto withSettings =
      other.GetContext2d({.WillReadFrequently = true, .ColorSpace = html::PredefinedColorSpace::DisplayP3});
    REQUIRE(withSettings);
    CHECK(withSettings->GetContextAttributes().WillReadFrequently);
    CHECK(withSettings->WillReadFrequently());
    CHECK(withSettings->GetContextAttributes().ColorSpace == html::PredefinedColorSpace::DisplayP3);
    // Settings given later are ignored.
    CHECK(other.GetContext2d({})->GetContextAttributes().WillReadFrequently);

    // Until something is drawn, there's no buffer and the canvas is transparent black.
    CHECK_FALSE(canvas.HasCreatedImageBuffer());
    CHECK(
      context->IsSurfaceBufferTransparentBlack(html::CanvasRenderingContext::SurfaceBuffer::DisplayBuffer));
  }

  TEST_CASE("CanvasRenderingContext2D: the state's defaults and setters",
            "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    Canvas2DFixture fixture(u8"<canvas id=canvas></canvas>");
    auto &context = fixture.Context();

    CHECK(context.LineWidth() == 1);
    CHECK(context.LineCap() == html::CanvasLineCap::Butt);
    CHECK(context.LineJoin() == html::CanvasLineJoin::Miter);
    CHECK(context.MiterLimit() == 10);
    CHECK(context.GetLineDash().empty());
    CHECK(context.LineDashOffset() == 0);
    CHECK(context.ShadowOffsetX() == 0);
    CHECK(context.ShadowBlur() == 0);
    CHECK(context.ShadowColor() == u8"rgba(0, 0, 0, 0)");
    CHECK(context.GlobalAlpha() == 1);
    CHECK(context.GlobalCompositeOperation() == u8"source-over");
    CHECK(context.ImageSmoothingEnabled());
    CHECK(context.GetImageSmoothingQuality() == html::ImageSmoothingQuality::Low);
    CHECK(context.TextAlign() == html::CanvasTextAlign::Start);
    CHECK(context.TextBaseline() == html::CanvasTextBaseline::Alphabetic);
    CHECK(context.GetDirection() == html::CanvasDirection::Ltr);
    CHECK(StyleString(context.FillStyle()) == u8"#000000");
    CHECK(StyleString(context.StrokeStyle()) == u8"#000000");

    SECTION("invalid values are ignored")
    {
      context.SetLineWidth(0);
      context.SetLineWidth(-1);
      context.SetLineWidth(std::numeric_limits<double>::infinity());
      CHECK(context.LineWidth() == 1);
      context.SetMiterLimit(std::nan(""));
      CHECK(context.MiterLimit() == 10);
      context.SetShadowBlur(-2);
      CHECK(context.ShadowBlur() == 0);
      context.SetGlobalAlpha(1.5);
      CHECK(context.GlobalAlpha() == 1);
      context.SetLineDash({1, -1});
      CHECK(context.GetLineDash().empty());
      context.SetShadowColor(u8"not a colour");
      CHECK(context.ShadowColor() == u8"rgba(0, 0, 0, 0)");
      context.SetFillStyle(dom::DOMString(u8"nonsense"));
      CHECK(StyleString(context.FillStyle()) == u8"#000000");
    }

    SECTION("valid values are kept")
    {
      context.SetLineWidth(2.5);
      CHECK(context.LineWidth() == 2.5);
      context.SetLineCap(html::CanvasLineCap::Round);
      CHECK(context.LineCap() == html::CanvasLineCap::Round);
      context.SetLineJoin(html::CanvasLineJoin::Bevel);
      CHECK(context.LineJoin() == html::CanvasLineJoin::Bevel);

      // An odd dash list is repeated.
      context.SetLineDash({1, 2, 3});
      CHECK(context.GetLineDash() == List<double> {1, 2, 3, 1, 2, 3});
      context.SetLineDashOffset(4);
      CHECK(context.LineDashOffset() == 4);

      context.SetShadowColor(u8"rgb(255 0 0 / 50%)");
      CHECK(context.ShadowColor() == u8"rgba(255, 0, 0, 0.5)");
      context.SetFillStyle(dom::DOMString(u8"blue"));
      CHECK(StyleString(context.FillStyle()) == u8"#0000ff");
      context.SetStrokeStyle(dom::DOMString(u8"rgba(0, 128, 0, 0.25)"));
      CHECK(StyleString(context.StrokeStyle()) == u8"rgba(0, 128, 0, 0.25)");

      context.SetImageSmoothingEnabled(false);
      CHECK_FALSE(context.ImageSmoothingEnabled());
      context.SetImageSmoothingQuality(html::ImageSmoothingQuality::High);
      CHECK(context.GetImageSmoothingQuality() == html::ImageSmoothingQuality::High);
      context.SetTextAlign(html::CanvasTextAlign::Center);
      CHECK(context.TextAlign() == html::CanvasTextAlign::Center);
      context.SetTextBaseline(html::CanvasTextBaseline::Top);
      CHECK(context.TextBaseline() == html::CanvasTextBaseline::Top);
      context.SetDirection(html::CanvasDirection::Rtl);
      CHECK(context.GetDirection() == html::CanvasDirection::Rtl);
    }

    SECTION("compositing names")
    {
      context.SetGlobalCompositeOperation(u8"destination-in");
      CHECK(context.GlobalCompositeOperation() == u8"destination-in");
      context.SetGlobalCompositeOperation(u8"lighter");
      CHECK(context.GlobalCompositeOperation() == u8"lighter");
      context.SetGlobalCompositeOperation(u8"multiply");
      CHECK(context.GlobalCompositeOperation() == u8"multiply");
      // `difference` is the blend mode.
      context.SetGlobalCompositeOperation(u8"difference");
      CHECK(context.GlobalCompositeOperation() == u8"difference");
      CHECK(context.StateStack().back().GlobalBlend == BlendMode::Difference);
      CHECK(context.StateStack().back().GlobalComposite == CompositeOperator::SourceOver);
      // Names that aren't standard are ignored.
      context.SetGlobalCompositeOperation(u8"darker");
      context.SetGlobalCompositeOperation(u8"Copy");
      CHECK(context.GlobalCompositeOperation() == u8"difference");
    }
  }

  TEST_CASE("CanvasRenderingContext2D: colours for the canvas", "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    Canvas2DFixture fixture(u8"<canvas id=canvas style='color: rgb(0, 0, 255)'></canvas>"
                            u8"<canvas id=plain style='color: lime'></canvas>");

    // `currentcolor` is the canvas's inline `color`.
    auto &context = fixture.Context();
    context.SetFillStyle(dom::DOMString(u8"currentcolor"));
    CHECK(StyleString(context.FillStyle()) == u8"#0000ff");

    // System colours are allowed.
    context.SetStrokeStyle(dom::DOMString(u8"red"));
    context.SetStrokeStyle(dom::DOMString(u8"Canvas"));
    CHECK(StyleString(context.StrokeStyle()) != u8"#ff0000");
  }

  TEST_CASE("CanvasRenderingContext2D: saving and restoring", "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    Canvas2DFixture fixture(u8"<canvas id=canvas></canvas>");
    auto &context = fixture.Context();

    context.Save();
    context.SetLineWidth(5);
    context.Translate(10, 0);
    context.Save();
    context.SetFillStyle(dom::DOMString(u8"red"));
    CHECK(context.StateStack().size() == 3u);

    context.Restore();
    CHECK(StyleString(context.FillStyle()) == u8"#000000");
    CHECK(context.LineWidth() == 5);
    context.Restore();
    CHECK(context.LineWidth() == 1);
    CHECK(context.GetTransform()->IsIdentity());
    // Restoring past the first state does nothing.
    context.Restore();
    CHECK(context.StateStack().size() == 1u);

    // Saves that change nothing are only counted.
    context.Save();
    context.Restore();
    CHECK(context.StateStack().size() == 1u);

    SECTION("the path keeps its place across restores")
    {
      context.Save();
      context.Translate(100, 0);
      context.Rect(0, 0, 10, 10);
      context.Restore();
      CHECK(context.IsPointInPath(105, 5));
      CHECK_FALSE(context.IsPointInPath(5, 5));
    }

    SECTION("resetting returns to the defaults")
    {
      context.SetLineWidth(3);
      context.Save();
      context.Rect(0, 0, 10, 10);
      context.Reset();
      CHECK(context.LineWidth() == 1);
      CHECK(context.StateStack().size() == 1u);
      CHECK_FALSE(context.IsPointInPath(5, 5));
    }

    SECTION("resizing the canvas resets it")
    {
      context.SetLineWidth(3);
      fixture.Canvas().Width(100);
      CHECK(context.LineWidth() == 1);
      CHECK(fixture.Canvas().Size() == IntSize(100, 150));
    }
  }

  TEST_CASE("CanvasRenderingContext2D: transforms", "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    Canvas2DFixture fixture(u8"<canvas id=canvas></canvas>");
    auto &context = fixture.Context();

    context.Translate(10, 20);
    context.Scale(2, 3);
    auto matrix = context.GetTransform();
    CHECK(matrix->GetIs2D());
    CHECK(matrix->A() == 2);
    CHECK(matrix->D() == 3);
    CHECK(matrix->E() == 10);
    CHECK(matrix->F() == 20);

    context.Rotate(std::numbers::pi / 2);
    CHECK(context.GetTransform()->B() == Catch::Approx(3));

    context.SetTransform(1, 0, 0, 1, 5, 5);
    CHECK(context.GetTransform()->E() == 5);
    REQUIRE_FALSE(context.SetTransform({.M11 = 2, .M41 = 7}).HasException());
    CHECK(context.GetTransform()->A() == 2);
    CHECK(context.GetTransform()->E() == 7);
    CHECK(context.SetTransform({.E = 1, .M41 = 2}).GetException().Code() == dom::ExceptionCode::TypeError);

    context.ResetTransform();
    CHECK(context.GetTransform()->IsIdentity());

    // A transform that can't be inverted stops drawing and paths until it's reset.
    context.Scale(0, 1);
    context.Rect(0, 0, 10, 10);
    CHECK_FALSE(context.IsPointInPath(5, 5));
    context.ResetTransform();
    context.Rect(0, 0, 10, 10);
    CHECK(context.IsPointInPath(5, 5));
  }

  TEST_CASE("CanvasRenderingContext2D: hit testing paths", "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    Canvas2DFixture fixture(u8"<canvas id=canvas></canvas>");
    auto &context = fixture.Context();

    // Two nested squares: even-odd leaves a hole that non-zero fills.
    context.Rect(0, 0, 100, 100);
    context.Rect(25, 25, 50, 50);
    CHECK(context.IsPointInPath(50, 50));
    CHECK_FALSE(context.IsPointInPath(50, 50, html::CanvasFillRule::Evenodd));
    CHECK(context.IsPointInPath(10, 10, html::CanvasFillRule::Evenodd));
    CHECK_FALSE(context.IsPointInPath(std::nan(""), 0));

    // The point is in the transform's space.
    context.BeginPath();
    context.Translate(200, 0);
    context.Rect(0, 0, 10, 10);
    CHECK(context.IsPointInPath(205, 5));
    CHECK_FALSE(context.IsPointInPath(5, 5));

    SECTION("strokes follow the line width")
    {
      PathStrokeStrategyScope stroker;
      context.ResetTransform();
      auto line = html::Path2D::Create(u8"M 0 50 L 100 50");
      CHECK(context.IsPointInStroke(*line, 50, 50));
      CHECK_FALSE(context.IsPointInStroke(*line, 50, 53));
      context.SetLineWidth(10);
      CHECK(context.IsPointInStroke(*line, 50, 53));
      CHECK(context.IsPointInPath(*html::Path2D::Create(u8"M 0 0 h 10 v 10 h -10 Z"), 5, 5));
    }
  }

  TEST_CASE("CanvasRenderingContext2D: gradients and patterns", "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    Canvas2DFixture fixture(u8"<canvas id=canvas></canvas>");
    auto &context = fixture.Context();

    CHECK(context.CreateLinearGradient(0, 0, std::nanf(""), 0).GetException().Code()
          == dom::ExceptionCode::NotSupportedError);
    CHECK(context.CreateRadialGradient(0, 0, -1, 0, 0, 1).GetException().Code()
          == dom::ExceptionCode::IndexSizeError);
    CHECK(context.CreateConicGradient(std::numeric_limits<float>::infinity(), 0, 0).GetException().Code()
          == dom::ExceptionCode::NotSupportedError);

    auto gradient = context.CreateLinearGradient(0, 0, 100, 0).ReleaseValue();
    CHECK(gradient->AddColorStop(1.5, u8"red").GetException().Code() == dom::ExceptionCode::IndexSizeError);
    CHECK(gradient->AddColorStop(0.5, u8"nonsense").GetException().Code() == dom::ExceptionCode::SyntaxError);
    CHECK_FALSE(gradient->AddColorStop(0, u8"red").HasException());
    CHECK_FALSE(gradient->AddColorStop(1, u8"blue").HasException());

    context.SetFillStyle(Ref<html::CanvasGradient>(gradient));
    auto fill = context.FillStyle();
    REQUIRE(std::holds_alternative<RefPtr<html::CanvasGradient>>(fill));
    CHECK(std::get<RefPtr<html::CanvasGradient>>(fill) == gradient.get());

    // A colour after a gradient is a colour again, even the one it was before.
    context.SetFillStyle(dom::DOMString(u8"black"));
    CHECK(StyleString(context.FillStyle()) == u8"#000000");

    // A conic gradient's angle starts from the x-axis.
    auto conic = context.CreateConicGradient(0, 50, 50).ReleaseValue();
    CHECK(std::get<Gradient::ConicData>(conic->GetGradient().GetData()).AngleRadians
          == Catch::Approx(std::numbers::pi / 2));

    SECTION("patterns")
    {
      bool repeatX = false;
      bool repeatY = false;
      CHECK(html::CanvasPattern::ParseRepetitionType(u8"", repeatX, repeatY));
      CHECK((repeatX && repeatY));
      CHECK(html::CanvasPattern::ParseRepetitionType(u8"repeat-y", repeatX, repeatY));
      CHECK((!repeatX && repeatY));
      CHECK(html::CanvasPattern::ParseRepetitionType(u8"no-repeat", repeatX, repeatY));
      CHECK((!repeatX && !repeatY));
      CHECK_FALSE(html::CanvasPattern::ParseRepetitionType(u8"Repeat", repeatX, repeatY));

      Ref<ImageBuffer> tile = ImageBuffer::Create({4, 4}, 1);
      auto pattern = html::CanvasPattern::Create(SourceImage(krys::move(tile)), true, false);
      REQUIRE_FALSE(pattern->SetTransform({.M11 = 2}).HasException());
      CHECK(pattern->GetPattern().PatternSpaceTransform().A() == 2);
      CHECK(pattern->SetTransform({.B = 1, .M12 = 0}).HasException());

      context.SetStrokeStyle(Ref<html::CanvasPattern>(pattern));
      CHECK(std::holds_alternative<RefPtr<html::CanvasPattern>>(context.StrokeStyle()));
    }
  }

  TEST_CASE("CanvasRenderingContext2D: drawing records into the canvas's buffer",
            "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    Canvas2DFixture fixture(u8"<canvas id=canvas width=100 height=50></canvas>");
    auto &context = fixture.Context();

    context.FillRect(10, 10, 20, 20);
    context.StrokeRect(0, 0, 5, 5);
    CHECK(fixture.Canvas().HasCreatedImageBuffer());
    CHECK_FALSE(
      context.IsSurfaceBufferTransparentBlack(html::CanvasRenderingContext::SurfaceBuffer::DisplayBuffer));
    auto drawn = fixture.Drawn();
    CHECK(Count(drawn, "fill-rect") == 1u);
    CHECK(Count(drawn, "stroke-rect") == 1u);

    SECTION("empty and non-finite rectangles draw nothing")
    {
      context.FillRect(0, 0, 0, 0);
      context.FillRect(std::nan(""), 0, 10, 10);
      CHECK(Count(fixture.Drawn(), "fill-rect") == 1u);
    }

    SECTION("clearing draws without shadows, alpha or compositing")
    {
      context.SetShadowColor(u8"red");
      context.SetShadowBlur(4);
      context.SetGlobalAlpha(0.5);
      context.ClearRect(0, 0, 100, 50);
      CHECK(Count(fixture.Drawn(), "clear-rect") == 1u);
    }

    SECTION("compositing modes that touch the whole canvas use a layer, and copy clears first")
    {
      context.SetGlobalCompositeOperation(u8"source-in");
      context.FillRect(0, 0, 10, 10);
      CHECK(Count(fixture.Drawn(), "begin-transparency-layer-with-composite-mode") == 1u);

      context.SetGlobalCompositeOperation(u8"copy");
      context.Rect(0, 0, 10, 10);
      context.Fill();
      auto names = fixture.Drawn();
      CHECK(Count(names, "clear-rect") == 1u);
      CHECK(Count(names, "fill-path") == 1u);
    }

    SECTION("clips and paths")
    {
      context.Rect(0, 0, 50, 50);
      context.Clip();
      context.BeginPath();
      REQUIRE_FALSE(context.Arc(25, 25, 10, 0, 2 * std::numbers::pi_v<float>).HasException());
      context.Fill();
      context.Stroke();
      auto names = fixture.Drawn();
      CHECK(Count(names, "clip-path") == 1u);
      CHECK(Count(names, "fill-path") == 1u);
      CHECK(Count(names, "stroke-path") == 1u);

      // An empty path draws nothing.
      context.BeginPath();
      context.Fill();
      CHECK(Count(fixture.Drawn(), "fill-path") == 1u);
    }

    SECTION("a zero-size gradient paints nothing")
    {
      auto gradient = context.CreateLinearGradient(5, 5, 5, 5).ReleaseValue();
      context.SetFillStyle(krys::move(gradient));
      context.FillRect(0, 0, 10, 10);
      CHECK(Count(fixture.Drawn(), "fill-rect") == 1u);
    }

    SECTION("the canvas paints its buffer")
    {
      auto painted = fixture.Painted();
      CHECK(painted.find(u8"draw-image-buffer") != utf8_string::npos);
    }
  }
}

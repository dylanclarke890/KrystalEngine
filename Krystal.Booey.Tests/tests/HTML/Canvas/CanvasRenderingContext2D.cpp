#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2D.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Graphics/TestAcceleratedImageBuffers.hpp"
#include "Krystal.Booey.Tests/Graphics/TestPathStrokeStrategy.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/TestImages.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/DOMMatrix.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayList.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/ImageBufferDisplayListBackend.hpp"
#include "Krystal.Booey/Graphics/SourceImage.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasGradient.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasPattern.hpp"
#include "Krystal.Booey/HTML/Canvas/Path2D.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/ImageData.hpp"
#include "Krystal.Booey/HTML/TextMetrics.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/SVG/SVGImageElement.hpp"
#include <cmath>
#include <limits>
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

  TEST_CASE("CanvasRenderingContext2D: prepared for display", "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    SECTION("without a host, the canvas records into a display list")
    {
      Canvas2DFixture fixture(u8"<canvas id=canvas width=20 height=10></canvas>");
      auto &context = fixture.Context();
      context.FillRect(0, 0, 5, 5);
      auto *buffer = fixture.Canvas().Buffer();
      REQUIRE(buffer);
      CHECK(buffer->GetRenderingMode() == RenderingMode::DisplayList);
      CHECK(buffer->Purpose() == RenderingPurpose::Canvas);
      CHECK_FALSE(context.IsAccelerated());
      CHECK_FALSE(context.DelegatesDisplay());
      CHECK_FALSE(context.NeedsPreparationForDisplay());

      // What's drawn waits for the rendering update to flush it.
      CHECK(context.HasDeferredOperations());
      CHECK(context.IsInPreparationForDisplayOrFlush());
      fixture.Document->PrepareCanvasesForDisplayOrFlushIfNeeded();
      CHECK_FALSE(context.HasDeferredOperations());
      CHECK_FALSE(context.IsInPreparationForDisplayOrFlush());
    }

    SECTION("the host's accelerated buffer is prepared in the rendering update")
    {
      style::tests::ResolverFixture fixture(u8"<!DOCTYPE html><canvas id=canvas width=20 height=10></canvas>",
                                            style::tests::WithView::No);
      auto page = Page::Create(CreateUnique<AcceleratingChromeClient>());
      auto &client = static_cast<AcceleratingChromeClient &>(page->GetChromeClient());
      fixture.Frame = ShareRefPtr(&page->MainFrame());
      fixture.Frame->CreateView({200, 100});
      fixture.Frame->SetDocument(ShareRefPtr<dom::Document>(fixture.Document.get()));
      page->UpdateRendering();

      auto &canvas = Downcast<html::HTMLCanvasElement>(*fixture.Document->GetElementById(u8"canvas"));
      auto context = canvas.GetContext2d();
      REQUIRE(context);
      context->FillRect(0, 0, 5, 5);
      auto *buffer = canvas.Buffer();
      REQUIRE(buffer);
      CHECK(client.BuffersMade == 1);
      CHECK(buffer->GetRenderingMode() == RenderingMode::Accelerated);
      CHECK(context->IsAccelerated());
      CHECK(context->DelegatesDisplay());
      CHECK(context->NeedsPreparationForDisplay());
      CHECK(page->IsRenderingUpdateScheduled());

      auto &backend = static_cast<TestAcceleratedBackend &>(*buffer->Backend());
      page->UpdateRendering();
      CHECK(backend.Preparations == 1);
      CHECK(backend.Flushes >= 1);
      CHECK_FALSE(context->HasDeferredOperations());
      CHECK_FALSE(context->IsInPreparationForDisplayOrFlush());

      // Drawing again prepares it again; an update with nothing drawn doesn't.
      context->FillRect(5, 5, 5, 5);
      page->UpdateRendering();
      CHECK(backend.Preparations == 2);
      page->UpdateRendering();
      CHECK(backend.Preparations == 2);
    }
  }

  TEST_CASE("CanvasRenderingContext2D: image data", "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    Canvas2DFixture fixture(u8"<canvas id=canvas width=4 height=4></canvas>");
    auto &context = fixture.Context();

    SECTION("made at a size")
    {
      auto made = context.CreateImageData(-3, 2, {});
      REQUIRE_FALSE(made.HasException());
      CHECK(made.Value()->Size() == IntSize(3, 2));
      CHECK(made.Value()->ColorSpace() == html::PredefinedColorSpace::SRGB);
      CHECK(context.CreateImageData(0, 2, {}).GetException().Code() == dom::ExceptionCode::IndexSizeError);

      auto like = context.CreateImageData(*made.Value());
      CHECK(like.Value()->Size() == IntSize(3, 2));
    }

    SECTION("read back: without a host, what's drawn reads as transparent black")
    {
      context.SetFillStyle(dom::DOMString(u8"red"));
      context.FillRect(0, 0, 4, 4);
      auto read = context.GetImageData(1, 1, 2, 2, {});
      REQUIRE_FALSE(read.HasException());
      CHECK(read.Value()->Size() == IntSize(2, 2));
      CHECK(read.Value()->Data().AsUint8ClampedArray()->Item(3) == 0);

      // Negative sizes read the other way; zero ones are errors.
      auto flipped = context.GetImageData(4, 4, -3, -1, {});
      CHECK(flipped.Value()->Size() == IntSize(3, 1));
      CHECK(context.GetImageData(0, 0, 0, 1, {}).GetException().Code() == dom::ExceptionCode::IndexSizeError);
    }

    SECTION("put: drawn as an image, replacing what's there, whatever the state")
    {
      context.SetGlobalAlpha(0.5);
      context.Translate(1, 1);
      context.SetShadowColor(u8"black");
      context.SetShadowBlur(3);
      auto data = context.CreateImageData(2, 2, {});
      context.PutImageData(*data.Value(), 1, 1);
      auto drawn = fixture.Drawn();
      CHECK(Count(drawn, "draw-native-image") == 1u);
      CHECK(Count(drawn, "set-ctm") == 1u);

      // Outside the canvas, or with an empty dirty rectangle, nothing's drawn.
      context.PutImageData(*data.Value(), 10, 10);
      context.PutImageData(*data.Value(), 0, 0, 0, 0, 0, 0);
      CHECK(Count(fixture.Drawn(), "draw-native-image") == 1u);
    }

    SECTION("a put over the whole canvas reads back without reading the buffer, until it's drawn into")
    {
      auto data = context.CreateImageData(4, 4, {});
      auto bytes = data.Value()->Data().AsUint8ClampedArray();
      bytes->Set(0, 200);
      bytes->Set(1, 100);
      bytes->Set(2, 50);
      bytes->Set(3, 128);
      context.PutImageData(*data.Value(), 0, 0);

      // As if premultiplied and back.
      auto read = context.GetImageData(0, 0, 4, 4, {});
      auto readBytes = read.Value()->Data().AsUint8ClampedArray();
      CHECK(readBytes->Item(0) == 201);
      CHECK(readBytes->Item(1) == 101);
      CHECK(readBytes->Item(2) == 51);
      CHECK(readBytes->Item(3) == 128);

      // The kept pixels are read once.
      CHECK(context.GetImageData(0, 0, 4, 4, {}).Value()->Data().AsUint8ClampedArray()->Item(3) == 0);

      context.PutImageData(*data.Value(), 0, 0);
      context.FillRect(0, 0, 1, 1);
      CHECK(context.GetImageData(0, 0, 4, 4, {}).Value()->Data().AsUint8ClampedArray()->Item(3) == 0);
    }
  }

  TEST_CASE("CanvasRenderingContext2D: drawing images", "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    Canvas2DFixture fixture(
      u8"<canvas id=canvas width=10 height=10></canvas><canvas id=other width=4 height=4>"
      u8"</canvas><canvas id=empty width=0 height=4></canvas><img id=img src='file:///a.png'>"
      u8"<img id=pending>");
    RunNetworkingTasks();
    auto &context = fixture.Context();
    auto &image = Downcast<html::HTMLImageElement>(fixture.ById(u8"img"));
    REQUIRE(image.Complete());

    SECTION("an image")
    {
      REQUIRE_FALSE(context.DrawImage(ShareRefPtr(&image), 1, 1).HasException());
      REQUIRE_FALSE(context.DrawImage(ShareRefPtr(&image), 0, 0, 6, 4).HasException());
      REQUIRE_FALSE(context.DrawImage(ShareRefPtr(&image), 0, 0, 1, 1, 2, 2, 4, 4).HasException());
      CHECK(Count(fixture.Drawn(), "draw-native-image") == 3u);

      // Empty or non-finite rectangles draw nothing; an image with no source isn't drawn either.
      auto &pending = Downcast<html::HTMLImageElement>(fixture.ById(u8"pending"));
      REQUIRE_FALSE(context.DrawImage(ShareRefPtr(&image), 0, 0, 0, 4).HasException());
      REQUIRE_FALSE(
        context.DrawImage(ShareRefPtr(&image), 0, 0, 1, 1, 0, 0, std::nanf(""), 4).HasException());
      REQUIRE_FALSE(context.DrawImage(ShareRefPtr(&pending), 0, 0).HasException());
      CHECK(Count(fixture.Drawn(), "draw-native-image") == 3u);
    }

    SECTION("compositing modes that touch the whole canvas draw through a buffer, and copy clears first")
    {
      context.SetGlobalCompositeOperation(u8"source-in");
      REQUIRE_FALSE(context.DrawImage(ShareRefPtr(&image), 1, 1).HasException());
      auto drawn = fixture.Drawn();
      CHECK(Count(drawn, "draw-image-buffer") == 1u);
      CHECK(Count(drawn, "clear-rect") == 1u);

      context.SetGlobalCompositeOperation(u8"copy");
      REQUIRE_FALSE(context.DrawImage(ShareRefPtr(&image), 1, 1).HasException());
      CHECK(Count(fixture.Drawn(), "clear-rect") == 2u);
    }

    SECTION("a canvas")
    {
      auto &other = fixture.Canvas(u8"other");
      std::ignore = fixture.Context(u8"other");
      REQUIRE_FALSE(context.DrawImage(ShareRefPtr(&other), 2, 2).HasException());
      CHECK(Count(fixture.Drawn(), "draw-image-buffer") == 1u);

      // A canvas with no pixels can't be drawn.
      auto &empty = fixture.Canvas(u8"empty");
      CHECK(context.DrawImage(ShareRefPtr(&empty), 0, 0).GetException().Code()
            == dom::ExceptionCode::InvalidStateError);
    }

    SECTION("patterns")
    {
      auto pattern = context.CreatePattern(ShareRefPtr(&image), u8"repeat-x");
      REQUIRE_FALSE(pattern.HasException());
      REQUIRE(pattern.Value());
      CHECK(pattern.Value()->GetPattern().RepeatX());
      CHECK_FALSE(pattern.Value()->GetPattern().RepeatY());

      CHECK(context.CreatePattern(ShareRefPtr(&image), u8"diagonal").GetException().Code()
            == dom::ExceptionCode::SyntaxError);
      // An image not yet loaded makes no pattern, without an error.
      auto &pending = Downcast<html::HTMLImageElement>(fixture.ById(u8"pending"));
      auto none = context.CreatePattern(ShareRefPtr(&pending), u8"repeat");
      REQUIRE_FALSE(none.HasException());
      CHECK_FALSE(none.Value());

      // A canvas is copied; without a host its pixels can't be, nor can an empty canvas's.
      auto &empty = fixture.Canvas(u8"empty");
      CHECK(context.CreatePattern(ShareRefPtr(&empty), u8"repeat").GetException().Code()
            == dom::ExceptionCode::InvalidStateError);
      std::ignore = fixture.Context(u8"other");
      CHECK(context.CreatePattern(ShareRefPtr(&fixture.Canvas(u8"other")), u8"repeat").GetException().Code()
            == dom::ExceptionCode::InvalidStateError);
    }
  }

  namespace
  {
    /// @brief The anchors of the glyph runs drawn into @p canvas's buffer, in order.
    KRYS_NODISCARD List<FloatPoint> GlyphAnchors(html::HTMLCanvasElement &canvas) noexcept
    {
      auto *buffer = canvas.Buffer();
      REQUIRE(buffer);
      auto *backend = static_cast<ImageBufferDisplayListBackend *>(buffer->Backend());
      List<FloatPoint> anchors;
      for (const auto &item : backend->CopyDisplayList()->Items())
      {
        if (const auto *glyphs = std::get_if<display_list::DrawGlyphs>(&item))
        {
          anchors.push_back(glyphs->LocalAnchor());
        }
      }
      return anchors;
    }

    /// @brief Whether a state recorded into @p canvas's buffer draws text with @p mode.
    KRYS_NODISCARD bool RecordedTextDrawingMode(html::HTMLCanvasElement &canvas,
                                                TextDrawingMode mode) noexcept
    {
      auto *buffer = canvas.Buffer();
      REQUIRE(buffer);
      auto *backend = static_cast<ImageBufferDisplayListBackend *>(buffer->Backend());
      for (const auto &item : backend->CopyDisplayList()->Items())
      {
        if (const auto *state = std::get_if<display_list::SetState>(&item))
        {
          if (state->State().GetTextDrawingMode() == TextDrawingModeFlags {mode})
          {
            return true;
          }
        }
      }
      return false;
    }
  }

  TEST_CASE("CanvasRenderingContext2D: text drawing styles", "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    layout::tests::RegisterAhem();
    Canvas2DFixture fixture(u8"<canvas id=canvas width=200 height=100></canvas>");
    auto &context = fixture.Context();

    SECTION("defaults")
    {
      CHECK(context.Font() == u8"10px sans-serif");
      CHECK(context.LetterSpacing() == u8"0px");
      CHECK(context.WordSpacing() == u8"0px");
      CHECK(context.FontKerning() == html::CanvasFontKerning::Auto);
      CHECK(context.FontStretch() == html::CanvasFontStretch::Normal);
      CHECK(context.FontVariantCaps() == html::CanvasFontVariantCaps::Normal);
      CHECK(context.TextRendering() == html::CanvasTextRendering::Auto);
      CHECK(context.Lang() == u8"inherit");
    }

    SECTION("the font is serialised without its line height or default values")
    {
      context.SetFont(u8"italic 400 12px/2 Unknown Font, sans-serif");
      CHECK(context.Font() == u8"italic 12px \"Unknown Font\", sans-serif");
      context.SetFont(u8"bold 20px Ahem");
      CHECK(context.Font() == u8"bold 20px Ahem");
      context.SetFont(u8"300 20px Ahem");
      CHECK(context.Font() == u8"300 20px Ahem");
    }

    SECTION("values that aren't fonts are ignored")
    {
      context.SetFont(u8"20px Ahem");
      context.SetFont(u8"bogus");
      context.SetFont(u8"inherit");
      context.SetFont(u8"initial");
      context.SetFont(u8"");
      CHECK(context.Font() == u8"20px Ahem");
    }

    SECTION("relative sizes are relative to the canvas's font")
    {
      // The canvas's font is 16px, the medium size.
      context.SetFont(u8"150% Ahem");
      CHECK(context.Font() == u8"24px Ahem");
      context.SetFont(u8"2em Ahem");
      CHECK(context.Font() == u8"32px Ahem");
      context.SetFont(u8"larger Ahem");
      CHECK(context.Font() == u8"19.2px Ahem");
      context.SetFont(u8"smaller Ahem");
      CHECK(context.Font() == u8"13.333333px Ahem");
    }

    SECTION("the font's width and caps are its stretch and caps")
    {
      context.SetFont(u8"small-caps condensed 20px Ahem");
      CHECK(context.FontStretch() == html::CanvasFontStretch::Condensed);
      CHECK(context.FontVariantCaps() == html::CanvasFontVariantCaps::SmallCaps);
      CHECK(context.Font() == u8"small-caps 20px Ahem");

      context.SetFontVariantCaps(html::CanvasFontVariantCaps::Normal);
      CHECK(context.Font() == u8"20px Ahem");

      // A new font resets them.
      context.SetFontStretch(html::CanvasFontStretch::Expanded);
      context.SetFont(u8"20px Ahem");
      CHECK(context.FontStretch() == html::CanvasFontStretch::Normal);
    }

    SECTION("styles set before the font is are kept when it's realised")
    {
      context.SetFontStretch(html::CanvasFontStretch::UltraExpanded);
      context.SetFontKerning(html::CanvasFontKerning::None);
      context.SetTextRendering(html::CanvasTextRendering::GeometricPrecision);
      context.SetLang(u8"fr");
      std::ignore = context.MeasureText(u8"a");
      CHECK(context.FontStretch() == html::CanvasFontStretch::UltraExpanded);
      CHECK(context.FontKerning() == html::CanvasFontKerning::None);
      CHECK(context.TextRendering() == html::CanvasTextRendering::GeometricPrecision);
      CHECK(context.Lang() == u8"fr");
    }

    SECTION("saving and restoring")
    {
      context.SetFont(u8"20px Ahem");
      context.Save();
      context.SetFont(u8"30px Ahem");
      context.SetLetterSpacing(u8"2px");
      context.SetFontKerning(html::CanvasFontKerning::Normal);
      context.Restore();
      CHECK(context.Font() == u8"20px Ahem");
      CHECK(context.LetterSpacing() == u8"0px");
      CHECK(context.FontKerning() == html::CanvasFontKerning::Auto);
      CHECK(context.MeasureText(u8"ab")->Width() == 40);
    }

    SECTION("letter and word spacing are lengths")
    {
      context.SetFont(u8"16px Ahem");
      context.SetLetterSpacing(u8"  2px ");
      CHECK(context.LetterSpacing() == u8"2px");
      CHECK(context.MeasureText(u8"ab")->Width() == 36);

      context.SetLetterSpacing(u8"0.5em");
      CHECK(context.LetterSpacing() == u8"0.5em");
      CHECK(context.MeasureText(u8"ab")->Width() == 48);

      // Values that aren't lengths, or use units that aren't allowed, are ignored.
      context.SetLetterSpacing(u8"10vw");
      context.SetLetterSpacing(u8"2lh");
      context.SetLetterSpacing(u8"bogus");
      context.SetLetterSpacing(u8"2px 3px");
      CHECK(context.LetterSpacing() == u8"0.5em");

      context.SetLetterSpacing(u8"0px");
      context.SetWordSpacing(u8"4px");
      CHECK(context.WordSpacing() == u8"4px");
      CHECK(context.MeasureText(u8"a b")->Width() == 52);

      // Spacing in font units is resolved again with a new font.
      context.SetWordSpacing(u8"1em");
      context.SetFont(u8"32px Ahem");
      CHECK(context.MeasureText(u8"a b")->Width() == 128);
    }
  }

  TEST_CASE("CanvasRenderingContext2D: measuring text", "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    layout::tests::RegisterAhem();
    Canvas2DFixture fixture(u8"<canvas id=canvas width=200 height=100></canvas>");
    auto &context = fixture.Context();
    context.SetFont(u8"20px Ahem");

    // Ahem's glyphs are 1em squares, 0.8em above the baseline.
    auto metrics = context.MeasureText(u8"abc");
    CHECK(metrics->Width() == 60);
    CHECK(metrics->AlphabeticBaseline() == 0);
    CHECK(metrics->EmHeightAscent() == Catch::Approx(16));
    CHECK(metrics->EmHeightDescent() == Catch::Approx(4));
    CHECK(metrics->FontBoundingBoxAscent() == 16);
    CHECK(metrics->FontBoundingBoxDescent() == 4);
    CHECK(metrics->IdeographicBaseline() == Catch::Approx(-4));
    CHECK(metrics->ActualBoundingBoxRight() >= 60);

    SECTION("the space characters are spaces")
    {
      CHECK(context.MeasureText(u8"a\tb\nc")->Width() == context.MeasureText(u8"a b c")->Width());
    }

    SECTION("the baseline and alignment move the box")
    {
      context.SetTextBaseline(html::CanvasTextBaseline::Top);
      context.SetTextAlign(html::CanvasTextAlign::Center);
      metrics = context.MeasureText(u8"abc");
      CHECK(metrics->AlphabeticBaseline() == -16);
      CHECK(metrics->FontBoundingBoxAscent() == 0);
      CHECK(metrics->ActualBoundingBoxLeft() == Catch::Approx(30).margin(1));
      CHECK(metrics->ActualBoundingBoxRight() == Catch::Approx(30).margin(1));

      // The end of right-to-left text is its left.
      context.SetTextAlign(html::CanvasTextAlign::End);
      context.SetDirection(html::CanvasDirection::Rtl);
      metrics = context.MeasureText(u8"abc");
      CHECK(metrics->ActualBoundingBoxLeft() == Catch::Approx(0).margin(1));
    }
  }

  TEST_CASE("CanvasRenderingContext2D: drawing text", "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    layout::tests::RegisterAhem();
    Canvas2DFixture fixture(u8"<canvas id=canvas width=200 height=100></canvas>");
    auto &context = fixture.Context();
    auto &canvas = fixture.Canvas();
    context.SetFont(u8"10px Ahem");

    SECTION("filling")
    {
      context.FillText(u8"abc", 10, 20);
      auto anchors = GlyphAnchors(canvas);
      REQUIRE(anchors.size() == 1);
      CHECK(anchors[0] == FloatPoint(10, 20));
      CHECK(RecordedTextDrawingMode(canvas, TextDrawingMode::Fill));
    }

    SECTION("stroking")
    {
      context.StrokeText(u8"abc", 10, 20);
      REQUIRE(GlyphAnchors(canvas).size() == 1);
      CHECK(RecordedTextDrawingMode(canvas, TextDrawingMode::Stroke));
    }

    SECTION("alignment")
    {
      context.SetTextAlign(html::CanvasTextAlign::Right);
      context.FillText(u8"abc", 100, 20);
      auto anchors = GlyphAnchors(canvas);
      REQUIRE(anchors.size() == 1);
      CHECK(anchors[0] == FloatPoint(70, 20));
    }

    SECTION("a maximum width narrower than the text scales it")
    {
      context.FillText(u8"abcd", 10, 20, 20.0);
      auto drawn = fixture.Drawn();
      CHECK(Count(drawn, "scale") == 1u);
      CHECK(Count(drawn, "draw-glyphs") == 1u);

      // A wider one doesn't.
      context.FillText(u8"abcd", 10, 20, 100.0);
      CHECK(Count(fixture.Drawn(), "scale") == 1u);
    }

    SECTION("runs of each direction are drawn one after the other")
    {
      context.FillText(u8"abאבcd", 10, 20);
      auto anchors = GlyphAnchors(canvas);
      REQUIRE(anchors.size() >= 3);
      CHECK(anchors.front() == FloatPoint(10, 20));
      // Each run starts where the run before it ended.
      CHECK(anchors[1].X() == Catch::Approx(30));
      CHECK(anchors.back().X() > anchors[1].X());
    }

    SECTION("nothing is drawn at non-finite positions or with a maximum width that isn't positive")
    {
      context.FillText(u8"abc", std::nan(""), 20);
      context.FillText(u8"abc", 10, std::numeric_limits<double>::infinity());
      context.FillText(u8"abc", 10, 20, 0.0);
      context.FillText(u8"abc", 10, 20, -5.0);
      CHECK_FALSE((canvas.HasCreatedImageBuffer() && Count(fixture.Drawn(), "draw-glyphs") != 0u));
    }
  }

  namespace
  {
    /// @brief The names of what's been drawn into @p canvas's buffer.
    KRYS_NODISCARD List<std::string_view> DrawnInto(html::HTMLCanvasElement &canvas) noexcept
    {
      auto *buffer = canvas.Buffer();
      REQUIRE(buffer);
      auto *backend = static_cast<ImageBufferDisplayListBackend *>(buffer->Backend());
      List<std::string_view> names;
      for (const auto &item : backend->CopyDisplayList()->Items())
      {
        names.push_back(std::visit([]<typename TItem>(const TItem &) { return TItem::Name; }, item));
      }
      return names;
    }
  }

  TEST_CASE("CanvasRenderingContext2D: the filter", "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    Canvas2DFixture fixture(u8"<canvas id=canvas width=100 height=50></canvas>");
    auto &context = fixture.Context();
    CHECK(context.FilterString() == u8"none");

    context.SetFilterString(u8"blur(2px) drop-shadow(1px 1px red)");
    CHECK(context.FilterString() == u8"blur(2px) drop-shadow(1px 1px red)");

    SECTION("values that aren't filters, and the empty string, are ignored")
    {
      context.SetFilterString(u8"");
      context.SetFilterString(u8"bogus");
      context.SetFilterString(u8"blur(2px) junk");
      context.SetFilterString(u8"inherit");
      CHECK(context.FilterString() == u8"blur(2px) drop-shadow(1px 1px red)");

      context.SetFilterString(u8"none");
      CHECK(context.FilterString() == u8"none");
    }

    SECTION("saved and restored")
    {
      context.Save();
      context.SetFilterString(u8"grayscale(1)");
      CHECK(context.FilterString() == u8"grayscale(1)");
      context.Restore();
      CHECK(context.FilterString() == u8"blur(2px) drop-shadow(1px 1px red)");
    }

    SECTION("without a page, drawing isn't filtered")
    {
      context.FillRect(10, 10, 20, 20);
      CHECK(Count(fixture.Drawn(), "begin-transparency-layer") == 0u);
      CHECK(Count(fixture.Drawn(), "fill-rect") == 1u);
    }
  }

  TEST_CASE("CanvasRenderingContext2D: drawing through the filter",
            "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    PaintFixture fixture(u8"<!DOCTYPE html><canvas id=canvas width=100 height=50></canvas>", {200, 100},
                         true);
    auto &canvas = Downcast<html::HTMLCanvasElement>(*fixture.Document->GetElementById(u8"canvas"));
    auto context = canvas.GetContext2d();
    REQUIRE(context);

    context->SetFilterString(u8"blur(2px)");
    REQUIRE(context->FilterString() == u8"blur(2px)");

    // Each drawing is drawn into a layer, with one more for the blur's style, and the state is as it was
    // after.
    auto drawAndCheck = [&](auto &&draw)
    {
      const auto depth = context->StateStack().size();
      const auto before = DrawnInto(canvas);
      draw();
      auto drawn = DrawnInto(canvas);
      CHECK(Count(drawn, "begin-transparency-layer") == Count(before, "begin-transparency-layer") + 2u);
      CHECK(Count(drawn, "end-transparency-layer") == Count(before, "end-transparency-layer") + 2u);
      CHECK(context->StateStack().size() == depth);
    };

    SECTION("rectangles")
    {
      drawAndCheck([&] { context->FillRect(10, 10, 20, 20); });
      drawAndCheck([&] { context->StrokeRect(10, 10, 20, 20); });
    }

    SECTION("paths")
    {
      context->Rect(10, 10, 20, 20);
      drawAndCheck([&] { context->Fill(); });
      drawAndCheck([&] { context->Stroke(); });
    }

    SECTION("text")
    {
      layout::tests::RegisterAhem();
      context->SetFont(u8"10px Ahem");
      drawAndCheck([&] { context->FillText(u8"abc", 10, 20); });
    }

    SECTION("no filter draws without a layer")
    {
      context->SetFilterString(u8"none");
      context->FillRect(10, 10, 20, 20);
      CHECK(Count(DrawnInto(canvas), "begin-transparency-layer") == 0u);
    }
  }

  TEST_CASE("CanvasRenderingContext2D: focus rings", "[HTML][Canvas][CanvasRenderingContext2D]")
  {
    Canvas2DFixture fixture(u8"<canvas id=canvas width=100 height=50><button id=button>Go</button>"
                            u8"<button id=other>No</button></canvas><button id=outside>Out</button>");
    auto &context = fixture.Context();
    auto &button = Downcast<html::HTMLElement>(fixture.ById(u8"button"));
    context.Rect(10, 10, 30, 20);

    SECTION("only for the focused element")
    {
      context.DrawFocusIfNeeded(button);
      CHECK(Count(fixture.Drawn(), "draw-focus-ring-path") == 0u);

      button.Focus();
      REQUIRE(button.Focused());
      context.DrawFocusIfNeeded(button);
      CHECK(Count(fixture.Drawn(), "draw-focus-ring-path") == 1u);
      context.DrawFocusIfNeeded(fixture.ById(u8"other"));
      CHECK(Count(fixture.Drawn(), "draw-focus-ring-path") == 1u);
    }

    SECTION("only for the canvas's fallback content")
    {
      auto &outside = Downcast<html::HTMLElement>(fixture.ById(u8"outside"));
      outside.Focus();
      REQUIRE(outside.Focused());
      context.DrawFocusIfNeeded(outside);
      CHECK(Count(fixture.Drawn(), "draw-focus-ring-path") == 0u);
    }

    SECTION("along a path, and not an empty one")
    {
      button.Focus();
      auto path = html::Path2D::Create();
      context.DrawFocusIfNeeded(*path, button);
      CHECK(Count(fixture.Drawn(), "draw-focus-ring-path") == 0u);
      path->Rect(0, 0, 5, 5);
      context.DrawFocusIfNeeded(*path, button);
      CHECK(Count(fixture.Drawn(), "draw-focus-ring-path") == 1u);
    }
  }
}

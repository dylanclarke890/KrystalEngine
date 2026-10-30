#include "Krystal.Booey/Platform/DragImage.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayList.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItems.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBufferDisplayListBackend.hpp"
#include "Krystal.Booey/Page/FrameSnapshotting.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/URL/URL.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A 400x300 view of @p markup in a page, with no body margin and 10px text on 20px lines.
    struct SnapshotFixture : style::tests::ResolverFixture
    {
      RefPtr<Page> TestPage;

      explicit SnapshotFixture(utf8_stringview markup) noexcept
          : ResolverFixture(u8"<!DOCTYPE html><style>body{margin:0;font:10px/20px sans-serif}</style>"
                              + utf8_string(markup),
                            style::tests::WithView::No),
            TestPage(Page::Create())
      {
        Frame = ShareRefPtr(&TestPage->MainFrame());
        Frame->CreateView({400, 300});
        Frame->SetDocument(ShareRefPtr<dom::Document>(Document.get()));
        (void)Document->UpdateLayout();
      }
    };

    KRYS_NODISCARD Ref<const display_list::DisplayList> DisplayListOf(const DragImageRef &image) noexcept
    {
      return static_cast<ImageBufferDisplayListBackend *>(image->Backend())->CopyDisplayList();
    }

    template <typename TItem>
    KRYS_NODISCARD bool Contains(const DragImageRef &image) noexcept
    {
      auto displayList = DisplayListOf(image);
      for (const auto &item : displayList->Items())
      {
        if (std::holds_alternative<TItem>(item))
        {
          return true;
        }
      }
      return false;
    }

    /// @brief An empty image of @p width by @p height.
    KRYS_NODISCARD DragImageRef ImageOfSize(int32 width, int32 height) noexcept
    {
      return ImageBuffer::Create(FloatSize(static_cast<float>(width), static_cast<float>(height)), 1);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FrameSnapshotting", "[Page][FrameSnapshotting]")
  {
    SnapshotFixture fixture(
      u8"<div id=box style=\"width:100px;height:40px;background:red\">"
      u8"<div id=wide style=\"position:relative;left:150px;width:50px;height:10px\"></div>"
      u8"</div><p id=p>some text</p><div id=hidden style=\"display:none\"></div>");
    auto &frame = *fixture.Frame;

    SECTION("A node, with its descendants' layers, on a transparent background")
    {
      auto snapshot = SnapshotNode(frame, fixture.ById(u8"box"), {});
      REQUIRE(snapshot);
      CHECK(snapshot->LogicalSize() == FloatSize(200, 40));
      CHECK(Contains<display_list::FillCompositedRect>(snapshot));
      // The view's state is put back.
      CHECK(frame.View()->BaseBackgroundColor() == Color::white);
      CHECK(frame.View()->GetPaintBehavior().IsEmpty());
    }

    SECTION("A node that isn't rendered")
    {
      CHECK(!SnapshotNode(frame, fixture.ById(u8"hidden"), {}));
    }

    SECTION("A rect of the frame")
    {
      auto snapshot = SnapshotFrameRect(frame, IntRect(10, 10, 50, 20), {});
      REQUIRE(snapshot);
      CHECK(snapshot->LogicalSize() == FloatSize(50, 20));
      CHECK(!SnapshotFrameRect(frame, IntRect(10, 10, 0, 20), {}));
    }

    SECTION("The selection, only when there's a range")
    {
      CHECK(!SnapshotSelection(frame, {}));

      auto &text = *fixture.ById(u8"p").FirstChild();
      fixture.Document->Selection().MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 0),
                                           dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 4),
                                           Affinity::Downstream);
      auto snapshot = SnapshotSelection(frame, {});
      REQUIRE(snapshot);
      auto bounds = EnclosingIntRect(fixture.Document->Selection().SelectionBounds());
      CHECK(snapshot->LogicalSize() == FloatSize(bounds.Size()));
      CHECK(Contains<display_list::DrawGlyphs>(snapshot));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DragImage: resizing and fading", "[Platform][DragImage]")
  {
    SECTION("Scaling redraws the image at the new size")
    {
      auto image = ImageOfSize(40, 20);
      auto scaled = ScaleDragImage(image, {2, 3});
      REQUIRE(scaled);
      CHECK(DragImageSize(scaled) == IntSize(80, 60));
      CHECK(Contains<display_list::DrawImageBuffer>(scaled));
      // The same size is the same image.
      CHECK(ScaleDragImage(image, {1, 1}) == image);
      CHECK(!ScaleDragImage(nullptr, {2, 2}));
    }

    SECTION("Fading keeps the size and draws the image translucent")
    {
      auto image = ImageOfSize(40, 20);
      auto dissolved = DissolveDragImageToFraction(image, 0.75f);
      REQUIRE(dissolved);
      CHECK(DragImageSize(dissolved) == IntSize(40, 20));
      auto displayList = DisplayListOf(dissolved);
      bool translucent = false;
      for (const auto &item : displayList->Items())
      {
        if (auto *setState = std::get_if<display_list::SetState>(&item))
        {
          translucent |= setState->State().Alpha() == Catch::Approx(0.75f);
        }
      }
      CHECK(translucent);
      CHECK(Contains<display_list::DrawImageBuffer>(dissolved));
    }

    SECTION("Fitting to a maximum size keeps the aspect ratio")
    {
      CHECK(DragImageSize(FitDragImageToMaxSize(ImageOfSize(400, 100), {400, 100}, {200, 200}))
            == IntSize(200, 50));
      CHECK(DragImageSize(FitDragImageToMaxSize(ImageOfSize(100, 100), {100, 100}, {200, 200}))
            == IntSize(100, 100));
      // An image laid out at another size is scaled to it first.
      CHECK(DragImageSize(FitDragImageToMaxSize(ImageOfSize(100, 100), {50, 20}, {200, 200}))
            == IntSize(50, 20));
    }

    SECTION("The device scale factor scales the image")
    {
      CHECK(DragImageSize(PlatformAdjustDragImageForDeviceScaleFactor(ImageOfSize(40, 20), 2))
            == IntSize(80, 40));
    }

    SECTION("No image")
    {
      CHECK(!CreateDragImageFromImage(nullptr, {}));
      CHECK(!CreateDragImageIconForCachedImageFilename(u8"image.png"));
      CHECK(DragImageSize(nullptr) == IntSize());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DragImage: nodes, selections and links",
                   "[Platform][DragImage]")
  {
    SnapshotFixture fixture(u8"<div id=box style=\"width:100px;height:40px;background:red\"></div>"
                            u8"<p id=p>some text</p><a id=link href=\"https://example.com/\">a link</a>");
    auto &frame = *fixture.Frame;

    SECTION("A node")
    {
      auto image = CreateDragImageForNode(frame, fixture.ById(u8"box"));
      REQUIRE(image);
      CHECK(DragImageSize(image) == IntSize(100, 40));
    }

    SECTION("The selection")
    {
      CHECK(!CreateDragImageForSelection(frame).DragImage);
      auto &text = *fixture.ById(u8"p").FirstChild();
      fixture.Document->Selection().MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 0),
                                           dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 4),
                                           Affinity::Downstream);
      CHECK(CreateDragImageForSelection(frame).DragImage);
    }

    SECTION("A link's label: its text over its URL, at most 200px wide")
    {
      url::URL url(u8"https://example.com/");
      auto labelled = CreateDragImageForLink(fixture.ById(u8"link"), url, u8"a link", 1).DragImage;
      auto unlabelled = CreateDragImageForLink(fixture.ById(u8"link"), url, u8"", 1).DragImage;
      REQUIRE(labelled);
      REQUIRE(unlabelled);
      CHECK(Contains<display_list::FillRoundedRect>(labelled));
      CHECK(Contains<display_list::DrawGlyphs>(labelled));
      // Without a label, the URL is the label, alone.
      CHECK(DragImageSize(unlabelled).Height() < DragImageSize(labelled).Height());

      // A label of both directions is drawn in visual order: the Hebrew right to left, lamed first (twice:
      // the label's text is doubled, in two colours).
      auto bidi = CreateDragImageForLink(fixture.ById(u8"link"), url, u8"abשל", 1).DragImage;
      REQUIRE(bidi);
      CHECK(DrawnText(*DisplayListOf(bidi), u8"של") == u8"לש|לש");

      utf8_string longLabel(200, u8'x');
      auto clipped = CreateDragImageForLink(fixture.ById(u8"link"), url, longLabel, 1).DragImage;
      REQUIRE(clipped);
      CHECK(DragImageSize(clipped).Width() <= 200);

      auto size = DragImageSize(labelled);
      CHECK(DragOffsetForLinkDragImage(labelled) == IntPoint(-size.Width() / 2, -2));
      auto anchor = AnchorPointForLinkDragImage(labelled);
      CHECK(anchor.X() == 0.5f);
      CHECK(anchor.Y() == Catch::Approx(static_cast<float>(size.Height() - 2) / size.Height()));
    }
  }
}

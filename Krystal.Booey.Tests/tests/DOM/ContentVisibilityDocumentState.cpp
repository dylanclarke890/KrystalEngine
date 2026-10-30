#include "Krystal.Booey/DOM/ContentVisibilityDocumentState.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/DOM/ContentVisibilityAutoStateChangeEvent.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Page/DOMSelection.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A document parsed in a page's main frame, whose rendering updates observe the elements with
    /// `content-visibility: auto`.
    struct ContentVisibilityFixture : css::tests::ValueFixture
    {
      static constexpr IntSize ViewSize {200, 100};
      Ref<Page> TestPage;
      Ref<dom::HTMLDocument> Document;

      explicit ContentVisibilityFixture(utf8_string &&markup) noexcept
          : TestPage(Page::Create()), Document(CreateRef<dom::HTMLDocument>())
      {
        html::EventLoop::Current().RunUntilIdle();
        auto &frame = TestPage->MainFrame();
        frame.CreateView(ViewSize);
        frame.SetDocument(ShareRefPtr<dom::Document>(Document.get()));
        auto parser = CreateRef<html::HTMLDocumentParser>(*Document);
        parser->InputStream().Append(u8"<!DOCTYPE html>" + krys::move(markup), IsEOF(true));
        (void)parser->PumpTokenizer();
        Document->StyleScope().FlushPendingUpdate();
      }

      KRYS_NODISCARD dom::Element &ById(dom::DOMStringView id) noexcept
      {
        auto element = Document->GetElementById(id);
        krys_assert(element);
        return *element;
      }

      /// @brief Runs a rendering update, then the tasks it queued.
      void UpdateRendering() noexcept
      {
        TestPage->UpdateRendering();
        html::EventLoop::Current().RunUntilIdle();
      }

      /// @brief Whether @p id's renderer was skipped by the last layout.
      KRYS_NODISCARD bool WasSkipped(dom::DOMStringView id) noexcept
      {
        auto *renderer = ById(id).Renderer();
        krys_assert(renderer);
        return renderer->WasSkippedDuringLastLayoutDueToContentVisibility().value_or(false);
      }

      /// @brief Whether the view, scrolled, paints something filled with @p colour.
      KRYS_NODISCARD bool Paints(utf8_stringview colour) noexcept
      {
        (void)Document->UpdateLayout();
        display_list::RecorderImpl recorder(
          FloatSize(static_cast<float>(ViewSize.Width()), static_cast<float>(ViewSize.Height())));
        TestPage->MainFrame().View()->Paint(recorder, IntRect(IntPoint(), ViewSize));
        auto colourText = utf8_string(u8"(color ") + utf8_string(colour) + u8")";
        return recorder.TakeDisplayList()->AsText({}).find(colourText) != utf8_string::npos;
      }
    };

    /// @brief The `contentvisibilityautostatechange` events fired at an element, as their `skipped`.
    struct StateChanges
    {
      List<bool> Skipped;
      dom::EventListenerHandle Handle;

      explicit StateChanges(dom::Element &element) noexcept
          : Handle(element.AddEventListener(
              u8"contentvisibilityautostatechange", [this](dom::Event &event)
              { Skipped.push_back(Downcast<dom::ContentVisibilityAutoStateChangeEvent>(event).Skipped()); }))
      {
      }
    };
  }

  TEST_CASE("ContentVisibilityDocumentState: content-visibility: hidden skips layout and painting",
            "[DOM][ContentVisibility]")
  {
    ContentVisibilityFixture fixture(
      u8"<style>body { margin: 0 } #box { content-visibility: hidden; contain-intrinsic-size: 100px 40px; "
      u8"width: 100px } #child { height: 30px; background: #00FF00 }</style>"
      u8"<div id=box><div id=child></div></div>");
    (void)fixture.Document->UpdateLayout();

    auto &box = fixture.ById(u8"box");
    auto &child = fixture.ById(u8"child");
    CHECK(fixture.WasSkipped(u8"child"));
    CHECK_FALSE(child.Renderer()->NeedsLayout());
    CHECK(Downcast<RenderBox>(*box.Renderer()).Height() == 40_lu);
    CHECK_FALSE(fixture.Paints(u8"#00FF00"));

    // Asking for the child's geometry lays out the skipped subtree, as though it were visible.
    CHECK(child.OffsetHeight() == 30);
    CHECK_FALSE(fixture.WasSkipped(u8"child"));
    CHECK(box.OffsetHeight() == 40);
    CHECK_FALSE(fixture.Paints(u8"#00FF00"));

    // Made visible, the child lays out and paints.
    REQUIRE_FALSE(box.SetAttribute(u8"style", u8"content-visibility: visible").HasException());
    (void)fixture.Document->UpdateLayout();
    CHECK(box.OffsetHeight() == 30);
    CHECK(fixture.Paints(u8"#00FF00"));
  }

  TEST_CASE(
    "ContentVisibilityDocumentState: content-visibility: auto skips the content far from the viewport",
    "[DOM][ContentVisibility]")
  {
    ContentVisibilityFixture fixture(
      u8"<style>body { margin: 0 } section { content-visibility: auto; contain-intrinsic-size: auto 50px } "
      u8"#a > div { height: 200px; background: #00FF00 } #b > div { height: 70px; background: #0000FF } "
      u8"#spacer { height: 300px }</style>"
      u8"<section id=a><div id=ainner></div></section><section id=b><div id=binner></div></section>"
      u8"<div id=spacer></div>");
    auto &a = fixture.ById(u8"a");
    auto &b = fixture.ById(u8"b");
    StateChanges aChanges(a);
    StateChanges bChanges(b);

    // Until the first observation, neither section is relevant.
    CHECK_FALSE(a.IsRelevantToUser());
    CHECK_FALSE(b.IsRelevantToUser());

    // The first finds both sections on screen at their intrinsic sizes.
    fixture.UpdateRendering();
    CHECK(fixture.Document->IsObservingContentVisibilityTargets());
    CHECK(a.IsRelevantToUser());
    CHECK(a.GetContentRelevancy() == EnumSet<dom::ContentRelevancy> {dom::ContentRelevancy::OnScreen});
    CHECK(b.IsRelevantToUser());
    CHECK(aChanges.Skipped == List<bool> {false});
    CHECK(bChanges.Skipped == List<bool> {false});
    CHECK_FALSE(fixture.WasSkipped(u8"ainner"));
    CHECK(a.OffsetHeight() == 200);

    // Laid out, the first pushes the second off screen, and the next update skips it.
    fixture.UpdateRendering();
    CHECK_FALSE(b.IsRelevantToUser());
    CHECK(bChanges.Skipped == List<bool> {false, true});
    CHECK(fixture.WasSkipped(u8"binner"));
    CHECK(fixture.Paints(u8"#00FF00"));
    CHECK_FALSE(fixture.Paints(u8"#0000FF"));

    // Scrolled to, the second becomes relevant and the first is skipped again, keeping its last remembered
    // size.
    fixture.Document->Window()->ScrollTo(0, 210);
    fixture.UpdateRendering();
    fixture.UpdateRendering();
    CHECK(b.IsRelevantToUser());
    CHECK_FALSE(a.IsRelevantToUser());
    CHECK(aChanges.Skipped == List<bool> {false, true});
    CHECK(bChanges.Skipped == List<bool> {false, true, false});
    CHECK_FALSE(fixture.WasSkipped(u8"binner"));
    CHECK(fixture.WasSkipped(u8"ainner"));
    CHECK(Downcast<RenderBox>(*a.Renderer()).Height() == 200_lu);
    CHECK(b.OffsetHeight() == 70);
    CHECK(fixture.Paints(u8"#0000FF"));

    // Without content-visibility: auto, the section is no longer observed.
    REQUIRE_FALSE(b.SetAttribute(u8"style", u8"content-visibility: visible").HasException());
    (void)fixture.Document->UpdateLayout();
    CHECK_FALSE(b.IsRelevantToUser());
  }

  TEST_CASE(
    "ContentVisibilityDocumentState: focus, selection, the top layer and scrolling make content relevant",
    "[DOM][ContentVisibility]")
  {
    ContentVisibilityFixture fixture(
      u8"<style>body { margin: 0 } #spacer { height: 300px } section { content-visibility: auto; "
      u8"contain-intrinsic-size: auto 50px }</style>"
      u8"<div id=spacer></div><section id=far><button id=button>Go</button><p id=text>Text</p>"
      u8"<dialog id=dialog>Dialog</dialog></section>");
    auto &far = fixture.ById(u8"far");
    fixture.UpdateRendering();
    REQUIRE_FALSE(far.IsRelevantToUser());

    SECTION("Focus")
    {
      Downcast<html::HTMLElement>(fixture.ById(u8"button")).Focus({.PreventScroll = true});
      fixture.UpdateRendering();
      CHECK(far.GetContentRelevancy() == EnumSet<dom::ContentRelevancy> {dom::ContentRelevancy::Focused});

      Downcast<html::HTMLElement>(fixture.ById(u8"button")).Blur();
      fixture.UpdateRendering();
      CHECK_FALSE(far.IsRelevantToUser());
    }

    SECTION("Selection")
    {
      REQUIRE_FALSE(
        fixture.Document->GetSelection()->SelectAllChildren(fixture.ById(u8"text")).HasException());
      fixture.UpdateRendering();
      CHECK(far.GetContentRelevancy() == EnumSet<dom::ContentRelevancy> {dom::ContentRelevancy::Selected});

      fixture.Document->GetSelection()->RemoveAllRanges();
      fixture.UpdateRendering();
      CHECK_FALSE(far.IsRelevantToUser());
    }

    SECTION("The top layer")
    {
      auto &dialog = Downcast<html::HTMLDialogElement>(fixture.ById(u8"dialog"));
      REQUIRE_FALSE(dialog.ShowModal().HasException());
      fixture.UpdateRendering();
      CHECK(far.GetContentRelevancy()
              .value_or(EnumSet<dom::ContentRelevancy> {})
              .Contains(dom::ContentRelevancy::IsInTopLayer));

      dialog.Close();
      fixture.UpdateRendering();
      CHECK_FALSE(far.GetContentRelevancy()->Contains(dom::ContentRelevancy::IsInTopLayer));
    }

    SECTION("Scrolling into view")
    {
      fixture.ById(u8"text").ScrollIntoView(true);
      CHECK(far.IsRelevantToUser());
      CHECK(fixture.Document->Window()->ScrollY() > 0);
    }
  }
}

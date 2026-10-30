#include "Krystal.Booey/HTML/LazyLoadImageObserver.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/TestImages.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A document parsed in a page's main frame (the implicit root of intersection observers), so its
    /// images load as they're inserted.
    struct LazyImageFixture : css::tests::ValueFixture
    {
      static constexpr IntSize ViewSize {200, 100};
      Ref<Page> TestPage;
      Ref<dom::HTMLDocument> Document;

      explicit LazyImageFixture(utf8_string &&markup) noexcept
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

      KRYS_NODISCARD LocalFrame &Frame() noexcept
      {
        return TestPage->MainFrame();
      }

      KRYS_NODISCARD dom::Element &ById(dom::DOMStringView id) noexcept
      {
        auto element = Document->GetElementById(id);
        krys_assert(element);
        return *element;
      }

      KRYS_NODISCARD html::HTMLImageElement &Image(dom::DOMStringView id) noexcept
      {
        return Downcast<html::HTMLImageElement>(ById(id));
      }

      void UpdateRendering() noexcept
      {
        TestPage->UpdateRendering();
      }

      /// @brief Whether any image paints.
      KRYS_NODISCARD bool PaintsAnImage() noexcept
      {
        (void)Document->UpdateLayout();
        display_list::RecorderImpl recorder(
          FloatSize(static_cast<float>(ViewSize.Width()), static_cast<float>(ViewSize.Height())));
        Frame().View()->PaintContents(recorder, IntRect(IntPoint(), ViewSize));
        return recorder.TakeDisplayList()->AsText({}).find(u8"draw-native-image") != utf8_string::npos;
      }
    };
  }

  TEST_CASE("LazyLoadImageObserver: loading reflects its known values", "[HTML][LazyLoadImageObserver]")
  {
    LazyImageFixture fixture(u8"<img id=missing><img id=lazy loading=LAZY><img id=invalid loading=soon>");
    CHECK(css::tests::ToNarrow(fixture.Image(u8"missing").Loading()) == "eager");
    CHECK(css::tests::ToNarrow(fixture.Image(u8"lazy").Loading()) == "lazy");
    CHECK(css::tests::ToNarrow(fixture.Image(u8"invalid").Loading()) == "eager");

    fixture.Image(u8"missing").Loading(u8"lazy");
    CHECK(fixture.Image(u8"missing").GetAttribute(u8"loading") == dom::DOMString(u8"lazy"));
  }

  TEST_CASE("LazyLoadImageObserver: a lazy image loads once it intersects the viewport",
            "[HTML][LazyLoadImageObserver]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    loader.Strategy.Resources[u8"file:///b.png"] = images::PNG;

    LazyImageFixture fixture(
      u8"<style>body { margin: 0 } img { display: block; width: 50px; height: 20px }</style>"
      u8"<img id=a loading=lazy src='file:///a.png'><div style='height: 300px'></div>"
      u8"<img id=b loading=lazy src='file:///b.png'>");
    auto &a = fixture.Image(u8"a");
    auto &b = fixture.Image(u8"b");
    auto aEvents = LoadEvents::ListenTo(a);
    auto bEvents = LoadEvents::ListenTo(b);

    // Nothing is fetched until the observer sees the images, and deferred images paint nothing.
    CHECK(loader.Strategy.Requests.empty());
    CHECK(a.IsDeferred());
    CHECK(b.IsDeferred());
    CHECK_FALSE(a.Complete());
    auto &lazyObserver = fixture.Document->GetLazyLoadImageObserver();
    CHECK(lazyObserver.IsObserved(a));
    CHECK(lazyObserver.IsObserved(b));
    CHECK_FALSE(fixture.PaintsAnImage());

    // The first observation finds the first image in view.
    fixture.UpdateRendering();
    CHECK(loader.Strategy.Requests == List<utf8_string> {u8"file:///a.png"});
    RunNetworkingTasks();
    CHECK_FALSE(a.IsDeferred());
    CHECK_FALSE(lazyObserver.IsObserved(a));
    CHECK(a.Complete());
    CHECK(aEvents->Loads == 1);
    CHECK(fixture.PaintsAnImage());
    CHECK(b.IsDeferred());
    CHECK(bEvents->Loads == 0);

    // Scrolled into view, the second loads.
    fixture.Document->Window()->ScrollTo(0, 250);
    fixture.UpdateRendering();
    CHECK(loader.Strategy.Requests == List<utf8_string> {u8"file:///a.png", u8"file:///b.png"});
    RunNetworkingTasks();
    CHECK_FALSE(b.IsDeferred());
    CHECK(b.Complete());
    CHECK(bEvents->Loads == 1);
    CHECK_FALSE(lazyObserver.IsObserved(b));
  }

  TEST_CASE("LazyLoadImageObserver: images in a scroller load within a scrollport of it",
            "[HTML][LazyLoadImageObserver]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///near.png"] = images::PNG;
    loader.Strategy.Resources[u8"file:///far.png"] = images::PNG;

    LazyImageFixture fixture(
      u8"<style>body { margin: 0 } img { display: block; width: 50px; height: 20px }</style>"
      u8"<div id=box style='height: 50px; overflow: auto'>"
      u8"<div style='height: 80px'></div><img id=near loading=lazy src='file:///near.png'>"
      u8"<div style='height: 100px'></div><img id=far loading=lazy src='file:///far.png'>"
      u8"</div>");

    // The scroller's clip grows by its own size: the image 80px down is within it, the one 200px down isn't.
    fixture.UpdateRendering();
    CHECK(loader.Strategy.Requests == List<utf8_string> {u8"file:///near.png"});
    CHECK(fixture.Image(u8"far").IsDeferred());

    fixture.ById(u8"box").ScrollTo(0, 120);
    fixture.UpdateRendering();
    CHECK(loader.Strategy.Requests == List<utf8_string> {u8"file:///near.png", u8"file:///far.png"});
  }

  TEST_CASE("LazyLoadImageObserver: images that load eagerly", "[HTML][LazyLoadImageObserver]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    loader.Strategy.Resources[u8"file:///b.png"] = images::PNG;

    LazyImageFixture fixture(
      u8"<style>body { margin: 0 } img { display: block; width: 50px; height: 20px }</style>"
      u8"<div style='height: 300px'></div><img id=a loading=lazy src='file:///a.png'><img id=b "
      u8"loading=lazy>");
    auto &a = fixture.Image(u8"a");
    auto &b = fixture.Image(u8"b");
    REQUIRE(a.IsDeferred());

    SECTION("Changing loading to eager loads a deferred image")
    {
      REQUIRE_FALSE(a.SetAttribute(u8"loading", u8"eager").HasException());
      CHECK(loader.Strategy.Requests == List<utf8_string> {u8"file:///a.png"});
      // It's observed until the load finishes.
      CHECK(fixture.Document->GetLazyLoadImageObserver().IsObserved(a));
      RunNetworkingTasks();
      CHECK(a.Complete());
      CHECK_FALSE(a.IsDeferred());
      CHECK_FALSE(fixture.Document->GetLazyLoadImageObserver().IsObserved(a));
    }

    SECTION("An image already in the memory cache isn't deferred")
    {
      REQUIRE_FALSE(a.SetAttribute(u8"loading", u8"eager").HasException());
      RunNetworkingTasks();
      REQUIRE(a.Complete());
      REQUIRE_FALSE(b.SetAttribute(u8"src", u8"file:///a.png").HasException());
      CHECK_FALSE(b.IsDeferred());
      RunNetworkingTasks();
      CHECK(b.Complete());
    }

    SECTION("Without scripting, images load eagerly")
    {
      auto environment = fixture.Frame().GetMediaEnvironment();
      environment.CanExecuteScripts = false;
      fixture.Frame().SetMediaEnvironment(environment);
      CHECK_FALSE(b.IsLazyLoadable());
      REQUIRE_FALSE(b.SetAttribute(u8"src", u8"file:///b.png").HasException());
      CHECK_FALSE(b.IsDeferred());
      CHECK(loader.Strategy.Requests == List<utf8_string> {u8"file:///b.png"});
    }

    SECTION("Removing the source stops observing the image")
    {
      a.RemoveAttribute(u8"src");
      CHECK_FALSE(a.IsDeferred());
      CHECK_FALSE(fixture.Document->GetLazyLoadImageObserver().IsObserved(a));
      fixture.UpdateRendering();
      CHECK(loader.Strategy.Requests.empty());
    }
  }
}

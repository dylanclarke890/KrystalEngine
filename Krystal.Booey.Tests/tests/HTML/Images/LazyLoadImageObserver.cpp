#include "Krystal.Booey/HTML/Images/LazyLoadImageObserver.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/Window.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Image.Tests/TestImages.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A document parsed in a page's main frame (the implicit root of intersection observers), so its
    /// images load as they're inserted.
    struct LazyImageFixture : css::tests::ValueFixture
    {
      constexpr static SizeI ViewSize {200, 100};
      Ref<boo::Frame> TestFrame;
      Ref<HTMLDocument> Document;

      explicit LazyImageFixture(utf8_string &&markup) noexcept
          : TestFrame(boo::Frame::Create()), Document(CreateRef<HTMLDocument>())
      {
        html::EventLoop::Current().RunUntilIdle();
        auto &frame = *TestFrame;
        frame.CreateView(ViewSize);
        frame.SetDocument(ShareRefPtr<boo::Document>(Document.get()));
        auto parser = CreateRef<html::DocumentParser>(*Document);
        parser->InputStream().Append(u8"<!DOCTYPE html>" + krys::move(markup), IsEOF::Yes);
        parser->PumpTokenizer();
        Document->StyleScope().FlushPendingUpdate();
      }

      KRYS_NODISCARD boo::Frame &Frame() const noexcept
      {
        return *TestFrame;
      }

      KRYS_NODISCARD Element &ById(DOMStringView id) const noexcept
      {
        auto element = Document->GetElementById(id);
        krys_assert(element);
        return *element;
      }

      KRYS_NODISCARD HTMLImageElement &Image(DOMStringView id) noexcept
      {
        return Downcast<HTMLImageElement>(ById(id));
      }

      void UpdateRendering() noexcept
      {
        TestFrame->UpdateRendering();
      }

      /// @brief Whether any image paints.
      KRYS_NODISCARD bool PaintsAnImage() noexcept
      {
        (void)Document->UpdateLayout();
        display_list::RecorderImpl recorder(
          SizeF(static_cast<float>(ViewSize.Width()), static_cast<float>(ViewSize.Height())));
        Frame().View()->PaintContents(recorder, RectI(PointI(), ViewSize));
        return recorder.TakeDisplayList()->AsText({}).contains(u8"draw-native-image");
      }
    };
  }

  TEST_CASE("LazyLoadImageObserver: loading reflects its known values",
            "[HTML][LazyLoadImageObserver][html:lazy-loading-attribute][html:dom-img-loading]")
  {
    LazyImageFixture fixture(u8"<img id=missing><img id=lazy loading=LAZY><img id=invalid loading=soon>");
    CHECK(css::tests::ToNarrow(fixture.Image(u8"missing").Loading()) == "eager");
    CHECK(css::tests::ToNarrow(fixture.Image(u8"lazy").Loading()) == "lazy");
    CHECK(css::tests::ToNarrow(fixture.Image(u8"invalid").Loading()) == "eager");

    fixture.Image(u8"missing").Loading(u8"lazy");
    CHECK(fixture.Image(u8"missing").GetAttribute(u8"loading") == DOMString(u8"lazy"));
  }

  TEST_CASE("LazyLoadImageObserver: a lazy image loads once it intersects the viewport",
            "[HTML][LazyLoadImageObserver][html:start-intersection-observing-a-lazy-loading-element]"
            "[html:stop-intersection-observing-a-lazy-loading-element][html:lazy-load-intersection-observer]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = img::tests::images::PNG;
    loader.Strategy.Resources[u8"file:///b.png"] = img::tests::images::PNG;

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

  TEST_CASE("LazyLoadImageObserver: a lazy image doesn't delay the load event",
            "[HTML][LazyLoadImageObserver][html:update-the-image-data]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = img::tests::images::PNG;
    loader.Strategy.Resources[u8"file:///b.png"] = img::tests::images::PNG;
    loader.Strategy.Defer = true;

    LazyImageFixture fixture(
      u8"<img id=lazy loading=lazy src='file:///a.png'><img id=eager src='file:///b.png'>");
    fixture.UpdateRendering();
    CHECK(loader.Strategy.Requests == List<utf8_string> {u8"file:///b.png", u8"file:///a.png"});

    // 24. Let delay load event be true if the img's lazy loading attribute is in the Eager state, ...
    // Both are loading, but only the eager one counts.
    CHECK(fixture.Document->GetCachedResourceLoader().RequestCount() == 1);

    loader.Strategy.CompleteDeferred();
    RunNetworkingTasks();
    CHECK(fixture.Document->GetCachedResourceLoader().RequestCount() == 0);
    CHECK(fixture.Image(u8"lazy").Complete());
  }

  TEST_CASE("LazyLoadImageObserver: images in a scroller load within a scrollport of it",
            "[HTML][LazyLoadImageObserver]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///near.png"] = img::tests::images::PNG;
    loader.Strategy.Resources[u8"file:///far.png"] = img::tests::images::PNG;

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

  TEST_CASE("LazyLoadImageObserver: images that load eagerly",
            "[HTML][LazyLoadImageObserver][html:will-lazy-load-element-steps]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = img::tests::images::PNG;
    loader.Strategy.Resources[u8"file:///b.png"] = img::tests::images::PNG;

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

    SECTION("Removing the source stops observing the image")
    {
      a.RemoveAttribute(u8"src");
      CHECK_FALSE(a.IsDeferred());
      CHECK_FALSE(fixture.Document->GetLazyLoadImageObserver().IsObserved(a));
      fixture.UpdateRendering();
      CHECK(loader.Strategy.Requests.empty());
    }
  }

  TEST_CASE("LazyLoadImageObserver: a lazy image moved to another document is observed there",
            "[HTML][LazyLoadImageObserver][html:start-intersection-observing-a-lazy-loading-element]"
            "[html:stop-intersection-observing-a-lazy-loading-element]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = img::tests::images::PNG;

    LazyImageFixture fixture(
      u8"<div style='height: 300px'></div><img id=a loading=lazy src='file:///a.png'>");
    LazyImageFixture other(u8"<p>other</p>");
    auto &a = fixture.Image(u8"a");
    REQUIRE(a.IsDeferred());
    REQUIRE(fixture.Document->GetLazyLoadImageObserver().IsObserved(a));

    // Moved into view in the other document, it's deferred there, and loads at that document's update.
    REQUIRE_FALSE(other.Document->Body()->AppendChild(a).HasException());
    CHECK_FALSE(fixture.Document->GetLazyLoadImageObserver().IsObserved(a));
    CHECK(a.IsDeferred());
    CHECK(other.Document->GetLazyLoadImageObserver().IsObserved(a));
    CHECK(loader.Strategy.Requests.empty());

    fixture.UpdateRendering();
    CHECK(loader.Strategy.Requests.empty());
    other.UpdateRendering();
    CHECK(loader.Strategy.Requests == List<utf8_string> {u8"file:///a.png"});
    RunNetworkingTasks();
    CHECK(a.Complete());
    CHECK_FALSE(other.Document->GetLazyLoadImageObserver().IsObserved(a));
  }
}

#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/TestImages.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Loader/Cache/MemoryCache.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD StyleImage *BackgroundImage(dom::Element &element) noexcept
    {
      auto *renderer = element.Renderer();
      REQUIRE(renderer);
      auto image = renderer->Style().GetBackgroundLayers().UsedValues().begin()->GetImage().TryStyleImage();
      return image.get();
    }

    KRYS_NODISCARD bool Contains(const utf8_string &text, utf8_stringview part) noexcept
    {
      return text.find(part) != utf8_string::npos;
    }
  }

  TEST_CASE("CachedResourceLoader: a url() image loads once its renderer has style, and paints once it has",
            "[Loader][CachedResourceLoader]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///images/a.png"] = images::PNG;
    loader.Strategy.Defer = true;

    PaintFixture fixture(u8"<!DOCTYPE html><div id=box style='width: 30px; height: 20px; background: "
                         u8"url(file:///images/a.png)'>");
    auto painted = fixture.Painted();
    auto &box = *fixture.Document->GetElementById(u8"box");

    REQUIRE(loader.Strategy.Requests == List<utf8_string> {u8"file:///images/a.png"});
    auto *image = BackgroundImage(box);
    REQUIRE(image);
    CHECK_FALSE(image->IsPending());
    REQUIRE(image->GetCachedImage());
    CHECK(image->GetCachedImage()->IsLoading());
    CHECK_FALSE(Contains(painted, u8"native-image"));

    loader.Strategy.CompleteDeferred();
    // The completion is a networking task.
    CHECK(image->GetCachedImage()->IsLoading());
    RunNetworkingTasks();

    CHECK_FALSE(image->GetCachedImage()->IsLoading());
    CHECK(image->GetCachedImage()->GetStatus() == CachedResource::Cached);
    CHECK(image->HasImage());
    CHECK(image->ImageSize(box.Renderer(), 1) == FloatSize(3, 2));
    CHECK(Contains(fixture.Painted(), u8"draw-pattern-native-image"));
  }

  TEST_CASE("CachedResourceLoader: images are shared through the memory cache",
            "[Loader][CachedResourceLoader]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///images/shared.png"] = images::PNG;

    PaintFixture fixture(
      u8"<!DOCTYPE html>"
      u8"<div id=a style='height: 10px; background: url(file:///images/shared.png)'></div>"
      u8"<div id=b style='height: 10px; background: url(file:///images/shared.png)'></div>");
    (void)fixture.Painted();
    RunNetworkingTasks();

    CHECK(loader.Strategy.Requests.size() == 1u);
    auto *a = BackgroundImage(*fixture.Document->GetElementById(u8"a"));
    auto *b = BackgroundImage(*fixture.Document->GetElementById(u8"b"));
    REQUIRE(a);
    REQUIRE(b);
    CHECK(a->GetCachedImage() == b->GetCachedImage());
    CHECK(a->GetCachedImage()->NumberOfClients() == 2u);
    CHECK(
      MemoryCache::Singleton().ResourceForRequest(ResourceRequest(url::URL(u8"file:///images/shared.png")))
      == a->GetCachedImage());
  }

  TEST_CASE("CachedResourceLoader: failed loads and data that isn't an image",
            "[Loader][CachedResourceLoader]")
  {
    LoaderFixture loader;
    const uint8 text[] = "not an image";
    loader.Strategy.Resources[u8"file:///images/text.png"] = Span<const uint8>(text);

    PaintFixture fixture(
      u8"<!DOCTYPE html>"
      u8"<div id=missing style='height: 10px; background: url(file:///images/missing.png)'></div>"
      u8"<div id=text style='height: 10px; background: url(file:///images/text.png)'></div>");
    (void)fixture.Painted();
    RunNetworkingTasks();

    auto *missing = BackgroundImage(*fixture.Document->GetElementById(u8"missing"));
    REQUIRE(missing);
    CHECK(missing->ErrorOccurred());
    REQUIRE(missing->GetCachedImage());
    CHECK(missing->GetCachedImage()->GetStatus() == CachedResource::LoadError);
    CHECK(missing->GetCachedImage()->GetResourceError().ErrorCode() == 404);

    auto *notAnImage = BackgroundImage(*fixture.Document->GetElementById(u8"text"));
    REQUIRE(notAnImage);
    CHECK(notAnImage->ErrorOccurred());
    CHECK(notAnImage->GetCachedImage()->GetStatus() == CachedResource::DecodeError);

    CHECK_FALSE(Contains(fixture.Painted(), u8"native-image"));
  }

  TEST_CASE("CachedResourceLoader: data URLs load without the host", "[Loader][CachedResourceLoader]")
  {
    LoaderFixture loader;

    PaintFixture fixture(
      u8"<!DOCTYPE html><div id=box style='width: 6px; height: 4px; background: url(data:image/png;base64,"
      + utf8_string(images::PNGBase64) + u8")'>");
    (void)fixture.Painted();
    RunNetworkingTasks();

    CHECK(loader.Strategy.Requests.empty());
    auto *image = BackgroundImage(*fixture.Document->GetElementById(u8"box"));
    REQUIRE(image);
    CHECK(image->HasImage());
    CHECK(image->UsesDataProtocol());
    CHECK(Contains(fixture.Painted(), u8"draw-pattern-native-image"));
  }

  TEST_CASE("CachedResourceLoader: without a loader strategy, loads fail", "[Loader][CachedResourceLoader]")
  {
    LoaderFixture loader;
    PlatformStrategies::Singleton().SetLoaderStrategy(nullptr);

    PaintFixture fixture(
      u8"<!DOCTYPE html><div id=box style='height: 10px; background: url(file:///images/nowhere.png)'>");
    (void)fixture.Painted();

    auto *image = BackgroundImage(*fixture.Document->GetElementById(u8"box"));
    REQUIRE(image);
    CHECK_FALSE(image->IsPending());
    // The request fails before it starts, so there's no cached image.
    CHECK_FALSE(image->GetCachedImage());
  }

  TEST_CASE("CachedResourceLoader: a load schedules a rendering update", "[Loader][CachedResourceLoader]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///images/update.png"] = images::PNG;

    auto page = Page::Create();
    auto &frame = page->MainFrame();
    frame.CreateView({200, 100});
    auto document = CreateRef<dom::HTMLDocument>();
    frame.SetDocument(ShareRefPtr<dom::Document>(document.get()));
    auto parser = CreateRef<html::HTMLDocumentParser>(*document);
    parser->InputStream().Append(
      u8"<!DOCTYPE html><div style='height: 10px; background: url(file:///images/update.png)'></div>",
      IsEOF(true));
    parser->PumpTokenizer();
    document->StyleScope().FlushPendingUpdate();
    page->UpdateRendering();
    CHECK_FALSE(page->IsRenderingUpdateScheduled());

    RunNetworkingTasks();
    CHECK(page->IsRenderingUpdateScheduled());
  }

  TEST_CASE("MemoryCache: prunes resources nothing uses when it's over capacity", "[Loader][MemoryCache]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///images/dead.png"] = images::PNG;

    PaintFixture fixture(u8"<!DOCTYPE html>");
    auto &resourceLoader = fixture.Document->GetCachedResourceLoader();
    {
      auto image = resourceLoader.RequestImage(
        CachedResourceRequest(ResourceRequest(url::URL(u8"file:///images/dead.png")),
                              CachedResourceLoader::DefaultCachedResourceOptions()));
      REQUIRE(image);
      RunNetworkingTasks();
      CHECK((*image)->InCache());
    }
    resourceLoader.GarbageCollectDocumentResources();

    auto &memoryCache = MemoryCache::Singleton();
    const ResourceRequest request(url::URL(u8"file:///images/dead.png"));
    REQUIRE(memoryCache.ResourceForRequest(request));
    CHECK(memoryCache.DeadSize() > 0u);

    memoryCache.SetCapacities(0, 0, 0);
    CHECK_FALSE(memoryCache.ResourceForRequest(request));
    CHECK(memoryCache.Size() == 0u);
    memoryCache.SetCapacities(0, 8'192 * 1'024, 8'192 * 1'024);
  }

  TEST_CASE("CachedResourceLoader: a list marker image is laid out at its size once it loads",
            "[Loader][CachedResourceLoader]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///images/marker.png"] = images::PNG;

    PaintFixture fixture(u8"<!DOCTYPE html><ul style='list-style: url(file:///images/marker.png) inside'><li "
                         u8"id=item style='height: "
                         u8"24px'></li></ul>");
    CHECK_FALSE(Contains(fixture.Painted(), u8"native-image"));
    RunNetworkingTasks();

    auto painted = fixture.Painted();
    CHECK(Contains(painted, u8"draw-native-image"));
    CHECK(Contains(painted, u8"(dest-rect at (48,"));
    CHECK(Contains(painted, u8"size 3x2)"));
  }

  TEST_CASE("CachedResourceLoader: the frame's cache policy reloads cached resources once per document",
            "[Loader][CachedResourceLoader]")
  {
    LoaderFixture loader;
    loader.Strategy.AddText(u8"file:///a.css", u8"#box { width: 10px }");
    constexpr utf8_stringview markup = u8"<link rel=stylesheet href='file:///a.css'>"
                                       u8"<style>@import 'file:///a.css';</style><div id=box></div>";

    {
      PageInFrameFixture first {utf8_string(markup)};
      RunNetworkingTasks();
      CHECK(first.RendererSize(u8"box") == IntSize(10, 0));
    }
    // A document uses what's in the memory cache.
    {
      PageInFrameFixture second {utf8_string(markup)};
      RunNetworkingTasks();
      CHECK(loader.Strategy.Requests.size() == 1u);
    }
    // Revalidating loads it again (a file has no validators), once for the document.
    loader.Strategy.AddText(u8"file:///a.css", u8"#box { width: 20px }");
    {
      PageInFrameFixture reloaded {utf8_string(markup), {200, 100}, CachePolicy::Revalidate};
      RunNetworkingTasks();
      CHECK(loader.Strategy.Requests.size() == 2u);
      CHECK(reloaded.RendererSize(u8"box") == IntSize(20, 0));
    }
  }
}

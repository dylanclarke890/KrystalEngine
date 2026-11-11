#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/AnimatedImages.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/EXIFImages.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/TestImages.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/EventListener.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/Images/BitmapImage.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMapElement.hpp"
#include "Krystal.Booey/HTML/Parser/DocumentParser.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Rendering/Replaced/RenderImage.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Core/Threading/ManualWorkQueue.hpp"
#include "Krystal.Core/Utils/ScopeExit.hpp"

namespace krys::boo::tests
{
  namespace
  {
    struct ImageFixture : PageInFrameFixture
    {
      using PageInFrameFixture::PageInFrameFixture;

      KRYS_NODISCARD html::HTMLImageElement &Image(dom::DOMStringView id) noexcept
      {
        return Downcast<html::HTMLImageElement>(ById(id));
      }
    };

    KRYS_NODISCARD size_t Count(const utf8_string &text, utf8_stringview part) noexcept
    {
      size_t count = 0;
      for (auto position = text.find(part); position != utf8_string::npos;
           position = text.find(part, position + 1))
      {
        ++count;
      }
      return count;
    }

    /// @brief Counts the `decode()` promises fulfilled, and keeps the messages of those rejected.
    struct DecodeResults
    {
      int Fulfilled = 0;
      List<utf8_string> Rejections;

      KRYS_NODISCARD ImageLoader::DecodePromise Promise() noexcept
      {
        return [this](dom::ExceptionOr<void> &&result)
        {
          if (result.HasException())
          {
            CHECK(result.GetException().Code() == dom::ExceptionCode::EncodingError);
            Rejections.push_back(result.GetException().Message());
          }
          else
          {
            ++Fulfilled;
          }
        };
      }
    };
  }

  TEST_CASE("HTMLImageElement::AssociatedMapElement: the first map whose id or name follows the first #",
            "[HTML][HTMLImageElement][html:rules-for-parsing-a-hash-name-reference]")
  {
    ImageFixture fixture(u8"<img id=empty usemap=#><img id=hashes usemap='a#b#c'><img id=none usemap=b>"
                         u8"<map id=blank name=''></map><map id=byname name='b#c'></map>");
    // Nothing after the #.
    CHECK(fixture.Image(u8"empty").AssociatedMapElement() == nullptr);
    // Everything after the first #.
    CHECK(fixture.Image(u8"hashes").AssociatedMapElement() == &fixture.ById(u8"byname"));
    // No #.
    CHECK(fixture.Image(u8"none").AssociatedMapElement() == nullptr);
  }

  TEST_CASE("HTMLImageElement: an image loads its source, fires load and paints",
            "[HTML][HTMLImageElement][html:dom-img-complete][html:dom-img-naturalwidth][html:dom-img-width]"
            "[html:img-dimensions]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    loader.Strategy.Defer = true;

    ImageFixture fixture(u8"<img id=img src='file:///a.png'>");
    auto &image = fixture.Image(u8"img");
    auto events = LoadEvents::ListenTo(image);

    REQUIRE(loader.Strategy.Requests == List<utf8_string> {u8"file:///a.png"});
    CHECK(image.CurrentURL().String() == u8"file:///a.png");
    CHECK_FALSE(image.Complete());
    CHECK(image.NaturalWidth() == 0u);
    CHECK(fixture.RendererSize(u8"img") == IntSize(0, 0));
    CHECK(Count(fixture.Painted(), u8"draw-native-image") == 0u);

    loader.Strategy.CompleteDeferred();
    RunNetworkingTasks();

    CHECK(events->Loads == 1);
    CHECK(events->Errors == 0);
    CHECK(image.Complete());
    CHECK(image.NaturalWidth() == 3u);
    CHECK(image.NaturalHeight() == 2u);
    CHECK(image.Width() == 3u);
    CHECK(fixture.RendererSize(u8"img") == IntSize(3, 2));
    auto *renderer = DynamicDowncast<RenderImage>(image.Renderer());
    REQUIRE(renderer);
    CHECK(renderer->GetCachedImage() == image.GetCachedImage());
    // On the line's baseline.
    CHECK(Count(fixture.Painted(), u8"(dest-rect at (8,20) size 3x2)") == 1u);
  }

  TEST_CASE("HTMLImageElement: failed and empty sources fire error and show alt text",
            "[HTML][HTMLImageElement]")
  {
    LoaderFixture loader;

    ImageFixture fixture(u8"<img id=missing alt=Missing src='file:///none.png'><br>"
                         u8"<img id=empty src=''><br>"
                         u8"<img id=nothing alt='' src='file:///none.png'><br>"
                         u8"<img id=nosrc alt=Text>");
    auto missingEvents = LoadEvents::ListenTo(fixture.ById(u8"missing"));
    auto emptyEvents = LoadEvents::ListenTo(fixture.ById(u8"empty"));
    RunNetworkingTasks();

    CHECK(missingEvents->Errors == 1);
    CHECK(missingEvents->Loads == 0);
    CHECK(emptyEvents->Errors == 1);
    CHECK(fixture.Image(u8"missing").Complete());
    CHECK(fixture.Image(u8"missing").NaturalWidth() == 0u);

    // The alt text, with 4px of padding, sizes the box.
    auto missingSize = fixture.RendererSize(u8"missing");
    CHECK(missingSize.Width() > 20);
    CHECK(missingSize.Height() > 10);
    CHECK(fixture.RendererSize(u8"nosrc").Width() > 10);
    // An empty alt makes a broken inline image collapse.
    CHECK(fixture.RendererSize(u8"nothing") == IntSize(0, 0));

    auto painted = fixture.Painted();
    CHECK(Count(painted, u8"draw-glyphs") == 2u);
    CHECK(Count(painted, u8"(color #C0C0C0)") >= 1u);
    CHECK(Count(painted, u8"draw-native-image") == 0u);
  }

  TEST_CASE("HTMLImageElement: a failed image without alt text shows the broken image icon",
            "[HTML][HTMLImageElement]")
  {
    LoaderFixture loader;
    ImageFixture fixture(u8"<img id=broken src='file:///none.png'><br>"
                         u8"<img id=large style='width: 60px; height: 40px' src='file:///none.png'><br>"
                         u8"<img id=tiny style='width: 10px; height: 10px' src='file:///none.png'>");
    RunNetworkingTasks();

    // The 16px icon with 4px of padding sizes the box.
    CHECK(fixture.RendererSize(u8"broken") == IntSize(20, 20));

    // It's drawn (its hill is green) inside the grey outline wherever it fits: not in the tiny image.
    auto painted = fixture.Painted();
    CHECK(Count(painted, u8"(color #549662)") == 2u);
    CHECK(Count(painted, u8"(color #C0C0C0)") >= 1u);
  }

  TEST_CASE("HTMLImageElement: the box follows the alt text when there's no image",
            "[HTML][HTMLImageElement]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;

    ImageFixture fixture(u8"<img id=text alt=a><img id=loaded alt=a src='file:///a.png'>");
    RunNetworkingTasks();
    auto small = fixture.RendererSize(u8"text");
    CHECK(fixture.RendererSize(u8"loaded") == IntSize(3, 2));

    fixture.Image(u8"text").Alt(u8"a much longer label");
    CHECK(fixture.RendererSize(u8"text").Width() > small.Width());

    // An alt change leaves a loaded image alone, and losing the image shows the text.
    fixture.Image(u8"loaded").Alt(u8"b");
    CHECK(fixture.RendererSize(u8"loaded") == IntSize(3, 2));
    fixture.Image(u8"loaded").RemoveAttribute(u8"src");
    RunNetworkingTasks();
    CHECK(fixture.RendererSize(u8"loaded") == small);
  }

  TEST_CASE("HTMLImageElement: an image without a source fires error if it has src or uses srcset or picture",
            "[HTML][HTMLImageElement][html:update-the-image-data][html:use-srcset-or-picture]"
            "[html:dom-img-currentsrc]")
  {
    LoaderFixture loader;
    ImageFixture fixture(u8"<img id=nocand srcset=' , '><img id=bad src='http://[bad'>"
                         u8"<picture><source srcset=','><img id=picnone></picture><img id=nosrc>");
    auto nocand = LoadEvents::ListenTo(fixture.ById(u8"nocand"));
    auto bad = LoadEvents::ListenTo(fixture.ById(u8"bad"));
    auto picnone = LoadEvents::ListenTo(fixture.ById(u8"picnone"));
    auto nosrc = LoadEvents::ListenTo(fixture.ById(u8"nosrc"));
    RunNetworkingTasks();

    // A srcset or picture without image sources selects no source.
    CHECK(nocand->Errors == 1);
    CHECK(css::tests::ToNarrow(fixture.Image(u8"nocand").CurrentSrc()) == "");
    // A URL that doesn't parse is the current URL as written.
    CHECK(bad->Errors == 1);
    CHECK(css::tests::ToNarrow(fixture.Image(u8"bad").CurrentSrc()) == "http://[bad");
    CHECK(picnone->Errors == 1);
    // Without src, srcset or picture, nothing happens.
    CHECK(nosrc->Errors == 0);
    CHECK(fixture.Image(u8"nosrc").Complete());
    CHECK(fixture.Image(u8"nocand").Complete());
  }

  TEST_CASE("HTMLImageElement: sizes from the width and height attributes, and object-fit",
            "[HTML][HTMLImageElement]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    loader.Strategy.Defer = true;

    ImageFixture fixture(u8"<style>img { display: block }</style>"
                         u8"<img id=both width=40 height=20 src='file:///a.png'>"
                         u8"<img id=width width=60 src='file:///a.png'>"
                         u8"<img id=contain style='width: 30px; height: 30px; object-fit: contain' "
                         u8"src='file:///a.png'>");
    // The attributes are the aspect ratio while the image loads.
    CHECK(fixture.RendererSize(u8"both") == IntSize(40, 20));

    loader.Strategy.CompleteDeferred();
    RunNetworkingTasks();

    CHECK(fixture.RendererSize(u8"both") == IntSize(40, 20));
    CHECK(fixture.RendererSize(u8"width") == IntSize(60, 40));
    CHECK(fixture.RendererSize(u8"contain") == IntSize(30, 30));
    // The contained image is centred in its box: 30x20, 5px down.
    CHECK(Count(fixture.Painted(), u8"(dest-rect at (8,73) size 30x20)") == 1u);
  }

  TEST_CASE("HTMLImageElement: srcset picks by density, and by width with sizes",
            "[HTML][HTMLImageElement][html:select-an-image-source][html:density-corrected-intrinsic-width-"
            "and-height]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///one.png"] = images::PNG;
    loader.Strategy.Resources[u8"file:///two.png"] = images::PNG;
    loader.Strategy.Resources[u8"file:///large.jpg"] = images::JPEG;

    ImageFixture fixture(
      u8"<img id=density src='file:///one.png' srcset='file:///two.png 2x'>"
      u8"<img id=widths srcset='file:///small.png 100w, file:///large.jpg 400w' sizes='200px'>");
    RunNetworkingTasks();

    CHECK(fixture.Image(u8"density").CurrentURL().String() == u8"file:///one.png");
    CHECK(fixture.Image(u8"density").UsesSrcsetOrPicture());

    // 400w at 200px is 2x: the 16x8 JPEG's natural size is halved.
    auto &widths = fixture.Image(u8"widths");
    CHECK(widths.CurrentURL().String() == u8"file:///large.jpg");
    CHECK(widths.NaturalWidth() == 8u);
    CHECK(widths.NaturalHeight() == 4u);
    CHECK(fixture.RendererSize(u8"widths") == IntSize(8, 4));
    CHECK(std::ranges::find(loader.Strategy.Requests, u8"file:///small.png")
          == loader.Strategy.Requests.end());

    // sizes defaults to 100vw: 200px.
    REQUIRE_FALSE(widths.SetAttribute(u8"sizes", u8"").HasException());
    CHECK(widths.CurrentURL().String() == u8"file:///large.jpg");
    REQUIRE_FALSE(widths.SetAttribute(u8"sizes", u8"50px").HasException());
    CHECK(widths.CurrentURL().String() == u8"file:///small.png");
  }

  TEST_CASE(
    "HTMLImageElement: a picture's sources, by type and media",
    "[HTML][HTMLImageElement][html:dom-img-currentsrc][html:the-source-element][html:update-the-source-set]"
    "[html:select-an-image-source]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///photo.jpg"] = images::JPEG;
    loader.Strategy.Resources[u8"file:///fallback.png"] = images::PNG;

    ImageFixture fixture(
      u8"<picture>"
      u8"<source srcset='file:///photo.avif' type=image/avif>"
      u8"<source srcset='file:///wide.png' media='(min-width: 1000px)'>"
      u8"<source id=jpeg srcset='file:///photo.jpg' type='image/jpeg; q=1' width=32 height=16>"
      u8"<img id=img src='file:///fallback.png' style='display: block'>"
      u8"</picture>");
    auto &image = fixture.Image(u8"img");
    RunNetworkingTasks();

    CHECK(image.CurrentURL().String() == u8"file:///photo.jpg");
    CHECK(image.UsesSrcsetOrPicture());
    // The selected source's width and height are the image's.
    CHECK(fixture.RendererSize(u8"img") == IntSize(32, 16));

    auto events = LoadEvents::ListenTo(image);
    REQUIRE_FALSE(fixture.ById(u8"jpeg").Remove().HasException());
    RunNetworkingTasks();

    CHECK(image.CurrentURL().String() == u8"file:///fallback.png");
    CHECK(events->Loads == 1);
    CHECK(fixture.RendererSize(u8"img") == IntSize(3, 2));
    CHECK(std::ranges::find(loader.Strategy.Requests, u8"file:///photo.avif")
          == loader.Strategy.Requests.end());
    CHECK(std::ranges::find(loader.Strategy.Requests, u8"file:///wide.png")
          == loader.Strategy.Requests.end());
  }

  TEST_CASE("HTMLImageElement: moving a picture's source or image selects the image's source again",
            "[HTML][HTMLImageElement][html:the-source-element][html:the-img-element]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///photo.jpg"] = images::JPEG;
    loader.Strategy.Resources[u8"file:///fallback.png"] = images::PNG;

    ImageFixture fixture(u8"<div id=outside></div><picture id=picture>"
                         u8"<source id=jpeg srcset='file:///photo.jpg' type=image/jpeg>"
                         u8"<img id=img src='file:///fallback.png' style='display: block'>"
                         u8"</picture>");
    auto &image = fixture.Image(u8"img");
    RunNetworkingTasks();
    REQUIRE(image.CurrentURL().String() == u8"file:///photo.jpg");

    auto &outside = fixture.ById(u8"outside");
    auto &picture = fixture.ById(u8"picture");
    SECTION("the source")
    {
      REQUIRE_FALSE(outside.MoveBefore(fixture.ById(u8"jpeg"), nullptr).HasException());
      RunNetworkingTasks();
      CHECK(image.CurrentURL().String() == u8"file:///fallback.png");

      REQUIRE_FALSE(picture.MoveBefore(fixture.ById(u8"jpeg"), &image).HasException());
      RunNetworkingTasks();
      CHECK(image.CurrentURL().String() == u8"file:///photo.jpg");
    }

    SECTION("the image")
    {
      REQUIRE_FALSE(outside.MoveBefore(image, nullptr).HasException());
      RunNetworkingTasks();
      CHECK(image.CurrentURL().String() == u8"file:///fallback.png");

      REQUIRE_FALSE(picture.MoveBefore(image, nullptr).HasException());
      RunNetworkingTasks();
      CHECK(image.CurrentURL().String() == u8"file:///photo.jpg");
    }
  }

  TEST_CASE("HTMLImageElement.decoding: reflects its known values, auto by default",
            "[HTML][HTMLImageElement][html:dom-img-decoding][html:attr-img-decoding]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto element = document->CreateElement(u8"img").ReleaseValue();
    auto &image = Downcast<html::HTMLImageElement>(Downcast<html::HTMLElement>(*element));
    CHECK(css::tests::ToNarrow(image.Decoding()) == "auto");
    image.Decoding(u8"ASYNC");
    CHECK(css::tests::ToNarrow(image.Decoding()) == "async");
    image.Decoding(u8"sync");
    CHECK(css::tests::ToNarrow(image.Decoding()) == "sync");
    image.Decoding(u8"later");
    CHECK(css::tests::ToNarrow(image.Decoding()) == "auto");
    CHECK(element->GetAttribute(u8"decoding") == u8"later");
  }

  TEST_CASE("HTMLImageElement: changing the source loads it", "[HTML][HTMLImageElement]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    loader.Strategy.Resources[u8"file:///b.jpg"] = images::JPEG;

    ImageFixture fixture(u8"<img id=img src='file:///a.png'>");
    auto &image = fixture.Image(u8"img");
    auto events = LoadEvents::ListenTo(image);
    RunNetworkingTasks();
    CHECK(events->Loads == 1);

    REQUIRE_FALSE(image.SetAttribute(u8"src", u8"file:///b.jpg").HasException());
    CHECK_FALSE(image.Complete());
    RunNetworkingTasks();

    CHECK(events->Loads == 2);
    CHECK(image.Complete());
    CHECK(image.CurrentURL().String() == u8"file:///b.jpg");
    CHECK(fixture.RendererSize(u8"img") == IntSize(16, 8));

    // Setting the same source again fires load again, from the memory cache.
    REQUIRE_FALSE(image.SetAttribute(u8"src", u8"file:///b.jpg").HasException());
    RunNetworkingTasks();
    CHECK(events->Loads == 3);
    CHECK(loader.Strategy.Requests.size() == 2u);
  }

  TEST_CASE("HTMLImageElement: the natural size is the density-corrected size, for the selected density",
            "[HTML][HTMLImageElement][html:density-corrected-intrinsic-width-and-height]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.jpg"] = images::DensityJPEG;

    ImageFixture fixture(u8"<img id=one src='file:///a.jpg'><img id=two srcset='file:///a.jpg 2x'>");
    RunNetworkingTasks();

    // 16x8 at 144 dots per inch.
    CHECK(fixture.Image(u8"one").NaturalWidth() == 8u);
    CHECK(fixture.Image(u8"one").NaturalHeight() == 4u);
    CHECK(fixture.RendererSize(u8"one") == IntSize(8, 4));
    // Then halved again for its density.
    CHECK(fixture.Image(u8"two").NaturalWidth() == 4u);
    CHECK(fixture.Image(u8"two").NaturalHeight() == 2u);
    CHECK(fixture.RendererSize(u8"two") == IntSize(4, 2));
  }

  TEST_CASE("HTMLImageElement: generated content images", "[HTML][HTMLImageElement]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;

    ImageFixture fixture(u8"<style>#before::before { content: url(file:///a.png) }</style>"
                         u8"<div id=replaced style='content: url(file:///a.png)'>text</div>"
                         u8"<div id=before></div>");
    RunNetworkingTasks();

    auto *replaced = DynamicDowncast<RenderImage>(fixture.ById(u8"replaced").Renderer());
    REQUIRE(replaced);
    CHECK(replaced->IsGeneratedContent());
    CHECK(fixture.RendererSize(u8"replaced") == IntSize(3, 2));
    CHECK(Count(fixture.Painted(), u8"draw-native-image") == 2u);
  }

  TEST_CASE("HTMLImageElement: animated images advance as they're painted", "[HTML][HTMLImageElement]")
  {
    using namespace std::chrono_literals;
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.gif"] = images::AnimatedGIF;
    loader.Strategy.Resources[u8"file:///b.webp"] = images::AnimatedWebP;

    ImageFixture fixture(
      u8"<img id=img src='file:///a.gif'>"
      u8"<div id=box style='width: 2px; height: 2px; background: url(file:///b.webp)'></div>");
    RunNetworkingTasks();
    auto &loop = html::EventLoop::Current();
    auto &image = Downcast<BitmapImage>(*fixture.Image(u8"img").GetImage());
    auto *background = fixture.ById(u8"box")
                         .Renderer()
                         ->Style()
                         .GetBackgroundLayers()
                         .UsedValues()
                         .begin()
                         ->GetImage()
                         .TryStyleImage()
                         ->GetCachedImage();
    REQUIRE(background);
    auto &backgroundImage = Downcast<BitmapImage>(*background->GetImage());

    auto start = loop.Now();
    (void)fixture.Painted();
    CHECK(image.IsAnimating());
    CHECK(backgroundImage.IsAnimating());

    loop.Run(start + 100ms);
    CHECK(image.CurrentFrameIndex() == 1u);
    CHECK(backgroundImage.CurrentFrameIndex() == 1u);
    // Painting the new frames starts their timers.
    (void)fixture.Painted();
    loop.Run(start + 200ms);
    CHECK(image.CurrentFrameIndex() == 2u);
    CHECK(backgroundImage.CurrentFrameIndex() == 2u);
  }

  TEST_CASE("HTMLImageElement: a loaded animated image starts animating when another element shows it",
            "[HTML][HTMLImageElement]")
  {
    using namespace std::chrono_literals;
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.gif"] = images::AnimatedGIF;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;

    ImageFixture fixture(u8"<img id=animated src='file:///a.gif'><img id=still src='file:///a.png'>"
                         u8"<img id=second><img id=third>");
    RunNetworkingTasks();
    auto &loop = html::EventLoop::Current();
    auto &animated = Downcast<BitmapImage>(*fixture.Image(u8"animated").GetImage());
    auto &still = Downcast<BitmapImage>(*fixture.Image(u8"still").GetImage());
    REQUIRE_FALSE(animated.IsAnimating());

    // The cached image gains a client, and its animation starts once the timers run, unpainted.
    REQUIRE_FALSE(fixture.Image(u8"second").SetAttribute(u8"src", u8"file:///a.gif").HasException());
    REQUIRE_FALSE(fixture.Image(u8"third").SetAttribute(u8"src", u8"file:///a.png").HasException());
    RunNetworkingTasks();
    REQUIRE(fixture.Image(u8"second").GetImage() == &animated);
    (void)fixture.Document->UpdateLayout();
    auto start = loop.Now();
    loop.Run(start);
    CHECK(animated.IsAnimating());
    CHECK_FALSE(still.IsAnimating());
    CHECK(animated.CurrentFrameIndex() == 0u);

    // Its first frame shows for its whole duration.
    loop.Run(start + 99ms);
    CHECK(animated.CurrentFrameIndex() == 0u);
    loop.Run(start + 100ms);
    CHECK(animated.CurrentFrameIndex() == 1u);
  }

  TEST_CASE("HTMLImageElement: animations pause out of view and resume in view", "[HTML][HTMLImageElement]")
  {
    using namespace std::chrono_literals;
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.gif"] = images::AnimatedGIF;

    ImageFixture fixture(
      u8"<style>body { margin: 0 }</style>"
      u8"<div id=box style='height: 100px; overflow: auto'>"
      u8"<img id=img src='file:///a.gif' style='display: block'><div style='height: 1000px'></div>"
      u8"</div>");
    RunNetworkingTasks();
    auto &loop = html::EventLoop::Current();
    auto &image = Downcast<BitmapImage>(*fixture.Image(u8"img").GetImage());
    auto &renderer = *fixture.Image(u8"img").Renderer();

    auto start = loop.Now();
    (void)fixture.Painted();
    REQUIRE(image.IsAnimating());

    // Scrolled out of view, the next frame finds the image hidden, and its animation pauses.
    fixture.ById(u8"box").ScrollTo(0, 500);
    loop.Run(start + 100ms);
    CHECK(image.CurrentFrameIndex() == 1u);
    CHECK(renderer.HasPausedImageAnimations());
    (void)fixture.Painted();
    CHECK_FALSE(image.IsAnimating());
    loop.Run(start + 400ms);
    CHECK(image.CurrentFrameIndex() == 1u);

    // Scrolled back, it resumes.
    fixture.ById(u8"box").ScrollTo(0, 0);
    CHECK_FALSE(renderer.HasPausedImageAnimations());
    CHECK(image.IsAnimating());
    loop.Run(start + 400ms);
    CHECK(image.CurrentFrameIndex() == 2u);
  }

  TEST_CASE("HTMLImageElement: animations pause while the page is hidden", "[HTML][HTMLImageElement]")
  {
    using namespace std::chrono_literals;
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.gif"] = images::AnimatedGIF;

    Ref<Page> page = Page::Create();
    ImageFixture fixture(*page, u8"<img id=img src='file:///a.gif'>");
    RunNetworkingTasks();
    auto &loop = html::EventLoop::Current();
    auto &image = Downcast<BitmapImage>(*fixture.Image(u8"img").GetImage());
    auto &renderer = *fixture.Image(u8"img").Renderer();

    auto start = loop.Now();
    (void)fixture.Painted();
    REQUIRE(image.IsAnimating());

    // Hidden, the next frame finds the image invisible, and its animation pauses.
    page->SetIsVisible(false);
    loop.Run(start + 100ms);
    CHECK(image.CurrentFrameIndex() == 1u);
    CHECK(renderer.HasPausedImageAnimations());
    loop.Run(start + 400ms);
    CHECK(image.CurrentFrameIndex() == 1u);

    // Shown, it resumes.
    page->SetIsVisible(true);
    CHECK_FALSE(renderer.HasPausedImageAnimations());
    CHECK(image.IsAnimating());
    loop.Run(start + 400ms);
    CHECK(image.CurrentFrameIndex() == 2u);
  }

  TEST_CASE("HTMLImageElement::Decode: fulfilled once the image has loaded and decoded, as a microtask",
            "[HTML][HTMLImageElement][html:dom-img-decode]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    loader.Strategy.Defer = true;

    ImageFixture fixture(u8"<img id=img src='file:///a.png'>");
    auto &image = fixture.Image(u8"img");
    DecodeResults results;

    // It waits for the image to load.
    image.Decode(results.Promise());
    RunNetworkingTasks();
    CHECK(results.Fulfilled == 0);
    loader.Strategy.CompleteDeferred();
    RunNetworkingTasks();
    CHECK(results.Fulfilled == 1);

    // Loaded, it's fulfilled at the next microtask checkpoint, not at once.
    image.Decode(results.Promise());
    CHECK(results.Fulfilled == 1);
    html::EventLoop::Current().PerformMicrotaskCheckpoint();
    CHECK(results.Fulfilled == 2);
    CHECK(results.Rejections.empty());
  }

  TEST_CASE(
    "HTMLImageElement::Decode: rejected without a source, when loading fails, or when the source changes",
    "[HTML][HTMLImageElement][html:dom-img-decode]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    loader.Strategy.Resources[u8"file:///b.jpg"] = images::JPEG;
    loader.Strategy.Defer = true;

    ImageFixture fixture(
      u8"<img id=none><img id=missing src='file:///none.png'><img id=img src='file:///a.png'>");
    DecodeResults results;

    SECTION("without a source")
    {
      fixture.Image(u8"none").Decode(results.Promise());
      RunNetworkingTasks();
      CHECK(results.Rejections == List<utf8_string> {u8"Missing source URL."});
    }

    SECTION("when loading fails")
    {
      fixture.Image(u8"missing").Decode(results.Promise());
      loader.Strategy.CompleteDeferred();
      RunNetworkingTasks();
      CHECK(results.Rejections == List<utf8_string> {u8"Loading error."});
    }

    SECTION("when the source changes, and a decode of the new source is fulfilled")
    {
      auto &image = fixture.Image(u8"img");
      image.Decode(results.Promise());
      REQUIRE_FALSE(image.SetAttribute(u8"src", u8"file:///b.jpg").HasException());
      image.Decode(results.Promise());
      RunNetworkingTasks();
      CHECK(results.Rejections == List<utf8_string> {u8"The image's source changed."});

      loader.Strategy.CompleteDeferred();
      RunNetworkingTasks();
      CHECK(results.Fulfilled == 1);
    }
    CHECK(results.Fulfilled <= 1);
  }

  TEST_CASE("HTMLImageElement::Decode: with the host's work queue, the image decodes there",
            "[HTML][HTMLImageElement][html:dom-img-decode]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    ManualWorkQueue queue;
    PlatformStrategies::Singleton().SetWorkQueue(&queue);
    auto clearWorkQueue = MakeScopeExit([] { PlatformStrategies::Singleton().SetWorkQueue(nullptr); });

    ImageFixture fixture(u8"<img id=img src='file:///a.png'>");
    RunNetworkingTasks();
    DecodeResults results;

    fixture.Image(u8"img").Decode(results.Promise());
    RunNetworkingTasks();
    CHECK(results.Fulfilled == 0);
    CHECK(queue.PendingCount() == 1);

    CHECK(queue.RunAll() == 1);
    RunNetworkingTasks();
    CHECK(results.Fulfilled == 1);
  }
}

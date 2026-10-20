#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/AnimatedImages.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/TestImages.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/Graphics/BitmapImage.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A document parsed in a frame, as a navigation's is, so its images load as they're inserted.
    struct ImageFixture : css::tests::ValueFixture
    {
      IntSize ViewSize;
      RefPtr<LocalFrame> Frame;
      Ref<dom::HTMLDocument> Document;

      explicit ImageFixture(utf8_string &&markup, IntSize viewSize = {200, 100}) noexcept
          : ViewSize(viewSize), Frame(LocalFrame::Create()), Document(CreateRef<dom::HTMLDocument>())
      {
        html::EventLoop::Current().RunUntilIdle();
        Frame->CreateView(ViewSize);
        Frame->SetDocument(ShareRefPtr<dom::Document>(Document.get()));
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

      KRYS_NODISCARD html::HTMLImageElement &Image(dom::DOMStringView id) noexcept
      {
        return Downcast<html::HTMLImageElement>(ById(id));
      }

      /// @brief The size of @p id's renderer, laid out.
      KRYS_NODISCARD IntSize RendererSize(dom::DOMStringView id) noexcept
      {
        (void)Document->UpdateLayout();
        auto *box = DynamicDowncast<RenderBox>(ById(id).Renderer());
        REQUIRE(box);
        return IntSize(box->Width().ToInt(), box->Height().ToInt());
      }

      /// @brief The display list of the whole view, laid out, as text.
      KRYS_NODISCARD utf8_string Painted() noexcept
      {
        (void)Document->UpdateLayout();
        display_list::RecorderImpl recorder(
          FloatSize(static_cast<float>(ViewSize.Width()), static_cast<float>(ViewSize.Height())));
        Frame->View()->PaintContents(recorder, IntRect(IntPoint(), ViewSize));
        return recorder.TakeDisplayList()->AsText({});
      }
    };

    /// @brief Counts an element's `load` and `error` events.
    struct LoadEvents : dom::EventListenerCallback
    {
      int32 Loads {0};
      int32 Errors {0};

      void HandleEvent(dom::Event &event) noexcept override
      {
        if (event.Type() == u8"load")
        {
          ++Loads;
        }
        else if (event.Type() == u8"error")
        {
          ++Errors;
        }
      }

      KRYS_NODISCARD static Ref<LoadEvents> ListenTo(dom::Element &element) noexcept
      {
        auto events = CreateRef<LoadEvents>();
        element.AddEventListener(dom::DOMStringAtom(u8"load"), Ref<dom::EventListenerCallback> {events},
                                 false);
        element.AddEventListener(dom::DOMStringAtom(u8"error"), Ref<dom::EventListenerCallback> {events},
                                 false);
        return events;
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
  }

  TEST_CASE("HTMLImageElement: an image loads its source, fires load and paints", "[HTML][HTMLImageElement]")
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

  TEST_CASE("HTMLImageElement: srcset picks by density, and by width with sizes", "[HTML][HTMLImageElement]")
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

  TEST_CASE("HTMLImageElement: a picture's sources, by type and media", "[HTML][HTMLImageElement]")
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
}
